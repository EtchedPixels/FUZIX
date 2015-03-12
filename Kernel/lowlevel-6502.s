

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
	.include "kernel02.def"

	.segment "COMMONMEM"
;
;	Unlike Z80 we need to deal with systems that have no overlapping
;	memory banks only a 'far write/far read' model (eg 6509). We pass
;	the arguments is a single pointer therefore we expect the platform
;	code to have copied the syscall arguments into udata then called us
;	it also saves any registers etc for us (as it will need them too)
;
;	Called with interrupts off, on the kernel stack
;	On completion U_DATA__U_ERROR an U_DATA__U_RETVAL hold the returns
;
unix_syscall_entry:
	jsr map_kernel		; Ensure kernel is mapped (no-op in some cases)
	lda #1
	sta _kernel_flag	; In kernel mode
	cli			; Interrupts now ok
	jsr _unix_syscall_i	; Enter C space via the __interrupt wrapper
	sei			; Interrupts back off
	lda #0
	sta _kernel_flag
unix_sig_exit:
	jsr map_process_always	; Map back process if we have common
	rts

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
	ldy #0
	sty _kernel_flag
	jsr map_process_always
	sei
	jmp platform_doexec

;
;	Platform code has saved the registers and ensured we are in the
;	right banks if running with no common (6509 etc). It has also
;	done stack switches and checked for re-entrancy
;
;	Caller on the exit side is responsible for stack switches and
;	checking for signals
;
;	The C world here is fairly ugly. We have to stash various bits of
;	zero page magic because its not re-entrant.
;
;	stash_zp must bappen *before* we change mapping so the zp stash
;	must be in common. The map routines are allowed to use _tmp1 and
;	_ptr1 which will upset userspace terribly if they are not saved and
;	properly restored!
;
interrupt_handler:
; Our caller will deal with ZP via stash_sp and any platform magic
	jsr map_save
	jsr map_kernel
	lda #1
	sta _inint
	jsr _platform_interrupt_i	; call via C int wrapper
	lda #0
	sta _inint
	lda _kernel_flag
	bne interrupt_k
	jsr map_process_always		; may have switched task
	jmp int_switch
interrupt_k:
	jsr map_restore
int_switch:
	lda #0
	sta _inint
	rts

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
