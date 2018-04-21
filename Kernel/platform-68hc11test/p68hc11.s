;
;	    68HC11 Simulation Platform 
;

        .file "p68hc11"
	.mode mshort

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl interrupt_handler
        .globl program_vectors
	.globl map_kernel
	.globl map_process
	.globl map_process_always
	.globl map_save
	.globl map_restore
	.globl outchar

        ; exported debugging tools
        .globl platform_monitor
        .globl platform_reboot

	include "cpu.def"
	include "eeprom.def"
        include "kernel.def"
        include "../kernel-hc11.def"

        .sect .data

trapmsg:
	.ascii "Trapdoor: SP="
	.byte 0
trapmsg2:
	.ascii ", PC="
        .byte 0
tm_user_sp:
	.word 0

savedbank:
	.word 0

	.sect .text
platform_monitor:
	sei
	bra platform_monitor

platform_reboot:
	jmp reboot

outchar:
	psha
outchar1:
	ldaa scsr
	anda #0x80
	beq outchar
	stab scdr
	pula
init_early:
	rts

init_hardware:
	; set system RAM size
	ldd ram		; Size from firmware
	std ramsize
	clc
	sbcb #64
	sbca #0
	std procmem	; 64K is lost to kernel

	; Our vectors are in high memory unlike Z80 but we still
	; need vectors
	clra
	clrb		; pass NULL
	jsr program_vectors
	rts


program_vectors:
;
;	FIXME: figure out how this will interact with eeprom (eeprom should
;	bank switch then call into some secondary vector)
;
	rts	; rti will be in eeprom

;
;	Memory is fully banked, with no real common. We simply track the
;	"correct" user bank for use by the copiers.
;
map_process_always:
	psha
	ldaa U_DATA__U_PAGE
	staa usrbank
	pula
	rts

map_kernel:
	clr usrbank
	rts

map_restore:
	psha
	ldaa savedbank
	pula
	rts
map_save:
	psha
	ldaa curbank
	staa savedbank
	pula
	rts

map_process:
	xgdx
	ldaa P_TAB__P_PAGE_OFFSET,x
	staa usrbank
	xgdx
	rts

;
;	Bank handling
;
	.globl doexec
	.globl sigdispatch
	.globl _ugetc
	.globl _ugetw
	.globl _uget
	.globl _uputc
	.globl _uputw
	.globl _uputs
	.globl _uput
	.globl _uzero


;
;	D = user address
;
doexec:
	xgdx		; function into X
	ldab usrbank	; bank
	ldy U_DATA__U_ISP
	jsr farcall
;
;	If this returns we should probably do an exit call or something
;	FIXME
;
	jmp platform_monitor


;
;	D = user address
;
sigdispatch:
	xgdx
	ldy U_DATA__U_SYSCALL_SP
	ldab usrbank
	; This may not return which is fine
	jmp farcall
	; Signal handler completed and returned back to the eeprom and thus
	; banked back to kernel so returns to our caller

;
;	We have no common but instead route far accesses via the eeprom
;	helpers in the firmware. X points at the stackframe and B holds
;	the previous bank.
;
_ugetc:
	xgdx
	ldaa usrbank
	jsr fargetb
	clra
	rts
_ugetw:
	xgdx
	ldaa usrbank
	jmp fargetw

;
;	D = src, 2(s) = dest, 4(s) = size
;
_uget:
	tsx
	xgdy		; D was src, we want it in Y
	ldd 4,x		; size
	std tmp1	; in tmp1
	ldx 2,x		; destination in X
	clrb		; 0 = kernel
	ldaa usrbank	; user space
	jmp farcopy
;
;	D = value, 2(s) = dst
;
_uputc:
	tsx
	ldx 2,x		; x is dst
	xgdy		; value into y
	ldaa usrbank
	jmp farputb

_uputw:
	tsx
	ldx 2,x		; x is dst
	xgdy		; value into y
	ldaa usrbank
	jmp farputw
;
;	D = src, 2(s) = dst, 4(s) = size
;
_uput:
	tsx
	xgdy		; D was src, we want it in Y
	ldd 4,x		; size
	std tmp1	; in tmp1
	ldx 2,x		; destination in X
	clra		; 0 = kernel
	ldab usrbank	; user space
	jmp farcopy
_uzero:
	tsx
	ldy 2,x		; length
	xgdx		; pointer
	ldaa usrbank
	jmp farzero

	rts

;
;	System call and interrupt vectors (via the eeprom firmware. It does
;	the bank switches to 0 and return and the rti).
;
;	We only use SCI rx, timer and external irqs
;	plus illegal opcodes and swi for syscall stuff
;
;	Y points to the far stack frame, B holds the bank we trapped from
;
;	FIXME: if we resched in the interrupt handler we break the eeprom
;	irq vector handling assumptions. Need to rework this lot so we don't
;	return via the eeprom irq path ?
;


serial_int:
	ldaa #1
do_int:
	staa irqsrc
	jsr interrupt_handler
	clr irqsrc
	tst U_DATA__U_CURSIG
	bne int_signal
	rts
timer_int:
	ldaa #2
	bra do_int
irq_int:
	ldaa #3
	bra do_int

int_signal:
	jsr dispatch_process_signal
	rts
;
;	Synchronous events. This is ok to use tmp1 as we are not an
;	irq affecting the kernel
;
;	On entry the Y register points to the far stack frame and A holds
;	the bank we trapped from (and thus it's stack)
;
;	We are on a small trap handling stack with interrupts off. We must
;	get off that stack stub
;
;	We need to copy the trap frame (9 bytes), above that is a return
;	address (user space) and then arguments 2-4 (arg1 is in the trap
;	frame X). Total 17 bytes
;
swi_int:
	sty U_DATA__U_SYSCALL_SP ; userspace return stack
	lds #kstack_top		; switch to kernel stack
	clrb			; copy to kernel
	ldx #17			; bytes to copy
	stx tmp1
	ldx #paramcopy
	jsr farcopy
	ldx #paramcopy
	ldd 4,x			; stack frame X reg (arg1)
	std U_DATA__U_ARGN
	ldd 11,x		; arg 2
	std U_DATA__U_ARGN1
	ldd 13,x
	std U_DATA__U_ARGN2
	ldd 15,x
	std U_DATA__U_ARGN3
	ldd 4,x
	std U_DATA__U_CALLNO
	cli
	jsr unix_syscall_entry
	sei
	ldx U_DATA__U_ERROR
	ldd U_DATA__U_RETVAL
	tst U_DATA__U_CURSIG
	beq nosig
	psha
	pshb
	pshx
	;
	;	FIXME: sort out the actual signal delivery logic
	;
	jsr dispatch_process_signal
	pulx
	pulb
	pula
nosig:
	stx ret_x		; registers for farjmp return
	std ret_d

	ldy U_DATA__U_SYSCALL_SP
	iny			; eat return address
	iny
	cli
	rts

	.sect .data
irqsrc:	.byte 0

.comm	paramcopy,17,1
