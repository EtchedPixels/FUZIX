
I_ADDR		.equ	0x18
I_SIZE		.equ	0x08
I_TYPE		.equ	0x00

loadbuf		.equ	0x0100

DIRTYPE		.equ	0x40
FILETYPE	.equ	0x80

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
	push hl
	pop ix
	ld a,(hl)
	cp #'*'
	jr z, do_system
	ld a,1(ix)
	cp #':'
	jr nz, boot_name
	ld a,2(ix)
	or a
	jr nz, boot_name
	ld a,(hl)
	cp #'A'
	jr c, boot_name
	cp #'P'
	jr nc, boot_name
	sub #'A'
	call drive_set
	jr con_next
do_system:
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
	jp nz, bread_disk
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

