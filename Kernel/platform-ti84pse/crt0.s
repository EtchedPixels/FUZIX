	.module crt0
	.area _CODE
	.area _CODE2
	.area _CONST
	.area _INITIALIZED
	.area _DATA
	.area _BSEG
	.area _BSS
	.area _HEAP
	.area _INITIALIZER
	.area _GSINIT
	.area _GSFINAL
	.area _DISCARD
	.area _COMMONMEM
	.area _FONT
	.area _HOME
	.area _PAGE0

	; imported symbols
	.globl _fuzix_main
	.globl init_hardware
	.globl s__INITIALIZER
	.globl s__INITIALIZED
	.globl l__INITIALIZER
	.globl s__COMMONMEM
	.globl l__COMMONMEM
	.globl s__DISCARD
	.globl l__DISCARD
	.globl s__DATA
	.globl l__DATA
	.globl kstack_top 

	.globl _init_display

        .area _CODE
	; 0x0000
	; XXX: This should be a separate module, probably
_rst00:
	; TODO: This should jump to #null_handler post-boot
	jp init
	.ds 5
_rst08:
	.ds 8
_rst10:
	.ds 8
_rst18:
	.ds 8
_rst20:
	.ds 8
_rst28:
	.ds 8 ; XXX: panic kernel here for unknown restart?
_rst30:
	.ds 8 ; TODO: syscall entry
_rst38:
	; Mode 1 interrupt handler
	.ds 8

	.ds 19
_boot:
	; ASIC jumps here
	jp init
bootmagic:
	.db 0xFF, 0xA5, 0xFF

init:
	di

	; Memory mode 0
	xor a
	out (4), a

	; Map RAM page 00 in bank C and Flash 04 in bank B
	out (5), a
	ld a, #3
	out (7), a
	; Copy to populate high RAM
	ld hl, #0x8000
	ld de, #0xC000
	ld bc, #0x4000
	ldir

	; Enable normal kernel memory map
	ld a, #1
	out (6), a
	inc a
	out (7), a

	; Map _COMMONMEM from page 80
	ld a, #64
	out (0x27), a
	; TODO: ^ Implement this in z80e

	; then zero the data area
	ld hl, #s__DATA
	ld de, #s__DATA + 1
	ld bc, #l__DATA - 1
	ld (hl), #0
	ldir

	ld sp, #kstack_top

	; Deal with flash unlocking bits
	ld a, #0x7C
	out (7), a
	call #0x8000 ; unlock_flash

	; Make all RAM executable
	xor a
	out (0x25), a
	ld a, #0xFF
	out (0x26), a

	call #0x8003 ; lock_flash
	ld a, #3
	out (7), a

	call init_hardware

	call _fuzix_main

stop:	di ; Unreachable
	halt
	jr stop
