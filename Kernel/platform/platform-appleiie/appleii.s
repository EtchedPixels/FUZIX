;
;	    Apple IIe platform functions
;

        .export init_early
        .export init_hardware
        .export _program_vectors
	.export map_kernel
	.export map_proc
	.export map_proc_always
	.export map_save_kernel
	.export map_restore

	.export _unix_syscall_i
	.export _plt_interrupt_i
	.export plt_doexec

	.export _hd_map

        ; exported debugging tools
        .export _plt_monitor
	.export _plt_reboot
        .export outchar
	.export ___hard_di
	.export ___hard_ei
	.export ___hard_irqrestore
	.export vector
	.export _sys_stubs

	.import interrupt_handler
	.import _ramsize
	.import _procmem
	.import nmi_handler
	.import unix_syscall_entry
	.import kstack_top
	.import istack_switched_sp
	.import istack_top
	.import _unix_syscall
	.import _plt_interrupt
	.import _kernel_flag
	.import stash_zp
	.import pushax
	.import _relocator
	.import _udata

	.import outcharhex
	.import outxa
	.import incaxy

	.import _create_init_common

        .include "kernel.def"
        .include "../../cpu-6502/kernel02.def"
	.include "zeropage.inc"

;
;	syscall is jsr [$00fe] (for now anyway)
;
syscall	     =  $FE
; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0x0200 upwards after the common data blocks)
; -----------------------------------------------------------------------------
            .segment "COMMONMEM"

;
;	Fixme - can we get back to the AppleII monitor ?
;
_plt_monitor:
	lda	$C002		; ROM I invoke thee
	jmp	$FF59		; Monitor entry point

_plt_reboot:
	jmp	_plt_reboot	; FIXME: original ROM map and jmp

___hard_di:
	php
	sei			; Save old state in return to C
	pla			; Old status
	rts
___hard_ei:
	cli			; on 6502 cli enables IRQs!!!
	rts

___hard_irqrestore:
	and	#4		; IRQ flag
	bne	irq_on
	cli
	rts
irq_on:
	sei
	rts

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
        .code

init_early:
        rts

init_hardware:
        ; set system RAM size for test purposes
	lda	#128
	sta	_ramsize
	lda	#0
	sta	_ramsize+1
	lda	#64
	sta	_procmem
	lda	#0
	sta	_procmem+1
	rts

	    ; copied into the stubs of each binary
_sys_stubs:
	jmp	syscall_entry
	.byte	0
	.word	0
	.word	0
	.word	0
	.word 0
	.word 0
	.word 0

;
;	We will set the vectors and stubs up early in boot for both banks  so
;	need do nothing here.
;
_program_vectors:
	rts

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .segment "COMMONMEM"
;
;	For swap only this is trivial. The later iie + ramworks kernel
;	will have far more to do as we'll need to load a page register
;	from the process page. We need these routines in common so always
;	available. (Probably crt0.s will need to coppy commonmem to both
;	language banks not just the vectors)
;
;	For performance reasons user is in the main bank
;
map_proc_always:
	sta $C002
	sta $C004
	sta $C054
	rts
;
;	X,A points to the map table of this process but it doesn't
;	matter as we have one process
;
map_proc:
	cmp	#0
	bne	map_proc_always
	cpx	#0
	bne	map_proc_always
;
;	Map in the kernel
;	$200-$BFFF from aux bank
;	Our language bank is permanently(ish)
;	the alt bank except for the stubs that
;	invoke ProDOS.
;
map_kernel:
	sta $C003
	sta $C005
	sta $C055
	rts

;
;	We will need to save/restore the
;	ZP/Language page once we are running
;	ProDOS hooks with interrupts TODO
;
map_restore:
	lda	saved_map
	beq	map_kernel
	bne	map_proc_always
;
;	Save the current mapping. For the moment we just track if we are
;	user or kernel. We may eventually need to track language bank
;	switches, video and more.
;
map_save_kernel:
	pha
	lda $C013
	and #$80
	sta saved_map
	; and map in the kernel
	sta $C003
	sta $C005
	sta $C055
	pla
	rts

saved_map:  .byte 0

; outchar: Wait for UART TX idle, then print the char in a without
; corrupting other registers

outchar:
; We don't have an easy way to do this - could vtoutput ??
	rts

;
;	Code that will live in each bank at the same address and is copied
;	there at boot.
;
	.segment "STUBS"
;
;	Interrupt vector logic. Keep this in platform for the 6502 so that
;	we can use the shorter one for the CMOS chip
;
vector:
	pha
	txa
	pha
	tya
	pha
	cld

;	Remember where we are coming from. Also remember we may not be
;	going back quite the same way. For single process in memory however
;	it's not too hard - if we are in kernel mapping an IRQ will always
;	exit in kernel mapping.

	jsr	map_save_kernel

;
;	Q: do we want to spot brk() instructions and signal them ?
;
;
;	Save the old stack ptr
;
	jsr	stash_zp		; Save zero page bits
	tsx				; and save the 6502 stack ptr
	stx	istack_switched_sp	; in uarea/stacks
;
;	Hope the user hasn't used all the CPU stack
;
;	FIXME: we should check here if S is too low and if so set it high
;	and deliver SIGKILL
;
;	Configure the C stack to the i stack
;
	lda	#<istack_top
	sta	sp
	lda	#>istack_top
	sta	sp+1
	jsr	interrupt_handler
;
;	Reload the previous value into the stack ptr
;
	ldx	istack_switched_sp
	txs				; recover 6502 stack
	jsr	stash_zp		; restore zero page bits

	lda	saved_map		; we entered kernel map
	beq	exit_as_kernel		; so we exit that way
					; no premption or signals
;
;	TODO: pre-emption
;



;
;	Signal handling on 6502 is foul as the C stack may be inconsistent
;	during an IRQ. We push a new complete rti frame below the official
;	one, along with a vector and the signal number. The glue in the
;	app is expected to switch to a signal stack or similar, pop the
;	values, invoke the signal handler and then return.
;

	lda	_udata + U_DATA__U_CURSIG
	beq	irqout


	sta	$C009			; Back to the users S and ZP
	tay
	tsx
	txa
	sec
	sbc	#6			; move down past the existing rti
	tax
	txs
	lda	#>irqout
	pha
	lda	#<irqout
	pha				; stack a return vector
	tya
	pha				; stack signal number
	ldx	#0
	stx	_udata + U_DATA__U_CURSIG
	asl	a
	tay
	lda	_udata + U_DATA__U_SIGVEC,y	; Our vector (low)
	pha					; stack half of vector
	lda	_udata + U_DATA__U_SIGVEC+1,y	; High half
	pha					; stack rest of vector
	txa
	sta	_udata + U_DATA__U_SIGVEC,y	; Wipe the vector
	sta	_udata + U_DATA__U_SIGVEC+1,y
	lda	#<PROGLOAD + 20
	pha
	lda	#>PROGLOAD + 20
	lda	#0
	pha				; dummy flags, with irq enable
	rti	    			; return on the fake frame
					; if the handler returns
					; rather than doing a longjmp
					; we'll end up at irqout and pop the
					; real frame
irqout:
	jsr	map_restore
exit_as_kernel:
	pla
	tay
	pla
	tax
	pla
	rti
;
;	    sp/sp+1 are the C stack of the userspace
;	    with the syscall number in X
;	    Y indicates the number of bytes of argument
;
syscall_entry:
	php
	sei
	cld

	stx	_udata + U_DATA__U_CALLNO

	; No arguments - skip all the copying and stack bits
	cpy	#0
	beq	noargs

	; Remove the arguments. This is fine as by the time we go back
	; to the user stack we'll have finished with them
	lda	sp
	sta	ptr1
	ldx	sp+1
	stx	ptr1+1
	jsr	incaxy
	sta	sp
	stx	sp+1

	;
	;	We copy the arguments but need to deal with the compiler
	;	stacking in the reverse order. At this point ptr1 points
	;	to the last byte of the arguments (first argument). We go
	;	down the stack copying words up the argument list.
	;
	ldx	#0
copy_args:
	dey
	lda	(ptr1),y		; copy the arguments over
	sta	_udata + U_DATA__U_ARGN+1,x
	dey
	lda	(ptr1),y
	sta	_udata + U_DATA__U_ARGN,x
        inx
	inx
	cpy	#0
	bne	copy_args
noargs:
	;
	; Now we need to stack switch. Save the adjusted stack we want
	; for return
	;
	lda	sp
	pha
	lda	sp+1
	pha
	tsx
	stx	_udata + U_DATA__U_SYSCALL_SP
;
;	We save a copy of the high byte of sp here as we may need it to get
;	the brk() syscall right.
;
	sta _udata + U_DATA__U_SYSCALL_SP + 1
;
;	Now switch to the kernel ZP/S
;	TODO: sort this mapping out properly
;
	sta $C008
	ldx #$FF
	txs
;
;
;	Set up the C stack
;
	lda	#<kstack_top
	sta	sp
	lda	#>kstack_top
	sta	sp+1

	cli
;
;	Caution: We may enter here and context switch and another task
;	exit via its own syscall returning in its own memory context.
;
;	Don't assume anything we stored statically *except* the uarea
;	will be different. The uarea is banked in and out (or copied in
;	more awkward systems).
;
	jsr	unix_syscall_entry

	sei
;
;	Correct the system stack
;
	ldx	_udata + U_DATA__U_SYSCALL_SP

	sta	$C009			; Switch to user stsck and Zp

	txs
;
;	From that recover the C stack and the syscall buf ptr
;
	pla
	sta	sp+1
	pla
	sta 	sp
	lda	_udata + U_DATA__U_CURSIG
	beq	syscout
	tay

	tsx				; Move past existing return stack
	dex
	dex
	dex
	txs

	;
	;	The signal handler might make syscalls so we need to get
	;	our return saved and return the right value!
	;
	lda	_udata + U_DATA__U_ERROR
	pha
	lda	_udata + U_DATA__U_RETVAL
	pha
	lda	_udata + U_DATA__U_RETVAL+1
	pha
	lda	#>sigret		; Return address
	pha
	lda	#<sigret
	pha

	tya
	pha				; signal
	ldx	#0
	stx	_udata + U_DATA__U_CURSIG
	asl	a
	tay
	lda	_udata + U_DATA__U_SIGVEC,y	; Our vector
	pha
	lda	_udata + U_DATA__U_SIGVEC+1,y
	pha
	txa
	sta	_udata + U_DATA__U_SIGVEC,y	; Wipe the vector
	sta	_udata + U_DATA__U_SIGVEC+1,y

	; Invoke the helper with signal and vector stacked
	; it will then return to syscout and recover the original
	; frame. If the handler made syscalls then
	jmp (PROGLOAD + 20)

	;
	; FIXME: should loop for more signals if appropriate
	;
sigret:
	pla		; Unstack the syscall return pieces
	tax
	pla
	tay
	pla
	plp		; from original stack frame
	rts

syscout:
;	We may be in decimal mode beyond this line.. take care
;
	plp

;	Copy the return data over
;
	ldy _udata + U_DATA__U_RETVAL
	ldx _udata + U_DATA__U_RETVAL+1
;	Also sets Z for us
	lda _udata + U_DATA__U_ERROR

	rts

;
;	IRQ must be off on entry
;
plt_doexec:
;
;	Start address of executable
;
	stx	ptr3+1		; Point ptr3 at base
	sta	ptr3
	stx	ptr2+1		; Point ptr2 at base + 0x10
	lda	#16
	sta	ptr2
	ldy	#0
	lda	(ptr2),y	; Get the signal vector pointer
	sta	PROGLOAD+16	; if we loaded high put the vector in
	iny
	lda	(ptr2),y
	sta	PROGLOAD+17	; the low space where it is expected

	lda	ptr2

;
;	    Switch zero page and CPU stack to user - TODO
;
	sta $C009		; Auxiliary stack and SP selected

	    stx ptr1+1
	    sta ptr1


;
;	Set up the C stack. FIXME: assumes for now our sp in ZP matches it
;
	lda	_udata + U_DATA__U_ISP
	sta	sp
	ldx 	_udata + U_DATA__U_ISP+1
        stx	sp+1

;
;	Set up the 6502 stack: TODO
;
	ldx #$ff
	txs
;
;	Relocation is done in kernel because the 6502 isn't quite smart
;	enough to do its own ZP relocations as far as I can tell (prove me
;	wrong....)
;
	jsr _relocator

	jmp (ptr3)		; Enter user application

;
;	Straight jumps no funny banking issues yet
;	If we start using the alternate language page we'll need to save
;	and restre that state plus have stubs in both copies.
;
_unix_syscall_i:
	jmp _unix_syscall
_plt_interrupt_i:
	jmp _plt_interrupt

	.segment "COMMONDATA"

	.export _statusdata
_hd_map:
	.res 1
_statusdata:
	.res 8

	.code
	; Dummy stubs for now
	.export _block_rw_pascal
	.export _block_rw_prodos
	.export _pascal_status
_block_rw_pascal:
_block_rw_prodos:
	rts

_pascal_status:
	rts
