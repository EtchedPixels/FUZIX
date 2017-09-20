

	.P816
	.I8
	.A8
	
	.export unix_syscall_entry
	.export _doexec
	.export interrupt_handler
	.export nmi_handler

	.export outstring
	.export outstringhex
	.export outnewline
	.export outcharhex
	.export outxa
	.export stash_zp

	.export _need_resched

	.import outchar
	.import _kernel_flag
	.import _unix_syscall_i
	.import map_restore
	.import map_save
	.import map_process_always
	.import map_kernel
	.import _platform_interrupt_i
	.import platform_doexec
	.import _inint
	.import CTemp
	.import _trap_monitor

	.include "platform/zeropage.inc"
	.include "platform/kernel.def"
	.include "kernel816.def"

	.segment "COMMONMEM"
;
;	Unlike Z80 we need to deal with systems that have no overlapping
;	memory banks. We pass the arguments is a single pointer therefore
;	we expect the platform code to have copied the syscall arguments into
;	udata then called us it also saves any registers etc for us (as it will
;	need them too)
;
;	Called with interrupts off, on the kernel stack
;	On completion U_DATA__U_ERROR an U_DATA__U_RETVAL hold the returns
;
;	Caller is expected to set 65C816 to I8A8
;
;	FIXME: do we want 6502 binaries syscall or to implement a cleaner
;	65c816 brk based syscall ?
;
syscall_entry:
	php
	sei
	cld
	sep #$30
	.i16
	lda #KERNEL_BANK
	pha
	plb
	rep #$30
	stx U_DATA__U_CALLNO
	cpy #0
	beq noargs

	FIXME: inter bank move the arguments

noargs:
	rep #$10
	.i16
	ldx sp
	stx U_DATA__U_SYSCALL_SP
	tsx
	stx U_DATA__U_PAGE+2		; ewww.. FIXME
	; FIXME: kstack actually depends on process number so needs to be
	; a look up
	ldx #kstack_top
	stx sp	
	cli

	sep #$10
	.i8
	lda #1
	sta _kernel_flag	; In kernel mode
	cli			; Interrupts now ok
	jsr _unix_syscall_i	; Enter C space via the __interrupt wrapper
	sei			; Interrupts back off
	stz _kernel_flag
	rep #$10
	.i16
	ldx U_DATA__U_PAGE+2
	txs
	ldx U_DATA__U_SYSCALL_SP
	stx sp
	.i8
	sep #$10
	lda U_DATA__U_CURSIG
	bne signal_out
	plp
	; We may now be in decimal !
	ldy U_DATA__U_RETVAL
	ldx U_DATA__U_RETVAL+1
	; also sets z for us
	lda U_DATA__U_ERROR
	rts
signal_out:
	tay
	clz U_DATA__U_CURSIG
	rep #$10
	.i16
	tsx
	dex			; move past existing frame
	dex
	dex
	dex			; FIXME check is 4 bytes
	txs
	setp #$10
	.i8
	; Stack the signal return (the signal itself can cause syscalls)
	lda U_DATA__U_ERROR
	pha
	lda U_DATA__U_RETVAL
	pha
	lda U_DATA__U_RETVAL+1
	pha
	lda #>sigret		; needs to be a fixed address in user
	pha
	lda #<sigret		; FIXME
	pha
	phy
	asl a
	tay
	rep #$10
	.i16
	ldx U_DATA__U_SIGVEC,y
	clz U_DATA__U_SIGVEC,y
	phx
	lda U_DATA__U_PAGE
	pha
	ldx #PROGLOAD+20
	phx
	sep #$10
	.i8
	rtl	;	return into user app

;
;	On 6502 the platform code is responsible for invoking the
;	signal dispatch (as it may have to be in the stub in the
;	process space if we have no common)
;


;
;	doexec is a special case syscall exit path. As we may have no
;	common we have to hand the last bits off to the platform code
;	x,a holds the target address. This routine is in common and is the
;	one case we can and do want to have fastcall.
;
_doexec:
	; FIXME set up U_DATA__U_PAGE+2 here by lookup
	sei
	stz _kernel_flag
	; where to save address ... ? (or is it always bank:0 anyway ?)
	rep #$30
	.i16
	.a16
	lda U_DATA__U_PAGE+2	;	CPU stack
	tcs			;	Set CPU stack at xxFF
	ina			;	Zp follows
	xba			;	now in the form 00xx as we need
	pha
	pld
	; We are now on the correct DP and CPU stack
	ldy U_DATA__U_ISP
	sty sp			;	sp is in DP so we write user version
	sep #$10
	.i8
	.a8
	lda U_DATA__U_PAGE	;	bank
	pha
	lda #0			;	should be passed address but will
	pha			;	I think always be zero !
	pha
	cli
	tax			;	bank is 0
	tay			;	ZP base is 0
	rtl
	
;
;	The C world here is fairly ugly. We have to stash various bits of
;	zero page magic because its not re-entrant.
;
interrupt_handler:
	rep #$30
	pha
	phx
	phy
	sep #$30			; 8i8a
	cld

	; FIXME: rewrite this in 16i mode
	; Save our ZP in case we are in kernel mode and using it
	; (we could saacrifice 256 bytes to IRQ handling instead which
	; might be smarter FIXME)
	jsr stash_zp			; side effect saves sp

	.i16
	rep #$10
	tsx
	stx istack_switched_sp
	ldx istack
	txs
	ldx #istack
	stx sp				; C stack is now right

	sep #$10

	.i8

	lda #1
	sta _inint
	jsr _platform_interrupt_i	; call via C int wrapper
	stz _inint
	jsr map_process_always		; may have switched task
	jmp int_switch
int_switch:
	stz _inint

	; Restore the stack we arrived on

	.i16
	rep #$10
	ldx istack_sitched_sp
	txs
	sep #$10
	.i8
	jsr_stash_zp
	; TODO .. pre-emption

	; Signal return path
	; The basic idea here is that if a signal is pending we
	; build a new stack frame under the real one and rti to that. The
	; hook code in low user memory will then clean up the real frame
	lda U_DATA__U_CURSIG
	clz U_DATA__U_CURSIG
	bne signal_exit
	rep #$30
	ply
	plx
	pla
	rti

signal_exit:
	tay		; save signal 8bits


	; Move down the stack frame
	; FIXME: we need to mangle the frame to take out the page
	; so we can match syscall (or modify syscall!)
	.i16
	rep #$10
	tsx
	dex			; 7 bites FIXME check
	dex
	dex
	dex
	dex
	dex
	dex
	txs
	ldx #irqout
	phx			; return vector
	sep #$10
	.i8
	phy			; signal code
	tya
	asl a
	tay
	rep #$30
	.i16
	.a16
	lda U_DATA__U_SIGVEC,y
	clz U_DATA__U_SIGVEC,y
	pha
	lda #PROGLOAD+20
	pha
	sep #$30
	.i8
	.a8
	lda U_DATA__U_PAGE
	pha 
	lda #$30		; i8a8
	pha
	rti
;
;	We can make the map routines generic as all 65c816 are the same
;	mapping model. We don't change anything but instead track the page
;	we need to use for the'far' operations.
;
map_process_always:
	lda U_DATA__U_PAGE
	sta userpage
	rts
map_process:
	cmp #0
	bne map_process_2
	cmpx #0
	bne map_process_2
map_kernel:
	rts
map_process_2:
	sta ptr1
	stx ptr1+1
	lda (ptr1)	; 4 bytes if needed
	sta userpage
	rts
userpage:
	.byte 0

;
;	The following is taken from the debugger example as referenced in
;	the compiler documentation. We swap a stashed ZP in our commondata
;	with an IRQ handler one. The commondata is per process and we depend
;	upon this to make it all work
;
; Swap the C temporaries
;
stash_zp:
        ldy     #zpsavespace-1
Swap1:  ldx     CTemp,y
        lda     <sp,y
        sta     CTemp,y
        txa
        sta     sp,y
        dey
        bpl     Swap1
        rts

nmi_handler:
	ldx #>nmi_trap
	lda #<nmi_trap
	jsr outstring
nmi_stop:
	jmp _trap_monitor
nmi_trap:
	.byte "NMI!", 0

outstring:
	sta ptr1
	stx ptr1+1
	ldy #0
outstringl:
	lda (ptr1),y
	cmp #0
	beq outdone1
	jsr outchar
	iny
	jmp outstringl

outstringhex:	; string in X,A
	sta ptr1
	stx ptr1+1
	ldy #0
outstringhexl:
	lda (ptr1),y
	cmp #0
	beq outdone1
	jsr outcharhex
	iny
	jmp outstringhexl

outnewline:
	pha
	lda #10
	jsr outchar
	lda #10
	jsr outchar
	pla
outdone1:
	rts

outcharhex:
	pha
	and #$f0
	lsr a
	lsr a
	lsr a
	lsr a
	cmp #10
	bcc deci1
	clc
	adc #7
deci1:
	clc
	adc #48			; ascii zero
	jsr outchar
	pla
	and #$0f
	cmp #10
	bcc deci2
	clc
	adc #7
deci2:
	clc
	adc #48
	jmp outchar

outxa:	pha
	txa
	jsr outcharhex
	pla
	jmp outcharhex

_need_resched:
	.byte 0
