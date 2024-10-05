        ; startup code
	.code

init:
        di
        ld sp, kstack_top

        ; Configure memory map
        call init_early

	; move the common memory where it belongs    
	ld hl, __bss
	ld de, __common
	ld bc, __common_size
	ldir
	ld de, __commondata
	ld bc, __commondata_size
	ldir
	; and the discard
	ld de, __discard
	ld bc, __discard_size
	ldir
	; then zero the data area
	ld hl, __bss
	ld de, __bss + 1
	ld bc, __bss_size - 1
	ld (hl), 0
	ldir

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; main shouldn't return, but if it does...
        di
stop:   halt
        jr stop
