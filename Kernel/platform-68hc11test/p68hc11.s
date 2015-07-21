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

        ; exported debugging tools
        .globl trap_monitor
        .globl trap_reboot

        include "kernel.def"
        include "../kernel-hc11.def"

;
;	TODO: hardware registers
;

	.globl baud
.equ baud,0xf000
	.globl sccr1
.equ sccr1,0xf000
	.globl sccr2
.equ sccr2,0xf000
	.globl scdr
.equ scdr,0xf000
	.globl scsr
.equ scsr,0xf000

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
trap_monitor:
	sei
	bra trap_monitor

trap_reboot:
	jmp reboot

init_early:
	rts

init_hardware:
	; set system RAM size
	ldd #512
	std ramsize
	ldd #512-64
	std procmem

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
	.globl _ugets
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
	ldaa usrbank	; bank
	ldy U_DATA__U_ISP
	jsr farcall
;
;	If this returns we should probably do an exit call or something
;	FIXME
;
	jmp trap_monitor


;
;	D = user address
;
sigdispatch:
	xgdx
	ldy U_DATA__U_SYSCALL_SP
	ldaa usrbank
	; This may not return which is fine
	jmp farcall
	; Signal handler completed and returned back to the eeprom and thus
	; banked back to kernel so returns to our caller

;
;	We have no common but instead route far accesses via the eeprom
;	helpers in the firmware.
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
_ugets:
	tsx
	xgdy		; D was src, we want it in Y
	ldd 4,x		; size
	std tmp1	; in tmp1
	ldx 2,x		; destination in X
	clrb		; 0 = kernel
	ldaa usrbank	; user space
	jmp farzcopy	; returns error/size in D
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
