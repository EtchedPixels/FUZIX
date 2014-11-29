	        ; Ordering of segments for the linker.
	        ; WRS: Note we list all our segments here, even though
	        ; we don't use them all, because their ordering is set
	        ; when they are first seen.	

        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl kstack_top

	        ; startup code @0
	        .area .start

start:
		jmp main

bootme:
		lda 0xff22		; Switcher is in both images
		anda #0xFB		; at the same address
		sta 0xff22		; ROM switch
		jmp main

		.area .text

main:		orcc #0x10		; interrupts definitely off
		lds #kstack_top

		jsr init_early
		jsr init_hardware
		jsr _fuzix_main
		orcc #0x10
stop:		bra stop

