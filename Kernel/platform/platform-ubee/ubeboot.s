;
;	First go at a Microbee 128 boot sector
;
;	The 'bee actually has a rather civilised boot environment with
;	several boot ROM helpers available
;

MONITOR		.equ	0xE003		; jump into monitor
CONOUT		.equ	0xE00C		; print character in A
VDUSETUP	.equ	0xE01E		; set 64x16 mode
SELDISK		.equ	0xE021		; select disk in drive A
XMXFER		.equ	0xE02D		; bank to bank block move
					; HL bytes from D,IX to E,IY
FLASH		.equ	0xE033		; Print string in HL ending 0x80
					; wait for return
OFFLOAD		.equ	0xE039		; Start at track D sector E
					; load BC bytes into HL
					; Z = OK

;
;	We get loaded at 0x0080
;
;	We are bank 0, block 0, ROM is high. Annoyingly when we turn the
;	ROM off bank 0, block 0 ends up the common higher block
;

		.org 	0x6F00
start:		ld hl, 0x0080		; put ourselves somewhere safe
		ld de, 0x6F00		; 6F allows for video when we mirror
		ld bc, 128		; high
		ldir
		jp sysboot
sysboot:	ld sp, sysboot
		ld de, 0x0101		; start at track 1 for data for now
		ld iy, 0x0000		; final destination bank 0,1, 0x0000
		call offload16		; copy
		ld de, 0x020D		; track 2 sector 13 (1 track, 12
			  		; sectors for 16K)
		ld iy, 0x4000
		call offload16		; load next 16K, bank 0,1, 0x4000
					; 32K now loaded, low image done
		ld hl, 0x0000
		ld bc, 0x5000		; load another 20K
		ld de, 0x0305		; track 3 sector 5 (32K in from start)
		call OFFLOAD
		jr nz, badload
		ld a, 4		; bank 0 both, ROMs off
		di			; paranoia
		out (0x50), a		; bank 0 is now low and high
		jp strap+0x8000	; jump to our high copy

offload16:	ld hl, 0x4000
		ld bc, 0x4000
		push iy
		call OFFLOAD
		jr nz, badload
		pop iy
xm16:		ld hl, 0x4000
		ld ix, 0x4000
		ld de, 0x0001
		jp XMXFER

strap:	        inc a
		out (0x50), a		; bank 1 now low
		jp 0x100		; enter OS image in bank 0

badload:	ld hl, fail
		call FLASH
		jp MONITOR