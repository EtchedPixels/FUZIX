        ; startup code
	.common

	; Load at 0x0080
start:
	di
	ld sp, kstack_top
	; zero the BSS area
	ld hl, __bss
	ld de, __bss + 1
	ld bc, __bss_size - 1
	ld (hl), 0
	ldir
;	Zero buffers area
	ld hl, __buffers
	ld de, __buffers + 1
	ld bc, __buffers_size - 1
	ld (hl), 0
	ldir
	call init_early
	call init_hardware
	call _fuzix_main
	di
stop:	halt
	jr stop
