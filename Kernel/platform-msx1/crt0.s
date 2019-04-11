	        ; Ordering of segments for the linker.
		; ROM segments first
	        .area _CODE
		.area _LOW
		.area _HEADER
	        .area _CODE2
		; We need this to be above 0x8000
		.area _HOME
		.area _VIDEO
		; RAM based or may be copied to/from by the user
	        .area _COMMONMEM
	        .area _CONST
	        .area _INITIALIZED
	        .area _GSINIT
	        .area _GSFINAL
	        .area _BSEG
	        .area _BSS
	        .area _HEAP
	        .area _DATA
		; Need to do dynamic buffers yet
		.area _DISCARD
	        .area _INITIALIZER

		.globl interrupt_handler
		.globl null_handler
		.globl unix_syscall_entry

		.globl do_set_sub_slot
		.globl do_get_sub_slot

		.globl rst30
		.globl rst38		; for checking
;
;	First _CODE section
;
;	We put the bank switch stub in here and matching in RAM
;
		.area _CODE

start:		jp null_handler
		.ds 0x2d
rst30:
		jp unix_syscall_entry
	        .ds 5
rst38:		jp interrupt_handler
		; We only have 0x3B-0x4F free
do_set_sub_slot:
		in e,(c)		; Get bank map
		out (c),b		; Set bank map
		ld (hl),a		; Set slots
		jr do_get_sub_slot_2
do_get_sub_slot:
		in e,(c)
		out (c),b
do_get_sub_slot_2:			; Get complemented slot bits
		ld a,(hl)
		out (c),e
		ret
		; 4A-4F free

; Just so we don't pack the binary

		.area _PAGE0
