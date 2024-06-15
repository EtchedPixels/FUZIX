        ; exported symbols
        .export init
        .export init_from_rom
        .export _boot_from_rom

#include "kernelu.def"

        ; startup code
	.code

init:                       ; must be at 0x88 -- warm boot methods enter here
        xor a
        jr init_common
init_from_rom:              ; must be at 0x8B -- bootrom.s enters here
        ld a, 1
        ; fall through
init_common:
        di
        ld (_boot_from_rom), a
        or a
        jr nz, mappedok     ; bootrom.s loads us in the correct pages

        ; move kernel to the correct location in RAM
        ; note that this cannot cope with kernel images larger than 48KB
        ld hl, 0x0000
        ld a, 32               ; first page of RAM is page 32
movenextbank:
        out (MPGSEL_3), a       ; map page at 0xC000 upwards
        ld de, 0xC000
        ld bc, 0x4000          ; copy 16KB
        ldir
        inc a
        cp 35                  ; done three pages?
        jr nz, movenextbank

	; setup the memory paging for kernel
        out (MPGSEL_3), a       ; map page 35 at 0xC000
        ld a, 32
        out (MPGSEL_0), a       ; map page 32 at 0x0000
        inc a
        out (MPGSEL_1), a       ; map page 33 at 0x4000
        inc a
        out (MPGSEL_2), a       ; map page 34 at 0x8000

mappedok:
        ; switch to stack in high memory
        ld sp, kstack_top

        ; move the common memory where it belongs    
        ld hl, __bss
        ld de, __common
        ld bc, __common_size
        ldir
        ; and the discard (which might overlap)
        ld de, __discard
        ld bc, __discard_size
	add hl,bc
	ex de,hl
	add hl,bc
	ex de,hl
        lddr
	ldd
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
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop

_boot_from_rom:
	.byte 0
