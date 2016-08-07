;
; TR-DOS 5.03 calls
; Procedure addresses for another TR-DOS version may be different!

; If we are not sure what ROM page is active now (Basic-48 or Basic-48),
; then we need to set port 0x7FFD bit D4=1 manually because these hacks work
; correctly only when Basic-48 is mapped.

	.module betadisk_internal

	; exported symbols
	.globl _betadisk_seek_internal
	.globl _betadisk_read_internal

	.area _CODE

; _betadisk_seek_internal positions head to the requested track
; void betadisk_seek_internal(uint16_t track);

_betadisk_seek_internal:
	ld hl, #4  ; skipping return address and mapper word
	add hl, sp ; now hl points directly at the "track" argument
	ld a, (hl) ; it is uint16_t, but actually only lower byte matters (80 tracks max)
	or a       ; calculating needed side of the disk
	rra
	ld c,a
	ld a,#0x3C ; top side
	jr nc,01$
	ld a,#0x2C ; bottom side
01$:
	ld hl,#0x2F4D ; address of SEARCH TR-DOS procedure
	push hl
	jp 0x3D2F     ; classical way to perform jp (sp) inside TR-DOS ROM

; betadisk_read_internal reads desired sector from the current track
; void betadisk_read_internal(uint16_t sector, uint8_t* buf);

_betadisk_read_internal:
	pop ix        ; return address
	pop bc        ; mapper spacer
	pop de        ; sector number (only E matters)
	pop hl        ; buf address
	push hl
	push de
	push bc
	push ix
	ld bc,#0x2F1B ; address of READ SECTOR procedure of TR-DOS ROM
	push bc
	jp 0x3D2F     ; jp (SP)
