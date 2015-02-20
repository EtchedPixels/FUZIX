		;
		;  bootstrap for ASCII 8kb mapped rom
		;
		.area _BOOT

		.db 'A'
		.db 'B'
		.dw bootstrap
		.dw 0,0,0,0,0,0

		.include "msx2.def"
		.globl find_ram
		.globl enaslt

		; At this point the BIOS has detected the cartridge AB signature and
		; jumped here; we have the rom bios in slot_page 0, ram in slot_page 3, and rom
		; in slot_page 1 (this code).
		;
		; Kernel is in mapped ROM pages 0x02 to 0x08, this code is in page 0x00
		; We just set RAM in slot_page 2 and copy over the kernel in chunks of 8K
		;
bootstrap:
		di
		ld sp,#0xf340       ; temporary stack

		; FIXME: init vdp using bios so that font is in place after vdpinit
		ld a,#0x50
		ld (0xf3ae),a
		ld ix,#0x00d5
		call #0x015f

		;
		; set ram in slot_page 2
		;
		call find_ram
		ld hl,#PAGE2_BASE
		call enaslt

		ld b,#3	    ; starting ram page (copy to 3,2,1,4)
		ld c,#2	    ; starting rom page (copy from 2,3,4,5,6,7,8,9)

nextrampage:
		ld a, b
		out (RAM_PAGE2),a

		ld a,c
		ld (ASCII8_ROM_PAGE1),a

		exx
		ld hl, #ASCII8_PAGE1_BASE
		ld de, #PAGE2_BASE
		ld bc, #0x2000
		ldir
		exx

		inc c
		ld a,c
		ld (ASCII8_ROM_PAGE1),a

		exx
		ld hl, #ASCII8_PAGE1_BASE
		ld de, #PAGE2_BASE+0x2000
		ld bc, #0x2000
		ldir
		exx

		inc c
		dec b
		ld a,b
		cp #3
		jr z, done
		or a
		jr nz, nextrampage
		ld b, #4
		jr nextrampage
done:
		;
		; Collect BIOS info bits and stash to ram
		;
		call find_ram		; this requires ram page 0 in slot_page 3
		ld d,a
		call find_rom
		ld e,a

		ld a, #4
		out (RAM_PAGE3),a

		ld bc,(BIOS_VERSION1)		; localization and interrupt frequency
		ld hl,(BIOS_VDP_IOPORT) 	; vdp ports
		inc h
		inc l
		ld a,(BIOS_MACHINE_TYPE)	; machine type
		ld sp,#0xe100
		push de
		push bc
		push hl
		push af

		;
		; prepare for all-ram
		;
		ld a,d
		ld hl,#PAGE0_BASE
		call enaslt

		ld a, #3
		out (RAM_PAGE0), a
		ld a, #2
		out (RAM_PAGE1), a
		ld a, #1
		out (RAM_PAGE2), a

		; jump to start while keeping rom in page 1 (we are running from it)
		; will switch to all-ram in there
		jp 0x100


		; find slot currently set in page 1
		; returns slot in reg a using FxxxSSPP format (see enaslt)
find_rom:
		in a,(SLOT_SEL)
		rrca
		rrca
		and #3
		ld c,a
		ld b,#0
		ld hl,#0xfcc1       ; system variables containing slot and sub-slot data
                            ; set up by bios on boot
		add hl,bc
		ld a,(hl)
		and #0x80
		jr z,find_rom0
		or c
		ld c,a
		inc hl
		inc hl
		inc hl
		inc hl
		ld a,(hl)
		and #0x0C
find_rom0:
		or c
		ret

		; find slot currently set in page 3
		; returns slot in reg a using FxxxSSPP format (see enaslt)
find_ram:
		in a,(SLOT_SEL)
		rlca
		rlca
		and #3
		ld c,a
		ld b,#0
		ld hl,#0xfcc1       ; system variables containing slot and sub-slot data
                            ; set up by bios on boot
		add hl,bc
		ld a,(hl)
		and #0x80
		jr z,find_ram0
		or c
		ld c,a
		inc hl
		inc hl
		inc hl
		inc hl
		ld a,(hl)
		rlca
		rlca
		rlca
		rlca
		and #0x0C
find_ram0:
		or c
		ret

		; set slot and subslot at target address
		; (from msx bios listing)
		; hl - target address
		; a  - slot : FxxxSSPP
		;             F  : expanded slot flag (if F=0, SS is ignored)
		;             SS : expanded subslot
		;             PP : primary slot
enaslt:
		call selprm         ; calculate bit pattern and mask code
		jp m, eneslt        ; if expanded set secondary first
		in a,(SLOT_SEL)
		and c
		or b
		out (SLOT_SEL),a        ; set primary slot
		ret
eneslt:
		push hl
		call selexp         ; set secondary slot
		pop hl
		jr enaslt

		; calculate bit pattern and mask
selprm:
		di
		push af
		ld a,h
		rlca
		rlca
		and #3
		ld e,a              ; bank number
		ld a,#0xC0
selprm1:
		rlca
		rlca
		dec e
		jp p, selprm1
		ld e,a              ; mask pattern
		cpl
		ld c,a              ; inverted mask pattern
		pop af
		push af
		and #3              ; extract xxxxxxPP
		inc a
		ld b,a
		ld a,#0xAB
selprm2:
		add a,#0x55
		djnz selprm2
		ld d,a              ; primary slot bit pattern
		and e
		ld b,a
		pop af
		and a               ; if expanded slot set sign flag
		ret

		; set secondary slot
selexp:
		push af
		ld a,d
		and #0xC0          ; get slot number for bank 3
		ld c,a
		pop af
		push af
		ld d,a
		in a,(SLOT_SEL)
		ld b,a
		and #0x3F
		or c
		out (SLOT_SEL),a        ; set bank 3 to target slot
		ld a,d
		rrca
		rrca
		and #3
		ld d,a
		ld a,#0xAB          ; secondary slot to bit pattern
selexp1:
		add a,#0x55
		dec d
		jp p,selexp1
		and e
		ld d,a
		ld a,e
		cpl
		ld h,a
		ld a,(SUBSLOT_SEL)       ; read and update secondary slot register
		cpl
		ld l,a
		and h               ; strip off old bits
		or d                ; add new bits
		ld (SUBSLOT_SEL),a
		ld a,b
		out (SLOT_SEL),a        ; restore status
		pop af
		and #3
		ret

