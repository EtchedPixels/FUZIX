#include "msp430fr5969.h"
#include "kernel.def"
#include "kernel430x.def"
#include "globals.h"

; imported symbols
.globl fuzix_main
.globl init_early
.globl init_hardware

; Set up the stacks.


.section ".noinit"

; The interrupt stack.
.globl istack_top
istack_base:
	.fill 254
istack_top:

; The saved interrupt pointer. Stacks must be in low memory.
.globl istack_switched_sp
.comm istack_switched_sp, 2

; The userdata/system stack structure. See Kernel/include/kernel.h.
.globl udata
udata:
	.fill 512
kstack_top:

.text
.globl __main
__main:
	; Disable watchdog timer.

	mov.w #(WDTPW | WDTHOLD), &WDTCTL

	mov.b #CSKEY>>8, &CSCTL0_H                      ; Unlock clock registers
	mov.w #DCOFSEL_3 | DCORSEL, &CSCTL1             ; DCO to 8MHz
	; ACLK = VLO; SMCLK = MCLK = DCO
	mov.w #(SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK), &CSCTL2
	mov.w #(DIVA__1 | DIVS__1 | DIVM__1), &CSCTL3

	; Configure the LFXT.
	mov.w #0, &PJSEL1
	mov.w #BIT4|BIT5, &PJSEL0
	bis.w #LFXTDRIVE_3|HFXTOFF, &CSCTL4
	bic.w #LFXTBYPASS|LFXTOFF, &CSCTL4

	; Wait for oscillators to settle.

1:
	bic.w #LFXTOFFG|HFXTOFFG, &CSCTL5
	bic.w #OFIFG, &SFRIFG1
	bit.w #OFIFG, &SFRIFG1
	jnz 1b
	
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
	
.globl abort
abort:
	bic.w #GIE, SR
	mov #1f, r12
	mov #kstack_top, sp
	calla #kprintf
	jmp .
.data
1: .asciz "[abort]\n"
.text

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

.globl last_interrupt
.comm last_interrupt, 1

#define standard_interrupt(name, irq) \
	.globl name ## _trampoline { \
	.section ".lowtext", "ax", @progbits { \
	name ## _trampoline: \
	mov.b #(irq), &last_interrupt { \
	bra #interrupt_handler { \
	.section #name, "ax", @progbits { \
		.word name ## _trampoline

#define fallback_interrupt(name) \
	.text { \
	1: mov #3f, r12 { \
	   mov #kstack_top, sp { \
	   calla #kprintf { \
	2: jmp 2b { \
	.data { \
	3: .asciz #name { \
	.section #name, "ax", @progbits { \
	   .word 1b

;fallback_interrupt("__interrupt_vector_comp_e")
fallback_interrupt(__interrupt_vector_unmi)
fallback_interrupt(__interrupt_vector_sysnmi)

standard_interrupt(__interrupt_vector_wdt, INTERRUPT_WDT)

