;;;
;;; Tandy Color Computer 3
;;;
;;;    low level routines, but not the tricky ones.
;;;    see tricks.s for those.


            .module coco3

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
	    .globl _hz
	    .globl _bufpool
	    .globl _discard_size
            .globl _copy_common
	    .globl blkdev_rawflg
	    .globl blkdev_unrawflg

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
	    .globl video_init
	    .globl _scanmem

            include "kernel.def"
            include "../kernel09.def"


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

	
saved_map
	.db 0		; which mapping state where we in?
init1_mirror
	.db 0		; a *mirror* of gimme $ff91, which is WriteOnly
_need_resched
	.db 0		; scheduler flag


_plt_monitor:
	    orcc #0x10
	    bra _plt_monitor

_plt_reboot:
	orcc 	#0x10		; turn off interrupts
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
	    tfr cc,b		; return the old irq state
	    orcc #0x10
	    rts

;;; Turn on interrupts
;;;   takes: nothing
;;;   returns: nothing
___hard_ei:
	    andcc #0xef
	    rts

;;; Restore interrupts to saved setting
;;;   takes: B = saved state (as returned from _di )
;;;   returns: nothing
___hard_irqrestore:		; B holds the data
	    tfr b,cc
	    rts

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK
; -----------------------------------------------------------------------------
	.area .data
_hz:	.db	0  		; Is machine in 50hz?


            .area .discard

;;;  Stuff to initialize *before* hardware
;;;    takes: nothing
;;;    returns: nothing
init_early:
	ldx	#null_handler
	stx	1
	lda	#0x7E
	sta	0
        rts

;;; Initialize Hardware !
;;;    takes: nothing
;;;    returns: nothing
init_hardware:
	;; High speed poke
	sta	0xffd9		; high speed poke
	sta	0xffdf		; RAM mode
	;; set system RAM size
	jsr	_scanmem	; X = number of pages
	tfr 	x,d
	lslb
	rola
	lslb
	rola
	lslb
	rola
	std 	_ramsize
	subd	#64+16
	std 	_procmem
	;; set initial user mmu
	ldd	#8
	ldx	#$ffa0
b@	sta	,x+
	inca
	decb
	bne	b@
        ;; set temporary screen up
	clr	$ff9c		; reset scroll register
	ldb	#%01001100	; coco3 mode + fexx constant
	stb	$ff90
	;; detect PAL or NTSC ROM
	; Use the low 8K again for probing - we are running in the discard
	; area (C000-DFFF)
	ldb	#$3f		; put Super BASIC in mmu
	stb	$ffa8
	lda	$0033		; get BASIC's "mirror" of Video Reg
	ldb	#$00		; put Fuzix Kernel back in mmu
	stb	$ffa8		;
	anda	#$8		; mask off 50 hz bit
	sta	_hz		; save for future use
	;; continue setup of regs
	ora	#%00000100	; text @ 9 lines per char row
	sta	$ff98
	ldb	#%00010100	; 80 column mode
	stb	$ff99
	ldd	#$b400/8	; video at physical 0xb000
	std	$ff9d
	ldd	#$003f		; white on black text console
	sta	$ffb0
	stb	$ffb8
	;; clear video memory
	jsr	_video_init
        ;; Our vectors are in high memory unlike Z80 but we still
        ;; need vectors
	ldu	#0xfeee		; vector area
	lda	#$7e		; jump opcode
	ldx	#swi3_handler
	sta	,u+
	stx	,u++
	ldx 	#swi2_handler
	sta	,u+
	stx	,u++
	ldx 	#firq_handler
	sta	,u+
	stx	,u++
        ldx 	#my_interrupt_handler
	sta	,u+
	stx	,u++
        ldx 	#unix_syscall_entry
	sta	,u+
	stx 	,u++
	ldx 	#nmi_handler
	sta	,u+
	stx	,u
	jsr	_devtty_init
        rts


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area .common

;;; Platform specific userspace setup
;;;   We're going to borrow this to copy the common bank
;;;   into the userspace too.
;;;   takes: X = page table pointer
;;;   returns: nothing
_program_vectors:
	pshs	u
	;; setup the null pointer / sentinal bytes in low process memory
	lda	,x	     ; get process's blk address for address 0
	sta	0xffa8	     ; put in our mmu ( at address 0 )
	lda	#0x7E
	sta	0
	clr	0xffa8	     ;
	puls	u,pc


;;; This clear the interrupt source before calling the
;;; normal handler
;;;    takes: nothing ( it is an interrupt handler)
;;;    returns: nothing ( it is an interrupt handler )
my_interrupt_handler
	lda	$ff02		; clear pia irq latch by reading data port
	jmp	interrupt_handler ; jump to regular handler


;;; This is the swi2 handler. Only user-space will be calling this so
;;; assume userspace is already mapped in.  we take our final vectors from
;;; the current userspace as each proc can have it's own vector.
swi3_handler
	ldx	$fe		; load swi3 vector
	bra	b@
swi2_handler
	ldx	$fc		; load swi2 vector
b@	beq	a@		; if zero do nothing
	jmp	,x		;
a@	rti


;;;  FIXME:  these interrupt handlers should prolly do something
;;;  in the future.
firq_handler:
	    rti


;;; Userspace mapping pages 7+  kernel mapping pages 3-5, first common 6
;;; All registers preserved
map_proc_always:
	    pshs x,y,u
	    ldx #U_DATA__U_PAGE
	    jsr map_proc_2
	    puls x,y,u,pc

;;; Maps a page table into cpu space
;;;   takes: X - pointer page table ( ptptr )
;;;   returns: nothing
;;;   modifies: nothing
map_proc:
	    cmpx #0		; is zero?
	    bne map_proc_2	; no then map process; else: map the kernel
	;; !!! fall-through to below

;;; Maps the Kernel into CPU space
;;;   takes: nothing
;;;   returns: nothing
;;;   modifies: nothing
;;;	Map in the kernel below the current common, all registers preserved
map_kernel:
	    pshs a
	    lda #1		; flip to mmu map 1 (kernel)
	    sta	init1_mirror	; save copy in INIT1 mirror
	    sta 0xff91		;
	    puls a,pc

;;; User is in the FFA0 map with the top 8K as common
;;; As the core code currently does 16K happily but not 8 we just pair
;;; up pages

;;; Maps a page table into the MMU
;;;   takes: X = pointer to page table
;;;   returns: nothing
;;;   modifies: nothing
map_proc_2:
	    pshs x,y,a
	    ldy #0xffa0		; MMU user map. We can fiddle with

	    lda ,x+		; get byte from page table
	    sta ,y+		; put it in mmu
	    inca		; increment to get next 8k block
	    sta ,y+		; put it in mmu

	    lda ,x+
	    sta ,y+
	    inca
	    sta ,y+

	    lda ,x+
	    sta ,y+
	    inca
	    sta ,y+

	    lda ,x+		; bank all but common memory
	    sta ,y


	    lda  #0
	    sta init1_mirror		; and save INIT1 setting in mirror
	    sta 0xff91			; new mapping goes live here
	    puls x,y,a,pc		; so had better include common!

;;;
;;;	Restore a saved mapping. We are guaranteed that we won't switch
;;;	common copy between save and restore. Preserve all registers
;;;
;;;	We cheat somewhat. We have two mapping sets, so just remember
;;;	which space we were in. Note: we could be in kernel in either
;;;	space while doing user copies
;;;
map_restore:
	    pshs a
	    lda	saved_map
	    sta init1_mirror
	    sta 0xff91
	    puls a,pc

;;; Save current mapping
;;;   takes: nothing
;;;   returns: nothing
map_save:
	    pshs a
	    lda init1_mirror
	    sta saved_map
	    puls a,pc


;;;  Print a character to debugging
;;;   takes: A = character
;;;   returns: nothing
outchar:
	pshs 	b,x
	ldx	scrPos
	sta	,x+
	stx	scrPos
	puls 	b,x,pc

	.area	.data
scrPos	.dw	0xb400		; debugging screen buffer position
	.area	.text


;;; Maps the memory for swap transfers
;;;   takes: A = swap token ( a page no. )
;;;   returns: nothing
map_for_swap
	sta	0xffa8
	inca
	sta	0xffa9
	rts

;;; Copy existing common area to page
;;;   takes: B = destination page
;;;   returns: nothing
_copy_common
	pshs	u
	incb
	stb	0xffa8
	ldx	#0xe200
	ldu	#0xe200&0x1fff
a@	ldd	,x++
	std	,u++
	cmpx	#0xff00
	bne	a@
	clr	0xffa8
	puls	u,pc

	.area	.common
;;; Helper for blkdev drivers to setup memory based on rawflag
;;; WARNING: the blk_op struct will not be available for rawflag=1/2 after calling this!
blkdev_rawflg
	pshs d,x     ; save regs
	ldb _td_raw ; 0 = kernel 1 = user 2 = swap
        decb         ; compare to 1
        bmi out@     ; less than or equal: 0, or 1 don't do map
	beq proc@    ; is direct to process
        ;; is swap so map page into kernel memory at 0x0000
        ldb _td_page ; get page no for swap
        stb 0xffa8   ; task 1, kernel task regs.
        incb         ; inc page no... next block no.
        stb 0xffa9   ; store it in mmu
	puls d,x,pc
proc@	jsr map_proc_always
 	; get parameters from C, X points to cmd packet
out@	puls d,x,pc

;;; Helper for blkdev drivers to clean up memory after blkdev_rawflg
blkdev_unrawflg
        ldb #0
        stb 0xffa8
        incb
        stb 0xffa9
        jsr map_kernel
	rts
