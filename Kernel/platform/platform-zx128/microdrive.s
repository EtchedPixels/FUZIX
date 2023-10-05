;
;	Sinclair Microdrive Controller
;
;	Doing in software what floppy disk controllers do in hardware!
;
;	A microdrive has up to 254 sectors on it, in reality nearer 200. The
;	blocks are sequential on the tape but in *reverse* order. This is
;	because the formatter writes 254,253,... down to 1, with some
;	overwriting those blocks written first, then verifies the blocks
;	to see how many fitted.
;
;	Some of the blocks will be bad. The formatter in the ROM marks the
;	failed block bad and also the one following.
;
;	To use it like a floppy disc we use sector numbers instead of names
;	as the speccy does. It's not quite the same however. We handle bad
;	blocks by keeping a logical/physical mapping. Each physical block
;	has a physical and logical identifier. In spectrum firmware land
;	there is an "erase" command. When running as a pseudo-floppy we
;	don't have that. Instead we keep a blockmap table in physical 1 and
;	128.
;
;
;	FIXME: currently if we find the block but get a bad csum we can
;	loop effectively forever.
;
		.module microdrive

		.globl _mdv_motor_on
		.globl _mdv_motor_off
		.globl _mdv_bread
		.globl _mdv_bwrite

		; imports
		.globl _mdv_sector
		.globl _mdv_buf
		.globl _mdv_hdr_buf
		.globl _mdv_w_hdr_buf
		.globl _mdv_len
		.globl _mdv_page
		.globl _mdv_csum

		.globl map_process_save
		.globl map_kernel_restore
		.globl current_map
		.globl switch_bank

		.area _COMMONMEM

SECTORID	.equ	0x01
CSUM		.equ	0x0E
MAP_PREAMBLE	.equ	0x0C

nap_1ms:	push de
		ld de, #87
		jr napl
nap:		push de
napl:		dec de
		ld a, d
		or e
		jr nz,napl
		pop de
		ret
;
;	Must preserve E
;
mdv_csum_hdr:
		ld hl, #_mdv_hdr_buf	; header buffer
mdv_csum_hdr_2:
		ld bc, #0x0E01		; 14 bytes
		call mdv_csum_data
		cp (hl)
		ret
mdv_csum_w_hdr:
		ld hl, #_mdv_w_hdr_buf + MAP_PREAMBLE
		jr mdv_csum_hdr_2

mdv_csum_data:
		xor a
csum_data2:
		add (hl)
		adc #1
		inc hl
		jr z, csum_d2
		dec a
csum_d2:
		djnz csum_data2
		dec c
		jr nz, csum_data2
		ret

;
;	Load a microdrive sector into the buffer selected by _mdv_buf
;	for _mdv_len (in partial/full counts format). The lead partial goes
;	into _mdv_hdr_buf always
;
;	Note this loads a buffer, any buffer, whatever arrives. It's your
;	problem to decide if it's the buffer you wanted.
;

mdv_seek:	ld b, #8	; we need to see gap for 8 cycles
		dec hl
		ld a, h
		or l
		ret z		; expired
mdv_seek2:
		in a, (0xEF)
		and #4
		jr z, mdv_seek

		; We found a gap bit, celebrate
		djnz mdv_seek2
		; Happy gappy
		
		ld a, #3
		out (0xfe), a
		; Now do the same the other way up
mdv_seeku:	ld b, #6	; we need to see ungap for 6 cycles
		dec hl
		ld a, h
		or l
		ret z		; expired
mdv_seeku2:	in a, (0xEF)
		and #4
		jr nz, mdv_seeku
		djnz mdv_seeku2

		; Gappity gap
		ld a, #5
		out (0xfe), a
		ld a, #0xEE
		out (0xEF), a

		ld b, #0x3C	; Must see a sync within 60 cycles

mdv_sync:	in a, (0xEF)
		and #0x02
		jr z, mdv_sync_go
		djnz mdv_sync
		xor a
		out (0xfe), a
		jr mdv_seek	; back to square one

mdv_sync_go:
		; We are in sync
		ld hl, #_mdv_hdr_buf
		ld de, (_mdv_buf)
		ld bc, (_mdv_len)	; in partial/full pair format
		ld a, #7
		out (0xfe), a
		ld a, c
		ld c, #0xE7

;
;	Q: do we have enough clocks to pull the partial, flip buffer ptr and
;	continue. Seems we probably do
;
		inir			; copy the partial
		sub #1
		jr c, mdv_hdr_only
		ex de, hl		; just about fast enough
mdv_blockread:
		inir
		sub #1
		jr nc, mdv_blockread
		in a, (c)		; grab the checksum
		ld e, a
		; the eagle has landed
mdv_hdr_only:
		xor a
		out (0xfe), a
		ld a, #0xee
		out (0xEF), a
		or a
		ret

;
;	Load the next header, well probably header - you might get the
;	start of a data chunk, in which case try again
;

mdv_get_hdr:	ld hl, #0x0F00		; 15 + no loops
		ld (_mdv_len), hl
		ld hl, #0		; allow a long time to find a header
		call mdv_seek		; they seek him here, they seek him there
		jr z, hdr_fail		; if he ret's z he's off elsewhere


		; fixme: check its a header block !
		call mdv_csum_hdr
		ret z			; Z = good header
		ld a, #2
		ret			; 2 = bad csum
hdr_fail:	
		ld a, #3
		or a
		ret			; 3 = no response, give up for good

;
;	Find a microdrive block by matching header
;	
;	This uses the physical sector number which is *not* the same as
;	our logical one. We'll deal with that later.
;
;	FIXME: we should spot repetitions of the first block# seen so we can
;	give up after 3 loops of the tape exactly.
;
mdv_find_hdr:	ld bc, #2048		; worst case is 4 times round the tape

mdv_find_hdr_l:	push bc
		call mdv_get_hdr	; Fetch any header
		pop bc
		jr nz, mdv_find_hdr_bad	; If it didn't work check the error
		ld hl, #_mdv_hdr_buf
		ld a, (hl)		; Was it data ?
		cp #1
		jr nz, mdv_find_hdr_next; NZ, 1 = not a header
		ld hl, #_mdv_hdr_buf + SECTORID
		ld a, (_mdv_sector)
		cp (hl)
		ret z			; found it
		; Sector header, valid, but not the one we wanted
mdv_find_hdr_next:
		; Count down through our tape scan
		dec bc
		ld a, b
		or c
		jr nz, mdv_find_hdr_l	; keep looking
		inc a			; NZ
		ret
		;
		; Error 3 from mdv_get_hdr means there was nothing found
		; on the tape, so no point trying further. Otherwise it was
		; just a bad header, and we can carry on
		;
mdv_find_hdr_bad:
		cp #3			; 3 = give up now
		jr nz, mdv_find_hdr_next
		or a			; will be > 0
		ret			; NZ
;
;	Load the data for a microdrive block. It's assumed you just found
;	the right header then called this
;
mdv_get_blk:	ld hl, #0x0F02		; 15 + 2 loops (data) + csum in e
		ld (_mdv_len), hl
		ld hl, #0x01F4		; that's the count the IF1 allows
		call mdv_seek
		jr z, hdr_fail		; bad fail
		ld hl, #_mdv_hdr_buf
		ld a, (hl)
		out (0xfe), a
		and #0x01
		jr nz, failblk		; we got another header???
		; Sum the header block
		call mdv_csum_hdr
		jr nz, failblk
		ld hl, (_mdv_buf)	; now the data
		ld bc, #2		; 2 x 256 byte runs
		call mdv_csum_data	; checksum data into a
		cp e			; expected csum
		ret z			; good block
		ld a, #2
		out (0xfe), a
		; try again
failblk:
		ret

;
;	IRQ had must be off here...
;
mdv_put_blk:	ld a, #0xE6
		out (0xEF), a			; Write mode
		ld de, #0x0168			; Wait for record
		call nap			; May need to be a shade
						; longer. Compare timings
						; with Sinclair ROM
		ld hl, #_mdv_w_hdr_buf		; Header to write first
		ld a, (_mdv_csum)
		ld e, a
		ld a, #0x03			; Magneta border
		out (0xFE), a
		ld a, #0xE2
		out (0xEF), a			; Ready...
		nop				; FIXME: use a shorter
		nop				; way to wait 24 clocks
		nop
		nop
		nop
		nop
		ld c, #0xE7			; Data port
		ld b, #0x1B			; Header
		otir				; Header bytes out
		ld hl, (_mdv_buf)
		otir				; First 256 data
		otir				; Last 256 data
		out (c), e			; and the checksum
		ld a, #0xE6
		out (0xEF), a
		xor a
		out (0xFE), a			; Back in black
		ld a, #0xEE
		out (0xEF), a			; Writing off
		xor a
		ret

;
;	Template for writes
buftmplt:
		.db 0, 0, 0, 0
		.db 0, 0, 0, 0
		.db 0, 0, 0xff, 0xff		; preamble
		.db 0				; block not header
		.db 0				; sector
		.db 0				; length low
		.db 2				; length high
		.ascii "FUZIX     "		; filler basically
;
;	Write a sector from memory
;
mdv_store:	ld hl, (_mdv_buf)
		ld bc, #2			; 512 bytes
		call mdv_csum_data		; checksum into A
		ld (_mdv_csum), a		; save ready to write
		;
		; Now fill in the header on the data bloc
		;
		ld hl, #buftmplt
		ld de, #_mdv_w_hdr_buf
		ld bc, #14+MAP_PREAMBLE
		ldir
		ld a, (_mdv_sector)		; fill in the sector
		ld (_mdv_w_hdr_buf + MAP_PREAMBLE + 1), a
		call mdv_csum_w_hdr		; checksum
		ld (hl), a			; fill in the checksum
		in a, (0xef)
		and #1
		jr z, mdv_put_wp		; write protected
		call mdv_find_hdr		; find the header we want
		call z, mdv_put_blk		; write the block after it
		ret				; done
mdv_put_wp:	ld a, #4			; write protected - fail
		ret
;
;	Load a sector into memory.
;
mdv_fetch:	call mdv_find_hdr		; nz = not found
		call z, mdv_get_blk		; data if worked
		ret				; done


;
;	Microdrive motor control. This is basically a two wire clock/data
;	pair, shifted through the drives. We have a maximum of eight drives
;	so whenever we select we clock out 8 bits one of which turns on
;	a motor.
;

;
;	Turn all motors off
;
mdv_motors_off:	ld a, #0xff
		ld bc, #0x08EF
		jr mdv_motor_a
;
;	Turn on motor for microdrive unit A
;
mdv_motor:	ld bc, #0x08EF			; port EF, 8 cycles
		neg				; Clever way to get the
		add #0x09			; right bit number as used
mdv_motor_a:
		ld  e, a			; by the if1 firmware
;
;	Now we will do 8 cycles of bit banging clock and data
;
mdv_motor_lp:
		dec e				; are we there yet ?
		jr nz, mdv_motor_0		; send zero
;
;	Clock out an "on" bit
;
		ld a,#1
		out (0xF7), a			; data high
		ld a, #0xee
		out (c), a			; clock, !data
		call nap_1ms			; wait 1mS
		ld a, #0xec			; !clock !data
		jr mdv_motor_1			; into common path
;
;	Clock out an "off" bit
;
mdv_motor_0:
		ld a, c				; 0xEF
		out (0xEF), a			; clock 1
		xor a				; 0 to data
		out (0xF7), a
		call nap_1ms			; 1ms pulse 0
		ld a, #0xED			; clock 0

mdv_motor_1:	out (c), a
		call nap_1ms			; 1ms wait
		djnz mdv_motor_lp		; round we go
		ld a, #0x01
		out (0xF7), a			; 1 to data
		ld a, #0xEE			; clock high, !data
		out (0xEF), a			; done

;
;	"Spin" up the drive - in our case get the tape to drive speed
;
		ld bc, #13000
		jp nap


;
;	C language interfaces
;
;	int mdv_motors_off(void)
;
_mdv_motor_off:	call mdv_motors_off
		xor a
		out (0xfe), a
ret0:
		ld hl, #0
		ret

;
;	int mdv_motor_on(uint8_t drive)
;
_mdv_motor_on:	pop de
		pop hl
		pop bc
		push bc
		push hl
		push de
		ld a, c
		call mdv_motor
		jr ret0
;
;	int mdv_read(void)
;	mdv_sector and mdv_buf have been set up ready
;
;	This relies on the fact data is effectively common space on the
;	ZX128. It will break if this ceases to be true.
;
_mdv_bread:
		ld de, (current_map)		; Current map into e
		ld a, (_mdv_page)
		or a
		push de
		push af
		call nz, switch_bank		; Switch if mdv_page set
		call mdv_fetch			; Do the I/O
mdv_bout:
		jr nz, poprete			; Error codes for C
		xor a
poprete:
		ld l, a
		xor a
		ld h, a
		pop af
		pop de
		ld a, e
		call nz, switch_bank		; Switch bank if needed
		ret

_mdv_bwrite:
		ld de, (current_map)
		ld a, (_mdv_page)
		or a
		push de
		push af
		call nz, switch_bank
		call mdv_store
		jr mdv_bout

