;;;
;;; Multicomp 6809 FPGA-based computer
;;;
;;;    low level routines, but not the tricky ones.
;;;    see tricks.s for those.

;;; coco3:
;;; $ff91 writeonly
;;; $ffa0
;;; $ff90
;;; $ffd9   high-speed poke
;;; $ff9c   scroll register
;;; $ffae   super basic in MMU
;;; $c033   BASIC mirror of video reg
;;; $ff98   video row setup
;;; $ff99   video col setup
;;; $ff9d   video map setup
;;; $ffb0   video colour
;;; $ffb0   video colour
;;; $ffb8   video colour
;;; $ffb0   video colour
;;; $ffb0   video colour

;;; coco3 MMU
;;; accessed through registers $ff91 and $ffa0-$ffaf
;;; 2 possible memory maps: map0, map1 selected by $ff91[0]
;;; map0 is used for User mode, map1 is used for Kernel mode.
;;; map1 is selected at boot (ie, now).
;;; when 0, select map0 using pages stored in $ffa0-$ffa7
;;; when 1, select map1 using pages stored in $ffa8-$ffaf
;;; a 512K system has 64 blocks, numbered $00 to $3f
;;; write the block number to the paging register. On readback,
;;; only bits 5:0 are valid; the other bits can contain junk.

;;; multicomp09 MMU
;;; accessed through two WRITE-ONLY registers MMUADR, MMUDAT
;;; 2 possible memory maps: map0, map1 selected by MMUADR[6]
;;; map0 is used for User mode, map1 is used for Kernel mode.
;;; map0 is selected at boot (ie, now)
;;; .. to avoid pointless divergence from coco3, the first
;;; hardware setup step will be to flip to map1.
;;; [NAC HACK 2016Apr23] in the future, may handle this in
;;; forth or in the bootstrap
;;; when 0, select map0 using MAPSEL values 0-7
;;; when 1, select map1 using MAPSEL values 8-15
;;; MAPSEL is MMUADR[3:0]
;;; a  512K system has  64 blocks, numbered $00 to $3f
;;; a 1024K system has 128 blocks, numbered $00 to $7f
;;; Write the block number to MMUDAT[6:0]
;;; MMUDAT[7]=1 write-protects the selected block - NOT USED HERE!

;;; coco3: at the time the boot loader passes control this the code here,
;;; map1 is selected (Kernel space) and the map1 mapping
;;; registers are selecting blocks 0-7.
;;; map0 is selecting blocks $38-$3f.

;;; multicomp09: at the time the boot loader passes control this the code here,
;;; map0 is selected (user space) and the map0 mapping
;;; registers are selecting blocks 0-7.
;;; map1 mapping registers are uninitialised.

	.module multicomp09

	; exported symbols
	.globl init_early
	.globl init_hardware
	.globl interrupt_handler
        .globl _program_vectors
	.globl map_kernel
	.globl map_proc
	.globl map_proc_always
	.globl map_save
	.globl map_restore
	.globl _need_resched
	.globl _bufpool
	.globl _discard_size
        .globl _krn_mmu_map
        .globl _usr_mmu_map
	.globl curr_tr

	; exported debugging tools
        .globl _plt_monitor
	.globl _plt_reboot
        .globl outchar
	.globl ___hard_di
	.globl ___hard_ei
	.globl ___hard_irqrestore

	; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl unix_syscall_entry
	.globl nmi_handler
	.globl null_handler

        include "kernel.def"
        include "../../cpu-6809/kernel09.def"
	include "platform.def"

	.area	.buffers

_bufpool:
	.ds	BUFSIZE*NBUFS

	.area	.discard
_discard_size:
	.db	__sectionlen_.discard__/BUFSIZE

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK
; -----------------------------------------------------------------------------
	.area .common


saved_tr
	.db 0		; the saved state of the TR bit etc (MMU_MAP0 or MMUMAP1)
curr_tr
	.db 0		; the current state of the TR bit etc (MMU_MAP0 or MMUMAP1)
_need_resched
	.db 0		; scheduler flag

;;; multicomp09 mmu registers are write-only so need to store
;;; a copy of the mappings.
;;; Each byte represents one of the 8Kbyte physical address regions.
;;; In general, pages are assigned in pairs, but that is not true for the top
;;; 8Kbyte of address space, so my first attempt (just to store 4 bytes per map)
;;; came unstuck.
;;; [NAC HACK 2016May07] may not need to store usr_mmu_map at all.. review later.
;;;
;;; Need to write these values any time we're changing the MMU mapping.. UNLESS
;;; it's clear that the routine is subsequently going to restore a value that is
;;; currently stored here.
;;;
;;; The values of 0-7 set here for _krn_mmu_map are used to initialise the MMU
;;; mappings for the kernel. Don't change them!!
;;; The values of 0-7 set here for _usr_mmu_map reflect how the user mappings
;;; are set up when the kernel is started - so don't change them either!
_krn_mmu_map
	.db	0,1,2,3,4,5,6,7 ; mmu registers 8-f
_usr_mmu_map
	.db	0,1,2,3,4,5,6,7 ; mmu registers 0-7


_plt_monitor:
	orcc	#0x10
	bra	_plt_monitor

_plt_reboot:
	orcc 	#0x10		; turn off interrupts
        bra     _plt_reboot    ; [NAC HACK 2016May07] endless loop


        lda	#0x38		; put RAM block in memory
	sta	0xffa8		;
	;; copy reboot bounce routine down
	ldx	#0		;
	ldu	#bounce@
loop@	lda	,u+
	sta	,x+
	cmpu	#bounce_end@
	bne	loop@
	jmp	0		;
	;; this code is PIC and gets copied down to
	;; low memory on reboot to bounce to the reset
	;; vector.
bounce@
	;; [NAC HACK 2016May01] todo.. for multicomp need to re-enable the ROM.
	;; how to test this??
	lda	#0x06		; reset GIME (map in internal 32k rom)
	sta  	0xff90
	clr	0xff91
	clr	0x72
	jmp	[0xfffe]	; jmp to reset vector
bounce_end@



;;; Turn off interrupts
;;;    takes: nothing
;;;    returns: B = original irq (cc) state
___hard_di:
	tfr	cc,b		; return the old irq state
	orcc	#0x10
	rts

;;; Turn on interrupts
;;;   takes: nothing
;;;   returns: nothing
___hard_ei:
	andcc	#0xef
	rts

;;; Restore interrupts to saved setting
;;;   takes: B = saved state (as returned from _di )
;;;   returns: nothing
___hard_irqrestore:		; B holds the data
	tfr	b,cc
	rts

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK
; -----------------------------------------------------------------------------

        .area .discard

;;;  Stuff to initialize *before* hardware
;;;    takes: nothing
;;;    returns: nothing
init_early:
	ldx	#null_handler	; [NAC HACK 2016Apr23] what's this for??
	stx	1
	lda	#0x7E
	sta	0
        rts



;;; Initialize Hardware !
;;;    takes: nothing
;;;    returns: nothing
init_hardware:
	;; [NAC HACK 2016Apr23] todo: size the memory. For now, assume 512K like coco3
	;; set system RAM size
	ldd 	#512
	std 	_ramsize
	ldd 	#512-64
	std 	_procmem

;;; Enable timer interrupt
        lda     #TIMER_ON
        sta     TIMER

;;; [NAC HACK 2016Apr23] coco3 at this point sets up physical blocks 0-7 for user mode.
;;; ..which is the same mapping that is in use for kernel mode.

;;; multicomp09 currently has map0 selected and blocks 0-7 mapped.
;;; To match the coco3 set-up need to select blocks 0-7 for map1 then switch to map1.
;;; Want to end with _krn_mmu_map and _usr_mmu_map and curr_tr all correct.
;;;

	;; set up the map1 registers (MAPSEL=8..f) to use pages 0-7
	;; ..to match the pre-existing setup of map0.
	;; _krn_mmu_map is set up with the required values.
	;; while doing this, keep TR=0 because we are using
	;; map0 and don't want to switch the map yet.
	lda	#(MMU_MAP0|8)	; stay in map0, select 1st mapping register for map1
	ldx	#MMUADR

	ldy	#_krn_mmu_map
	ldb	,y+   		; page from krn_mmu_map
	std	,x		; Write A to MMUADR to set MAPSEL=8, then write B to MMUDAT
	inca			; next mapsel
	ldb     ,y+     	; next page from krn_mmu_map
	std	,x		; Write A to MMUADR to set MAPSEL=9, then write B to MMUDAT
	inca			; next mapsel
	ldb     ,y+     	; next page from krn_mmu_map
	std	,x		; Write A to MMUADR to set MAPSEL=a, then write B to MMUDAT
	inca			; next mapsel
	ldb     ,y+     	; next page from krn_mmu_map
	std	,x		; Write A to MMUADR to set MAPSEL=b, then write B to MMUDAT
	inca			; next mapsel
	ldb     ,y+     	; next page from krn_mmu_map
	std	,x		; Write A to MMUADR to set MAPSEL=c, then write B to MMUDAT
	inca			; next mapsel
	ldb     ,y+     	; next page from krn_mmu_map
	std	,x		; Write A to MMUADR to set MAPSEL=d, then write B to MMUDAT
	inca			; next mapsel
	ldb     ,y+     	; next page from krn_mmu_map
	std	,x		; Write A to MMUADR to set MAPSEL=e, then write B to MMUDAT
	inca			; next mapsel
	ldb     ,y+     	; next page from krn_mmu_map
	std	,x		; Write A to MMUADR to set MAPSEL=f, then write B to MMUDAT

	;; swap to map1
	;; the two labels generate entries in the map file that are useful
	;; when debugging: did we get past this step successfully.
gomap1:	lda	#MMU_MAP1
	sta	,x
	sta	curr_tr
atmap1:


	;; Multicomp09 has RAM at the hardware vector positions
	;; so we can write the addresses directly; 2 bytes per vector:
	;; no need for a jump op-code.
	ldx	#0xfff2		; address of SWI3 vector
	ldy	#badswi_handler
	sty	,x++		; SWI3 handler
	sty	,x++		; SWI2 handler
	ldy	#firq_handler
	sty	,x++		; FIRQ handler
	ldy	#interrupt_handler
	sty	,x++		; IRQ  handler
	ldy	#unix_syscall_entry
	sty	,x++		; SWI  handler
	ldy	#nmi_handler
	sty	,x++		; NMI  handler

	jsr	_devtty_init
xinihw:	rts


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

	.area .common

;;; Platform specific userspace setup
;;;   We're going to borrow this to copy the common bank
;;;   into the userspace too.
;;;   takes: X = page table pointer
;;;   returns: nothing
_program_vectors:
	;; copy the common section into user-space
	pshs	y
	ldy	#MMUADR

	;; setup the null pointer / sentinal bytes in low process memory
	lda	curr_tr		; Select MMU register associated with
	ora	#8		; block 8
	sta	,y

	lda	,x	     	; get process's blk address for address 0
	sta	1,y		; and put it in MMUDAT
	lda	#0x7E
	sta	0

	;; restore the MMU mapping that we trampled on
	;; MMUADR still has block 8 selected so no need to re-write it.

	;; retrieve value that used to be in block 8
	lda	_krn_mmu_map
	;; and restore it
	sta	1,y		; MMUDAT

	puls	pc,y	     	; restore reg and return



;;;  FIXME:  these interrupt handlers should prolly do something
;;;  in the future.
firq_handler:
badswi_handler:
	rti


;;; Userspace mapping pages 7+  kernel mapping pages 3-5, first common 6
;;;   takes: nothing
;;;   returns: nothing
;;;   modifies: nothing - all registers preserved
map_proc_always:
	pshs	x
	ldx	#U_DATA__U_PAGE
	jsr	map_proc_2
	puls	x,pc

;;; Maps a page table into cpu space
;;;   takes: X - pointer page table ( ptptr )
;;;   returns: nothing
;;;   modifies: nothing - all registers preserved
map_proc:
	cmpx	#0		; is zero?
	bne	map_proc_2	; no then map process; else: map the kernel
	;; !!! fall-through to below

;;; Maps the Kernel into CPU space
;;;   takes: nothing
;;;   returns: nothing
;;;   modifies: nothing - all registers preserved
;;;	Map in the kernel below the current common, all registers preserved
map_kernel:
	pshs	a
	lda	#MMU_MAP1	; flip to mmu map 1 (kernel)
	sta	MMUADR
	sta	curr_tr		; save copy
	puls 	a,pc

;;; User is in MAP0 with the top 8K as common
;;; As the core code currently does 16K happily but not 8 we just pair
;;; up pages

;;; Maps a page table into the MMU
;;;   takes: X = pointer to page table
;;;   returns: nothing
;;;   modifies: nothing - all registers preserved
map_proc_2:
	pshs	x,y,a,b

	;; first, copy entries from page table to usr_mmu_map
	ldy	#_usr_mmu_map

	lda	,x+		; get byte from page table
	sta	0,y		; copy to usr_mmu_map
	inca			; increment to get next 8K block
	sta	1,y		; copy to usr_mmu_map

	lda	,x+		; get byte from page table
	sta	2,y		; copy to usr_mmu_map
	inca			; increment to get next 8K block
	sta	3,y		; copy to usr_mmu_map

	lda	,x+		; get byte from page table
	sta	4,y		; copy to usr_mmu_map
	inca			; increment to get next 8K block
	sta	5,y		; copy to usr_mmu_map

	lda	,x+		; bank all but common memory
	sta	6,y		;

	;; now, update MMU with those values
	lda	#(MMU_MAP1|0)	; stay in map1, select mapsel=0
	ldx	#MMUADR

	ldb	,y+   		; page from usr_mmu_map
	std	,x		; Write A to MMUADR to set MAPSEL=0, then write B to MMUDAT
	inca			; next mapsel
	ldb     ,y+     	; next page from usr_mmu_map
	std	,x		; Write A to MMUADR to set MAPSEL=1, then write B to MMUDAT
	inca			; next mapsel
	ldb     ,y+     	; next page from usr_mmu_map
	std	,x		; Write A to MMUADR to set MAPSEL=2, then write B to MMUDAT
	inca			; next mapsel
	ldb     ,y+     	; next page from usr_mmu_map
	std	,x		; Write A to MMUADR to set MAPSEL=3, then write B to MMUDAT
	inca			; next mapsel
	ldb     ,y+     	; next page from usr_mmu_map
	std	,x		; Write A to MMUADR to set MAPSEL=4, then write B to MMUDAT
	inca			; next mapsel
	ldb     ,y+     	; next page from usr_mmu_map
	std	,x		; Write A to MMUADR to set MAPSEL=5, then write B to MMUDAT
	inca			; next mapsel
	ldb     ,y+     	; next page from usr_mmu_map
	std	,x		; Write A to MMUADR to set MAPSEL=6, then write B to MMUDAT

	lda	#MMU_MAP0
	sta	,x		; new mapping goes live here
	sta	curr_tr		; and remember new TR setting
	puls	x,y,a,b,pc	; so had better include common!

;;;
;;;	Restore a saved mapping. We are guaranteed that we won't switch
;;;	common copy between save and restore. Preserve all registers
;;;
;;;	We cheat somewhat. We have two mapping sets, so just remember
;;;	which space we were in. Note: we could be in kernel in either
;;;	space while doing user copies
;;;
map_restore:
	pshs	a
	lda	saved_tr
	sta	curr_tr
	sta	MMUADR
	puls	a,pc

;;; Save current mapping
;;;   takes: nothing
;;;   returns: nothing
map_save:
	pshs	a
	lda	curr_tr
	sta	saved_tr
	puls	a,pc

;;; Maps the memory for swap transfers
;;;   takes: A = swap token ( a page no. )
;;;   returns: nothing
;;; [NAC HACK 2016May01] maps 16K into kernel space
;;; [NAC HACK 2016May15] coco3 has this in .text instead of .common - is that correct?
map_for_swap
	ldb	curr_tr
	orb	#8
	stb	MMUADR		; select first 8K in kernel mapping mapsel=8
	sta	MMUDAT
	incb
	stb	MMUADR		; select second 8K in kernel mapping mapsel=9
	sta	MMUDAT
	rts

;;;  Print a character to debugging
;;;   takes: A = character
;;;   returns: nothing
outchar:
	pshs    b,cc
vdubiz  ldb     UARTSTA0
        bitb    #2
        beq     vdubiz	 ; busy

	sta	UARTDAT0 ; ready, send character
	puls	b,cc,pc
