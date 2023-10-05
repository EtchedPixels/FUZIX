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

		;
		; enable R800 CPU if available
		;
		ld a,(BIOS_CHGCPU)
		cp  #0xC3
		ld  a,#0x81		    ; run only in rom mode, as we do not use BIOS better save 4 ram pages
		call z,#BIOS_CHGCPU

		;
		; set ram in slot_page 2
		;
		call find_ram
		ld hl,#PAGE2_BASE
		call enaslt

		;
		; find ram size
		;
		call size_memory
		push hl

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

		pop hl				; re-stash ram size

		ld a, #4
		out (RAM_PAGE3),a

		ld sp,#0xe100
		push hl

		ld bc,(BIOS_VERSION1)		; localization and interrupt frequency
		ld hl,(BIOS_VDP_IOPORT) 	; vdp ports
		inc h
		inc l
		ld a,(BIOS_MACHINE_TYPE)	; machine type
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


		;
		; Size currently selected memory mapper
		;
size_memory:
		ld bc, #0x03FE		; make sure ram page 3 is selected
		out (c), b
		ld hl, #0x8000
		ld (hl), #0xAA		; we know there is a low page!
		ld bc, #0x04FE		; continue with page 4
ramscan_2:
		ld a, #0xAA
ramscan:
		out (c), b
		cp (hl)			; is it 0xAA
		jr z, ramwrapped	; we've wrapped (hopefully)
		inc b
		jr nz, ramscan
		jr ramerror		; not an error we *could* have 256 pages!
ramwrapped:
		ld a, #3
		out (c), a
		ld (hl), #0x55
		out (c), b
		ld a, (hl)
		cp #0x55
		jr z, ramerror		; Cool we wrapped both change to 0x55
		; Fluke RAM was 0xAA already
		ld a, #3
		out (c), a
		ld a, #0xAA
		ld (hl), a			; put the marker back as 0xAA
		inc b
		jr nz, ramscan_2		; Continue our memory walk
ramerror:   				; Ok so there are 256-b-3 pages of 16K)
		ld a,#3
		out (c), a			; always put page 0 back
		;
		;	Address map back to normal so can update kernel data
		;
		dec b			; take into account we started at page 3
		dec b
		dec b
		ld l, b
		ld h, #0
		ld a, l
		or a			; zero count -> 256 pages
		jr nz, pageslt256
		inc h
pageslt256:
		; hl contains num of pages
		ret



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

