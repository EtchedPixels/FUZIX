
       .include 'tuart.s'
;
;	Interrupt handlers. These will get replaced by some proper FIFO
; logic
;

	.area _COMMONMEM
	.module vector

	.globl interrupt_high
	.globl _tuart0_timer4

_tuart0_timer4:
	jp interrupt_high

tuart_ports    0
tuart_ports    1
tuart_ports    2

       .area _COMMONMEM

; Console tuart
tuart_handler_im2 0 0 res reti
; Additional ports on cards
tuart_handler_im2 1 0x80 set reti
tuart_handler_im2 2 0x90 res reti
