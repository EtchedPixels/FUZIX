;
;	Bootstrap for MSX-DOS2
;
;	The kernel image follows us
;
;	Our memory map courtesy of the MSX2 BIOS is 3,2,1,0 and we are
;	all RAM. We need to put the kernel into 3,2,1,4 (can we use 0??)
;

		.area _BOOT

		.include "msx2.def"

		; we are linked at 0xF100 but start at 0x0100
		; hence the weird -0xF000 offsetting

bootstrap:
		ld a,#0x23
		out (0x2E),a
		ld a,#'1'
		out (0x2F),a
		ld a,(BIOS_CHGCPU)
		cp #0xC3
		ld a,#0x81
		call z, BIOS_CHGCPU
		ld ix,#biosinfo-0xF000	; we will write it low read it high
		
		; System info lives in the base ROM in slot 0, not in our
		; MSXDOS 2 provided mappings. Use RDSLT

		ld hl,#BIOS_VERSION1
		ld a, (BIOS_ROMSLOT)
		call RDSLT
		ld (ix),a
		inc hl
		ld a, (BIOS_ROMSLOT)
		call RDSLT
		ld 1(ix),a

		ld hl,#BIOS_VDP_IOPORT
		ld a, (BIOS_ROMSLOT)
		call RDSLT
		inc a
		ld 2(ix),a
		inc hl
		ld a, (BIOS_ROMSLOT)
		call RDSLT
		inc a
		ld 3(ix),a

		ld hl, #BIOS_MACHINE_TYPE
		ld a, (BIOS_ROMSLOT)
		call RDSLT
		ld 4(ix),a

		; Get RAM bank into D and ROM bank into E
		; Get RAM size into HL (pages count 0-256)
		xor a
		ld de,#0x0401
		call BIOS_EXTBIO
		ld d,(hl)	; Slot holding our mapper RAM
		inc hl
		ld l,(hl)	; Number of banks
		ld a,(BIOS_ROMSLOT)
		ld e,a

		ld a,#'2'
		out (0x2F),a

		ld h,#0
		cp l
		jr nz, nonzero
		inc h
nonzero:	
		;
		;	Take over time. TODO see about 4 v 0 paging
		;

		di
		;	TODO relocate ourself high and safe somewhere
		exx
		ld hl,#0x0100
		ld de,#0xf100
		ld bc,#256
		ldir
		exx
		ld a,#'3'
		out (0x2F),a
		jp go
;
;	and into our copy at the expect address
;
go:
		ld a,#'4'
		out (0x2F),a

		ld sp,#go

		; Now relocate the kernel to 0x0100 as it expects

		exx
		ld hl,#end-0xF000
	        ld de,#0x100
		ld bc,#0xF000
		ldir

		ld a,#'5'
		out (0x2F),a

		ld a,#4
		out (0xFD),a		; 0x4000 is now the final high page
		ld hl,#0xC000
		ld de,#0x4000
		ld bc,#0x3FFF		; top byte is magic subslot stuff
		ldir
		exx

		ld a,#'6'
		out (0x2F),a

		ld a,#2
		out (0xFD),a		; restore 0x4000 map

		; Now as we copied all of the 16K we copied ourself so can
		; now flip bank

		ld a,#4
		out (0xFF),a

		ld a,#'7'
		out (0x2F),a
		; Set up parameters

		push hl			; Number of RAM banks
		push de			; ROM and RAM slot

		; System info lives in the base ROM in slot 0, not in our
		; MSXDOS 2 provided mappings. Use RDSLT

		ld hl,(biosinfo)
		push hl			; BIOS information bits
		ld hl,(biosinfo + 2)
		push hl			; VDP info
		ld a, (biosinfo + 4)
		push af			; Machine type

		ld a,#'8'
		out (0x2F),a

		; launch

		jp 0x0100

biosinfo:
		.word 0			; BIOS information
		.word 0			; VDP ports
		.byte 0			; Machine Type

end:
		; Kernel is packed from here
