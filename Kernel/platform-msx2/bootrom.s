		.area _BOOT

		.db 'A'
		.db 'B'
		.dw bootstrap
		.dw 0,0,0,0,0,0

		.globl enaslt
		.globl _slotrom
		.globl _slotram
		.globl find_ram
		.globl kstack_top


		; At this point the BIOS has detected the cartridge AB signature and
		; jumped here; we have the rom bios in bank 0, ram in bank 3, and rom
		; in bank 1 (this code).
bootstrap:
		di
		ld sp,#0xf340       ; temporary stack

		; FIXME: init vdp using bios so that font is in place after vdpinit
		ld a,#0x50
		ld (0xf3ae),a
		ld ix,#0x00d5
		call #0x015f

		; read slot registers from bios before overwritting them
		call find_ram
		ld hl,#0x8000
		call enaslt		; set bank 2 to ram

		ld a, #4
		out (0xFE),a

		call find_ram
		ld d,a
		call find_rom
		ld e,a
		exx

		ld hl,#0xc000
		ld sp,#0xa000	; keep stack in ram
		call enaslt		; set bank 3 to rom

		; copy kernel page 0 from bank 3 to ram in bank 2
		; it contains the common area if the rom > 48Kb
		ld hl, #0xc000
		ld de, #0x8000
		ld bc, #0x4000
		ldir
		exx

		push de
		ld hl,#0xc000	; set bank 3 back to ram
		ld a,d
		call enaslt
		pop de			; store slot data in ram now
		ld a, #4
		out (0xFF),a
		ld a,d
		ld (_slotram),a
		ld a,e
		ld (_slotrom),a

		ld sp, #kstack_top	; move stack to final location

		; set cartridge rom in bank 0
		ld hl,#0
		call enaslt

		; copy kernel page 3 to ram
		ld a, #3
		out (0xFE),a
		ld hl, #0x0
		ld de, #0x8000
		ld bc, #0x4000
		ldir

		; set ram in bank 0 and cartridge rom in bank 2
		ld a,(_slotram)
		ld hl,#0
		call enaslt
		ld a,(_slotrom)
		ld hl,#0x8000
		call enaslt

		; copy kernel pages 2 and 1 to ram
		ld a, #2
		out (0xFC),a
		ld hl, #0x4000
		ld de, #0x0
		ld bc, #0x4000
		ldir
		ld a, #1
		out (0xFC),a
		ld hl, #0x8000
		ld de, #0x0
		ld bc, #0x4000
		ldir

		; prepare mapped ram pages for all-ram
		ld a, #3
		out (0xFC), a
		ld a, #2
		out (0xFD), a
		ld a, #1
		out (0xFE), a
		ld a, #4
		out (0xFF),a

		; set bank 2 to ram
		ld a,(_slotram)
		ld hl,#0x8000
		call enaslt

		; jump to start while keeping rom in page 1 (we are running from it)
		; will switch to all-ram in there
		jp 0x100


		; find slot currently set in page 1
		; returns slot in reg a using FxxxSSPP format (see enaslt)
find_rom:
		in a,(0xa8)
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
		in a,(0xa8)
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
		in a,(0xa8)
		and c
		or b
		out (0xa8),a        ; set primary slot
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
		in a,(0xa8)
		ld b,a
		and #0x3F
		or c
		out (0xa8),a        ; set bank 3 to target slot
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
		ld a,(0xffff)       ; read and update secondary slot register
		cpl
		ld l,a
		and h               ; strip off old bits
		or d                ; add new bits
		ld (0xffff),a
		ld a,b
		out (0xa8),a        ; restore status
		pop af
		and #3
		ret

