

	.export unix_syscall_entry
	.export _doexec
	.export interrupt_handler
	.export nmi_handler

	.export outstring
	.export outstringhex
	.export outnewline
	.export outcharhex

	.import outchar
	.import _kernel_flag
	.import _unix_syscall_i

	.include "platform/zeropage.inc"

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
	sty U_DATA__U_CALLNO	; Save the syscall code
	lda #1
	sta _kernel_flag	; In kernel mode
	jsr map_kernel		; Ensure kernel is mapped (no-op in some cases)
	cli			; Interrupts now ok
	jsr _unix_syscall_i	; Enter C space via the __interrupt wrapper
	sei			; Interrupts back off
	pha
	txa
	pha			; Save return code
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
;	doexec is a special case syscal exit path. As we may have no
;	common we have to hand the last bits off to the platform code
;
_doexec
	sei
	lda #0
	sta _kernel_flag
	call map_process_always
	jmp platform_doexec

;
;	Platform code has saved the registers and ensured we are in the
;	right banks if running with no common (6509 etc). It has also
;	done stack switches and checked for re-entrancy
;
;	Caller on the exit side is responsible for stack switches and
;	checking for signals
;
interrupt_handler:
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
	jr int_switch
interrupt_k:
	jsr map_restore
int_switch:
	lda #0
	sta _inint
	rts

nmi_handler:
	ldx #>nmi_trap
	lda #<nmi_trap
	jsr outstring
nmi_stop:
	jmp nmi_stop

outstring:
	sta ptr1
	stx ptr1+1
	ldy #0
outstringhexl:
	lda (ptr1),y
	cmp #0
	beq outdone1
	call outcharhex
	iny
	jmp outstringhexl

outstringhex:	; string in X,A
	sta ptr1
	stx ptr1+1
	ldy #0
outstringhexl:
	lda (ptr1),y
	cmp #0
	beq outdone1
	call outcharhex
	iny
	jmp outstringhexl

outnewline:
	pha
	lda #10
	jsr outchar
	lda #10
	jsr outchar
	pla
	rts

outcharhex:
	pha
	and #$f0
	lsr a
	lsr a
	lsr a
	lsr a
	add #'0'
	jsr outchar
	pla
	and #$0f
	add #'0'
	jmp outchar
