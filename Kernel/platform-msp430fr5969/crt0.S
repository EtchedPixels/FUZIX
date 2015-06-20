#include "msp430fr5969.h"
#include "kernel430x.def"

; imported symbols
.globl fuzix_main
.globl init_early
.globl init_hardware

; Set up the stacks.


.section ".noinit"
; The kernel stack.
kstack_base:
	.fill 256
kstack_top:

; The interrupt stack.
istack_base:
	.fill 254
istack_top:

; The saved interrupt pointer.
.globl istack_switch_sp
.comm istack_switch_sp, 2

; The userdata structure.
.globl udata
.comm udata, U_DATA__U_TOP - U_DATA__U_PTAB

.text
.globl __main
__main:
	; Disable watchdog timer.

	mov.w #(WDTPW | WDTHOLD), &WDTCTL

	mov.b #CSKEY>>8, &CSCTL0_H                      ; Unlock clock registers
	mov.w #DCOFSEL_3 | DCORSEL, &CSCTL1             ; DCO to 8MHz
	; ACLK = VLO; SMCLK = MCLK = DCO
	mov.w #(SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK), &CSCTL2
	mov.w #(DIVA__1 | DIVS__1 | DIVM__1), &CSCTL3
	mov.b #0, &CSCTL0_H                             ; Relock clock registers

	; Init stack.
	mov #kstack_top, SP

	; Wipe BSS.
	mov #__bssstart, r12
	mov #0, r13
	mov #__bsssize, r14
	calla #memset

	; Wipe the extra, big bit of BSS.
	mov #__bigbssstart, r12
	mov #0, r13
	mov #__bigbsssize, r14
	calla #memset

	; Set some kernel variables.
	mov.w #64, &ramsize
	mov.w #32, &procmem

	; Call C routines.
	calla #tty_rawinit
	calla #fuzix_main
stop:
	jmp stop

__reti:
	reti

.globl trap_monitor
trap_monitor:
	bic.w #GIE, SR
	jmp trap_monitor
	
.globl ei
ei:
	bis.w #GIE, SR
	reta

.globl di
di:
	; Return the old interrupt status in r12.
	mov SR, r12
	and #~GIE, r12
	bic.w #GIE, SR
	reta

.globl irqrestore
irqrestore:
	bic.w #GIE, SR           ; Ensure off
	bis.w r12, SR            ; Mask in saved value
	reta

.section "__interrupt_vector_reset", "ax", @progbits
	.word __main
.text

#define fallback_interrupt(name) \
	.text { \
	1: mov #3f, r12 { \
	   mov #kstack_top, sp { \
	   calla #kprintf { \
	2: jmp 2b { \
	.data { \
	3: .asciz name { \
	.section name, "ax", @progbits { \
	   .word 1b

;fallback_interrupt("__interrupt_vector_comp_e")
fallback_interrupt("__interrupt_vector_unmi")
fallback_interrupt("__interrupt_vector_sysnmi")

