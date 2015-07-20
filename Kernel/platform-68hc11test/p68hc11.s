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


doexec:
sigdispatch:
_ugetc:
_ugetw:
_ugets:
_uget:
_uputc:
_uputw:
_uput:
_uzero:
	/* FIXME */
	rts
