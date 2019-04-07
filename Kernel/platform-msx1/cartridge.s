;
;	We will nail this somewhere handy on a page boundary
;
		.area _HEADER
		.db 'A'
		.db 'B'
		.dw wtfami
		.dw 0,0,0,0,0,0

		.globl _fuzix_main
		.globl init_early
		.globl init_hardware
		.globl s__DATA
		.globl l__DATA
		.globl s__INITIALIZER
		.globl s__INITIALIZED
		.globl l__INITIALIZER
		.globl s__COMMONMEM
		.globl l__COMMONMEM

		.globl kstack_top

;
;	We are running in an unknown subslot of an unknown slot and
;	have 4000-7FFF mapped to us, BIOS (0:0) in the low 16K and
;	RAM ought to be in the top 16K because the BIOS needs it. We
;	can't btw even assume that all the RAM is in the same bank - what a
;	mess!
;
;	The BIOS has produced
;	EXPTBL - bit 7 set for each expanded slot
;	SLTTBL - current value of expansion slot addr
;	SLTATR - byte per page for slot contents, notably devices
;
CHPUT		.equ 0x00A2
INITXT		.equ 0x006F
EXPTBL		.equ 0xFCC1

pstring:
		ld a,(hl)
		or a
		ret z
		inc hl
		call CHPUT
		jr pstring

dophex:		ld c,a
		and #0xf0
		rrca
		rrca
		rrca
		rrca
		call pdigit
		ld a,c
		and #0x0f
pdigit:		cp #10
		jr c,isl
		add #7
isl:		add #'0'
		out (0x2F),a
		ret

phex:
		push af
		push bc
		call dophex
		ld a,#' '
		out (0x2f),a
		pop bc
		pop af
		ret

;		Ripple bits 2-3 into the whole byte
;
;		Returns A holding it rippled into the low 3 banks with the
;			top one unchange
;			E throughout
;
ripple:
		push af
		and #0x0C
		ld e,a
		rrca
		rrca
		or e		; low 4 bits done
		ld e,a		; and saved
		rlca
		rlca
		rlca
		rlca
		or e		; all bits done
		ld e,a
		and #0x3F
		ld d,a
		pop af
		and #0xC0
		or d
		ret

expan:		.asciz 'expanded'
		
wtfami:		ld sp,#0xF380		; below system variables
		;
		;	Initialize the debug port
		;
		ld a,#0x23
		out (0x2E),a
		ld a,#'@'
		out (0x2F),a
		call INITXT		; set 40 column text mode
		ld hl,#hello
		call pstring
		ld a,#'#'
		out (0x2F),a

		; We'd like the use the ROM services but unfortunately they
		; don't actually work for the top 16K in MSX

		ld sp,#0xfcc0		; We can trash the sysvars below this

		ld ix,#0xffff		; subslot control byte

		di
		ld a,(ix)
		cpl			; remember the old subslot mapping
					; for the RAM top 16K
		ld e,a
		in a,(0xA8)
		ld d,a			; Remember the old slot mapping
		and #0x0C		; This is the mapping for the cartridge
		rla
		rla
		rla			; move it to the top 2 bits
		rla
		ld b,a
		in a,(0xA8)
		and #0x3F
		or b
		ld b,a
		out (0xA8),a		; cartridge mapped in top 16K not stack
		ld a,(ix)
		cpl
		and #0x0c		; cartridge subslot if any
		rla
		rla
		rla
		rla
		ld c,a			; save the bits
		ld a,(0xffff)
		cpl
		and #0x3f		; mask off top 16K map
		or c
		ld c,a

		; At this point D/E get us the RAM mapping
		; C/B the ROM mapping
		; We could both be in the same slot so we have to do all the
		; painful stupid stuff 8(

	        ld a,d
		out (0xA8),a
		ld (ix),e

		ld a,d
		call phex
		ld a,e
		call phex
		ld a,b
		call phex
		ld a,c
		call phex
		exx

		ld hl,#0xC000
		ld bc,#0x3C80

copy_common_loop:
		exx			; Paging values
		ld a,b			; Map cartridge
		out (0xA8),a
		ld (ix),c
		exx			; Work values
		ld e,(hl)		; Fetch byte
		exx			; Paging values
		ld a,d			; Map RAM
		out (0xA8),a
		ld (ix),e
		exx			; Work values
		ld (hl),e		; Save to RAM
		inc hl			; Move on
		dec bc
		ld a,b
		or c
		jr nz, copy_common_loop

		; RAM is what is left mapped

		;
		;	Save a few things before we give the ROM the boot
		;	We keep them in the alt registers
		;
		ld a,(0x002D)	; machine type
		ld d,a		; d' is the machine type
		ld bc,(0x0006)	; bc' is the VDP port
		ld hl,(0x002B)	; hl' is the info bits
		exx
		
		;
		;	Now find out what slot we are
		;	The ROM is not very helpful here because we
		;	want to take it out too
		;
		in a,(0xA8)
		and #0x0C
		rrca
		rrca			; active slot

		ld b,#0
		ld c,a
		ld hl,#EXPTBL
		add hl,bc

		bit 7,(hl)
		jr z, not_expanded

		inc hl
		inc hl
		inc hl
		inc hl

		; Work out the required selects

		ld a,(hl)
		call ripple
		ld b,a

		in a,(0xA8)
		call ripple

		; Returns A = regs for final selection, E = regs
		; to fix sub selection

		ld c,a		; save final setting

		ld a,e		; All slots to our cartridge bank
		out (0xA8),a
		ld a,b
		ld (0xFFFF),a	; Sub-slot 0-2 to our cartridge
		ld a,c
		out (0xA8),a	; Slots 0-2 to cartridge, 3 restored


		jr mapped

not_expanded:
		in a,(0xA8)
		call ripple
		out (0xA8),a
mapped:
		di
		; We now have RAM high and 48K of our ROM space low
		; That's enough to get us up and running. We can work out
		; where the rest of the RAM is later on

		; Begin by setting up our RAM as a normal ROM based SDCC
		; process (for a change!)
;		ld hl,#s__INITIALIZER
;		ld de,#s__INITIALIZED
;		ld bc,#l__INITIALIZER
;		ldir
;		; Now unpack the common space
;		ld de,#s__COMMONMEM
;		ld bc,#l__COMMONMEM
;		ldir
;		; Wipe the BSS area
;		ld hl,#s__DATA
;		ld d,h
;		ld e,l
;		ld (hl),#0
;		ld bc,#l__DATA
;		inc de
;		dec bc
;		ldir
		; Switch to the right stack
		ld sp,#kstack_top
		call init_early
		call init_hardware
		call _fuzix_main
		di
stop:		halt
		jr stop

hello:		.ascii 'Loading FUZIX...'
		.db 13,10,0
