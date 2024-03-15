	        ; startup code
	        .code

		; Loaded at 0x0100 - 0xBFFF
start:
		di
		ld sp, kstack_top
		; Unpack the common, commondata and discard into the right
		; places - including in the upper shared RAM area
		ld hl, __bss
		ld de, __common
		ld bc, __common_size
		ldir
		ld de, __commondata
		ld bc, __commondata_size
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

;		Zero buffers area
		ld hl, __buffers
		ld de, __buffers+1
		ld bc, __buffers_size - 1
		ld (hl), 0
		ldir

		; and go....

		call init_early
		call init_hardware
		call _fuzix_main
		di
stop:		halt
		jr stop
