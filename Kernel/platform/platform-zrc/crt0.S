	        ; startup code
	        .code

		; Load at 0x0100
start:
		jp go
		.word 0x10DE
go:
		di
		ld sp, kstack_top
		ld hl, __bss
		ld de, __common
		ld bc, __common_size
		ldir
		ld de, __discard
		ld bc, __discard_size - 1
		ex de,hl
		add hl,bc
		ex de,hl
		add hl,bc
		lddr
		ldd

		; Zero the data area
		ld hl, __bss
		ld de, __bss + 1
		ld bc, __bss_size - 1
		ld (hl), 0
		ldir
#if 0
;		Zero buffers area
		ld hl, s__BUFFERS
		ld de, s__BUFFERS + 1
		ld bc, l__BUFFERS - 1
		ld (hl), 0
		ldir
#endif
		call init_early
		call init_hardware
		call _fuzix_main
		di
stop:		halt
		jr stop
