;
;	We have 12K loaded D000-FFFF. We actually keep ourselves highish up
;	to leave lots of room to load code
;
;	Currently this and the glue driver code is well under 2 disk blocks.
;	Adding partitions should still fit 2 blocks nicely.
;

I_ADDR		.equ	0x18
I_SIZE		.equ	0x08
I_TYPE		.equ	0x00

loadbuf		.equ	0x0100

DIRTYPE		.equ	0x40
FILETYPE	.equ	0x80

	.area _CODE

;
;	The main loop. We wait for about 2 seconds looking for input. If we
;	see input then we go to command line mode, if not we load the
;	default image name. If the default fails we then drop into command
;	line mode.
;
;	Right now we treat anything as a filename, except * which indicates
;	platform specific helpers. We may add other commands later (eg
;	single letters to switch drive).
;
boot_begin:
	; Run anything system specific
	call preboot
	ld hl,#filo
	call con_write
	; Load the partition table and find our partition
	call partitions
boot_up:
	ld hl,#prompt
	call con_write
	ld b,#200		; 2 seconds
boot_wait:
	push bc
	call con_check
	jr nz, con_prompt
	call delay10ms
	pop bc
	djnz boot_wait
boot_default:
	ld hl,#newline
	call con_write
	ld hl,#defname
boot_name:
	ld de,#loadname
	ld bc,#31
	ldir
	call load_kernel
	ld hl,#failed
	call con_write
con_next:
	ld hl,#prompt
	call con_write
con_prompt:
	call con_readbuf
	jr z, con_next
	ld a,(hl)
	cp #'*'
	jr nz, boot_name
	call system
	jr con_next

prompt:
	.asciz 'FILO: '
failed:
	.ascii 'Load failed.'
newline:
	.asciz '\n'
defname:
	.asciz 'fuzix'
filo:
	.ascii 'Fuzix Intermediate Loader v0.01.\n'
	.asciz '(C)2019 Alan Cox\n\n'

	.area _DATA

loadname:
	.ds 31
ibuf:
	.ds 64
dbuf:
	.ds 512
partbase_l:
	.dw 0
partbase_h:
	.dw 0

	.area _CODE

partitions:
	; No partitions found (hack for now)
	ld hl,#0
	ld (partbase_l),hl
	ld (partbase_h),hl
	ret
;
;	We have set the base according to the partition table, work relative
;	to it
;
;	The key to this being small is that a directory is a special kind of
;	file. We therefore only need one routine which loads a file at the
;	load address to do all the work. We load inode 1 (/) and find the
;	file, then we use the same function to load the actual file.
;
;	We could easily add subdirectories if we wanted.
;
;	The data loading is kept compact by having a routine to load an
;	array of block numbers. That same logic works for both the direct
;	blocks and the first indirect (which is all we care about).
;
load_kernel:
	;
	;	Load the super block
	;
	ld de,#1		; 0 is the boot block space, 1 the superblock
	ld hl,#dbuf
	call bread
	ret nz
	;
	;	Check the superblock is valid
	;
	ld hl,(dbuf)
	ld de,#12742		; file system
	or a
	sbc hl,de
	ret nz			; not valid
	;
	;	It's a filesystem so we can load inode 1 
	;
	ld hl,#1
	ld a,#DIRTYPE		; want a directory

	call loadi		; load / into memory
				; returns DE = size
	ret nz			; / failed to load or too large
	ld ix,#loadbuf		; start pointer into IX
	;
	;	Turn the size into a record count of 32 byte records
	;
	ld b,#5
shift5:
	srl d
	srl e
	djnz shift5
	; DE is now record count

	;
	;	Scan the loaded file (directory) for the wanted name entry
	;
find_by_name:
	push de
	ld hl,#loadname
	push ix
	call cmpname
	pop ix
	jr nz, next_name
	ld l,30(ix)
	ld h,31(ix)
	ld a,h			; empty slot even if matched
	or l
	jr nz, found_name
next_name:
	;
	;	Move on a record
	;
	ld de,#32
	add ix,de
	pop de
	dec de
	ld a,d
	or e
	jr nz, find_by_name
	;
	;	Not found
	;
	inc a			; force NZ
	ret			; not found
	;
	;	Name match found
	;
found_name:
	pop af			; Discard top of stack
	ld a,#FILETYPE		; must be a file
	call loadi		; replace the directory load with the target
	ret nz
	;
	;	And launch
	;
	jp loadbuf

;
;	Load a file or directory if not too big
;
loadi:
	; A is the required type, HL the inode
	call iget		; get the relevant inode into the ibuf
	ret nz
	ld de, (ibuf+I_SIZE+2)
	ld a,d
	or e
	ret nz			; over 64K
	ld de, (ibuf+I_SIZE)
	ld a,#0xE0
	cp d
	jr nc, retnz		; too big too load

	ld a,e
	or a
	jr z, directs		; exact blocks
	inc d			; load the partial
	;
	;	Now it's load time
	;
directs:
	push de
	ld a,d			; number of blocks
	cp #18
	jr z, shortfile
	ld a,#18		; direct blocks
shortfile:
	ld b,a
	ld hl,#loadbuf
	ld ix,#ibuf + I_ADDR
	call breadset		; read b blocks from the list in ix
	pop de
	ret nz
	; See what indirects are needed if any
	ld a,d
	cp #19
	jr c, load_done
	push de
	push hl
	ld e,(ix)
	inc ix
	ld d,(ix)
	inc ix
	ld hl,#dbuf
	call bread		; load the indirect block
	pop de
	pop hl
	ld a,d
	sub #18			; direct blocks
	ld b,a			; remainder
	ld ix,#dbuf
	call breadset
	ret nz
load_done:
	; File now loaded
	xor a
	ret
retnz:
	or a
	ret

;
;	Load inode HL into ibuf
;
;	Entry: HL = inode number, A = type bits
;
;	Return: NZ = error, Z = ok, data in ibuf
;
;	Destroys: AF/BC/DE/HL
;
iget:
	; Turn an inode into a block
	push af
	push hl
	ex de,hl
	srl d		; Divide inode by 8 (64 bytes per inode)
	srl e
	srl d
	srl e
	srl d
	srl e
	ld hl,#dbuf
	call bread
	pop hl
	jr nz, badiget
	ld a,l
	and #0x07	; inode offset in block
	add a,a		; x2
	add a,a		; x4
	add a,a		; x8
	add a,a		; x16
	add a,a		; x32
	ld h,#0
	ld l,a
	add hl,hl	; x64
	ld de,#dbuf
	add hl,de	; valid pointer
	ex de,hl
	ld hl,#I_TYPE+1
	add hl,de	; hl is now the type bits
	ld a,(hl)
	and #0xF0	; type bits
	ld l,a
	pop af
	cp l
	ret nz		; wrong type of file
	ex de,hl	; get pointer back into hl
	ld de,#ibuf
	ld bc,#64
	ldir
	ret		; Z
badiget:
	pop af
	ret

;
;	Read a set of blocks listed in the array at ix. Any zero entry means
;	a sparse block (zero the memory).
;
;	Entry: IX = table, B = count, HL = load address
;
;	Return: Z = success, HL = next load address
;
;	Destroys: AF BC DE IX
;
;
breadset:
reader_loop:
	ld e,(ix)
	inc ix
	ld d,(ix)
	inc ix
	push bc
	call bread		; read block DE to HL and move HL on
	pop bc
	ret nz
	djnz reader_loop
	ret

;
;	bread
;
;	Load a disk block (512 bytes) or zero block
;
;	HL = buffer to read into
;	DE = block relative to partbase
;
;	return
;	NZ = fail, Z = ok
;	HL = next read address
;	BC, DE, AF destroyed
;	IX, IY preserved
;
bread:
	ld a,d
	or e
	jr nz, bread_disk
	ld d,h
	ld e,l
	ld bc,#511
	ld (hl),#0
	inc de
	ldir
	inc hl
	ret

;
;	Compare the name at IX with the one in HL
;
;	Returns Z = matched
;
;	Destroys BC,HL,IX
;
;
cmpname:			; check name in IX matches HL for 30 chars
	ld b,#30
cmploop:
	ld a,(ix)
	cp (hl)
	ret nz
	or a
	ret z			; First matched \0 matches all
	inc hl
	inc ix
	djnz cmploop
	ret			; Z 


;
; ****************************************************************************
;
;	System specific code begins here
;
; ****************************************************************************

DATA		.equ 	0x10
ERROR		.equ	0x11
FEATURES	.equ	0x11
COUNT		.equ	0x12
LBA_0		.equ	0x13
LBA_1		.equ	0x14
LBA_2		.equ	0x15
LBA_3		.equ	0x16
STATUS		.equ	0x17
CMD		.equ	0x17

ERR		.equ	0
DRQ		.equ	3
READY		.equ	6
BUSY		.equ	7

ATA_READ	.equ	0x10

drive		.equ	0xE0		; Drive 0 for now


SC114		.equ	8
SC108		.equ	0		; until assigned

;
;	bread_disk
;
;	Load a disk block (512 bytes)
;
;	HL = buffer to read into
;	DE = block relative to partbase
;
;	return
;	NZ = fail, Z = ok
;	HL = next read address
;	BC, DE, AF destroyed
;	IX, IY preserved
;
;	For the SCM systems we have to hit the disk directly as the SCM
;	doesn't expose disk functionality at this point. For CP/M or ROMWBW
;	we could go via the monitor.
;
bread_disk:
	push hl
	ld hl,(partbase_l)
	add hl,de
	ld de,(partbase_h)
	jr nc, nocarry_lba
	inc de
nocarry_lba:
	call ide_ready
	; Loading block DEHL
	ld a,d
	and #0x0F
	ld d,a
	ld a,(drive)
	or d
	out (LBA_3),a
	ld a,e
	out (LBA_2),a
	ld a,h
	out (LBA_1),a
	ld a,l
	out (LBA_0),a
	call ide_ready
	ret nz
	pop hl
	ld a,#1
	out (COUNT),a
	ld a,#ATA_READ
	out (CMD),a
	nop
	call ide_wait_drq
	ret nz
	ld bc,#DATA
	inir
	inir
	xor a
	ret
;
;	Timeout might be a good idea
;
ide_ready:
	in a,(STATUS)
	bit ERR,a
	ret nz
	bit READY,a
	jr z, ide_ready
	xor a
	ret
ide_wait_drq:
	in a,(STATUS)
	bit ERR,a
	ret nz
	bit DRQ,a
	jr z,ide_wait_drq
	xor a
	ret

;
;	Console I/O
;
;	Read the name buffer into a buffer
;
;	Destroys AF, BC, DE
;
;	Returns length of buffer in A and sets Z/NZ accordingly
;	Buffer pointer is returned in HL
;
;	We implement the console via the SCM monitor helpers
;
con_readbuf:
	ld c,#0x05
	call mcall
	or a
	ex de,hl
	ret
;
;	Check for input
;
;	Returns NZ if data
;	Destroys AF,BC,DE,HL
;
con_check:
	ld c,#0x03
	jp mcall
;
;
;	Write the string at HL
;
;	Destroys AF,HL
;
con_write:
	push bc
	push de
con_writel:
	ld a,(hl)
	cp #0x0A
	jr z, con_writenl
	or a
	jr z, con_writedone
	ld c,#0x02
con_wop:
	push hl
	call mcall
	pop hl
	inc hl
	jr con_writel
con_writenl:
	ld c,#0x07
	jr con_wop
con_writedone:
	pop de
	pop bc
	ret
;
;	Pre boot. Run before anything else kicks off
;
preboot:
	ld c,#0x08
	call mcall
	ld a,h
	cp #SC108
	ret z
	cp #SC114
	ret z
	ld hl,#unsupported
	call con_write
	ld c,#0x00
	jp mcall

unsupported:
	.asciz 'Unsupported platform.\n'

;
;	System.
;	Input HL = command string
;
;	Return: none
;
;	Destroys: AF/BC/DE/HL
;
;	Do anything with a command beginning *. In our case we feed it to
;	the monitor
;
system:
	inc hl		; Skip the *
	ex de,hl
	ld c,#0x22	; Monitor call
	;
	;	Call monitor
	;
mcall:
	ex af,af'
	ld a,#1
	out (0x38),a
	ex af,af'
	rst 0x30
	ex af,af'
	xor a
	out (0x38),a
	ex af,af'
	ret
;
;	Delay approximately 10ms
;
;	Destroys AF/BC/DE/HL
;
;	SCM has a helper for this.
;
delay10ms:
	ld c,#0x0a
	ld de,#0x10
	jr mcall
