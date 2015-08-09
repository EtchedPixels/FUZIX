        	; imported symbols

		.file "crt0.s"
		.mode mshort

        	.globl fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl kstack_top
		.globl _start

	        ; startup code @0
	        .sect .start
_start:
		jmp start

		.sect .text
start:		
		sei
		lds #kstack_top
		ldx #edata
wipe:		clr ,x
		inx
		cmpx #_end
		bne wipe
;
;	FIXME: any set up needed ?
;

		jsr init_early
		jsr init_hardware
		jsr fuzix_main
		sei
stop:		bra stop

;
;	Zeropage compiler goo
;

	.sect .page0

		.globl _.frame
		.globl _.tmp
		.globl _.d0
		.globl _.d1
		.globl _.d2
		.globl _.d3
		.globl _.d4
		.globl _.d5
		.globl _.xy
		.globl _.z

.equ _.frame,0x40
.equ _.tmp,0x42
.equ _.d0,0x44
.equ _.d1,0x46
.equ _.d2,0x48
.equ _.d3,0x4A
.equ _.d4,0x4C
.equ _.d5,0x4E
.equ _.xy,0x50
.equ _.z,0x52
