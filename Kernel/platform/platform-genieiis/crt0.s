	; Ordering of segments for the linker.
	.area _CODE
	.area _CODE2
	.area _VIDEO
	.area _BUFFERS
	.area _DISCARD
	.area _BSEG
	.area _BSS
	.area _HEAP
	.area _GSINIT
	.area _GSFINAL
	.area _CONST
	.area _INITIALIZED
	.area _DATA
	.area _COMMONMEM
	.area _COMMONDATA
	.area _INITIALIZER
	.area _PAGE0
	; Buffers must be directly before discard as they will
	; expand over it

        ; imported symbols
        .globl _fuzix_main
	.globl init_hardware
	.globl _vtinit
	.globl kstack_top

	.globl s__DATA
	.globl l__DATA

	.module	crt0
	; startup code
	.area _CODE

;
;	Once the loader completes it jumps here
;	Currently we are not packing the binary - we may want to ?
;
start:
	ld	sp, #kstack_top	; just below us
	; then zero the data area
	ld	hl, #s__DATA
	ld	de, #s__DATA + 1
	ld	bc, #l__DATA - 1
	ld	(hl), #0
	ldir
	call	init_hardware
	call	_vtinit
	call	_fuzix_main
	di
stop:	halt
	jr	stop
