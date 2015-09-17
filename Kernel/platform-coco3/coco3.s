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
	    .globl map_process
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore
	    .globl _need_resched

            ; exported debugging tools
            .globl _trap_monitor
	    .globl _trap_reboot
            .globl outchar
	    .globl _di
	    .globl _ei
	    .globl _irqrestore

            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl unix_syscall_entry
	    .globl nmi_handler
	    .globl null_handler

            include "kernel.def"
            include "../kernel09.def"

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


_trap_monitor:
	    orcc #0x10
	    bra _trap_monitor

_trap_reboot:
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
_di:
	    tfr cc,b		; return the old irq state
	    orcc #0x10
	    rts

;;; Turn on interrupts
;;;   takes: nothing
;;;   returns: nothing
_ei:
	    andcc #0xef
	    rts

;;; Restore interrupts to saved setting
;;;   takes: B = saved state (as returned from _di )
;;;   returns: nothing
_irqrestore:			; B holds the data
	    tfr b,cc
	    rts

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK
; -----------------------------------------------------------------------------
            .area .text

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
	;; set initial user mmu
	ldd	#8
	ldx	#$ffa0
b@	sta	,x+
	inca
	decb
	bne	b@
        ;; set temporary screen up
	ldb	#%01000100	; coco3 mode
	stb	$ff90
	ldb	#%00001100	; text / 8 lines per char row
	stb	$ff98
	ldb	#%00010100	; 80 column mode
	stb	$ff99
	ldd	#$b400/8	; video at physical 0xb000
	std	$ff9d
	ldd	#$003f		; white on black text console
	sta	$ffb0
	stb	$ffb8
	;; clear video memory
	ldx	#$ac00
	lda	#$20
a@	sta	,x+
	cmpx	#$bb80
	bne	a@
	;; check for 512k RAM
	ldx	#0xffad
	ldy	#0xa000
	ldb	,x		; save mmu setting
	pshs	b
	ldd	#$3808
	sta	,x		; mmu to 38
	clr	,y		; clear it
	stb	,x		; mmu to 08
	clr	,y		; clear it
	inc	,y		; inc it
	sta	,x		; mmu to 38
	tst	,y		; test datum in 38
	tfr	cc,a		; A == test
	puls	b		; pull and reset mmu
	stb	,x		;
	tfr	a,cc		; get results in CC again
	beq	c@		; it didn't change so 512k
	;; only 128k!!!
d@	ldx	#nomem@
	jsr	_panic
	;; set system RAM size
c@	ldd 	#512
	std 	_ramsize
	ldd 	#512-64
	std 	_procmem
        ;; Our vectors are in high memory unlike Z80 but we still
        ;; need vectors
	ldu	#0xfeee		; vector area
	lda	#$7e		; jump opcode
	ldx 	#badswi_handler	; swi2 and swi3 are bad
	sta	,u+
	stx 	,u++
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
        rts
nomem@	fcn	"only have 128k\n"

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
	lda	0xffa8	     ; save mmu reg on stack
	pshs	a,x,u

	;; setup the null pointer / sentinal bytes in low process memory
	lda	[1,s]	     ; get process's blk address for address 0
	sta	0xffa8	     ; put in our mmu ( at address 0 )
	lda	#0x7E
	sta	0
	puls	a,x,u	     ; restore mmu reg
	sta	0xffa8	     ; 
	rts		     ; return

;;; This clear the interrupt source before calling the
;;; normal handler
;;;    takes: nothing ( it is an interrupt handler)
;;;    returns: nothing ( it is an interrupt handler )
my_interrupt_handler
	lda	$ff02		; clear pia irq latch by reading data port
	jmp	interrupt_handler ; jump to regular handler

;;;  FIXME:  these interrupt handlers should prolly do something
;;;  in the future.
firq_handler:
badswi_handler:
	    rti


;;; Userspace mapping pages 7+  kernel mapping pages 3-5, first common 6
;;; All registers preserved
map_process_always:
	    pshs x,y,u
	    ldx #U_DATA__U_PAGE
	    jsr map_process_2
	    puls x,y,u,pc

;;; Maps a page table into cpu space
;;;   takes: X - pointer page table ( ptptr )
;;;   returns: nothing
;;;   modifies: nothing
map_process:
	    cmpx #0		; is zero?
	    bne map_process_2	; no then map process; else: map the kernel
	;; !!! fall-through to below

;;; Maps the Kernel into CPU space
;;;   takes: nothing
;;;   returns: nothing
;;;   modifies: nothing
;;;	Map in the kernel below the current common, all registers preserved
map_kernel:
	    pshs a
	    lda #1		; flip to mmu map 1 (kernel)
	    sta 0xff91		;
	    sta	init1_mirror	; save copy in INIT1 mirror
	    puls a,pc

;;; User is in the FFA0 map with the top 8K as common
;;; As the core code currently does 16K happily but not 8 we just pair
;;; up pages

;;; Maps a page table into the MMU
;;;   takes: X = pointer to page table
;;;   returns: nothing
;;;   modifies: nothing
map_process_2:
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
	    sta 0xff91			; new mapping goes live here
	    sta init1_mirror		; and save INIT1 setting in mirror
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



