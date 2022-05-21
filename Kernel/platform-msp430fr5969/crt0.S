#include "msp430fr5969.h"
#include "kernel.def"
#include "kernel430x.def"
#include "globals.h"

; imported symbols
.globl fuzix_main
.globl init_early
.globl init_hardware

.globl plt_reboot
.globl plt_monitor
proc poweron
	mov.w #(WDTPW | WDTHOLD), &WDTCTL               ; Disable watchdog

plt_reboot:
plt_monitor:
	dint

	; Copy the startup code out of high rom and into its execution address.
	mov #0, r15
1:
	movx.b __load_start_boot_overlay(r15), __overlay_start_address(r15)
	inc r15
	cmp.w #__load_length_boot_overlay, r15
	jnz 1b

	; Go to the real startup code.
	br #plt_reboot_high

proc plt_reboot_high
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
	mov.w #kstack_top, SP

	; Wipe the first BSS.
	mov.w #__bssstart, r12
1:
	clr.b @r12
	inc r12
	cmp.w #__bssend, r12
	jnz 1b

	; ...and the second BSS.
	mov.w #__bss2start, r12
1:
	clr.b @r12
	inc r12
	cmp.w #__bss2end, r12
	jnz 1b

	; Now copy the initial values of the data segment out of high rom.
	mov.w #0, r12
1:
	movx.b __romdata_start(r12), __romdata_dest(r12)
	inc r12
	cmp #__romdata_length, r12
	jnz 1b
	
	; Set some kernel variables.
	mov.w #64, &ramsize
	mov.w #__user_length_kb, &procmem
	
	; Call C routines.
	br #plt_init

proc di
	; Return the old interrupt status in r12.
	mov SR, r12
	and #~GIE, r12
	bic.w #GIE, SR
	ret

proc irqrestore
	bic.w #GIE, SR           ; Ensure off
	bis.w r12, SR            ; Mask in saved value
	ret

.section ".syscall", "ax", @progbits
	.word unix_syscall_entry

.section "__interrupt_vector_reset", "ax", @progbits
	.word poweron

#define standard_interrupt(name, irq) \
	proc name ## _trampoline { \
	   mov.b #(irq), &last_interrupt { \
	   br #interrupt_handler { \
	.section #name, "ax", @progbits { \
		.word name ## _trampoline

#define fallback_interrupt(name) \
	proc name ## _fallback { \
	1: push #2f { \
	   br #fallback_interrupt_error { \
	.section #name, "ax", @progbits { \
	2: .word 1b

.global fallback_interrupt_error
proc abort
	push.w #0
fallback_interrupt_error:
	mov.w #kstack_top, sp
	mov.w #1f, r12
	call #kprintf
	jmp .
.section ".rodata"
1: .asciz "[I%d!]"

;fallback_interrupt("__interrupt_vector_comp_e")
fallback_interrupt(__interrupt_vector_unmi)
fallback_interrupt(__interrupt_vector_sysnmi)

standard_interrupt(__interrupt_vector_wdt, INTERRUPT_WDT)
standard_interrupt(__interrupt_vector_usci_a0, INTERRUPT_USCI_A0)

