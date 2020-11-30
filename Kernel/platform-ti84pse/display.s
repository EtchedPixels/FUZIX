	; Exported symbols
	.module display
	.globl _init_display
	.globl _copy_display
	.globl _plot_char

	; Imported symbols
	.globl _font4x6

	.area _CODE

PORT_LCD_CMD                        .equ 0x10
PORT_LCD_DATA                       .equ 0x11
LCD_CMD_SETOUTPUTMODE               .equ 0
LCD_CMD_SETDISPLAY                  .equ 2
LCD_CMD_AUTOINCDEC_SETX             .equ 4
LCD_CMD_AUTOINCDEC_SETY             .equ 6
LCD_CMD_POWERSUPPLY_SETENHANCEMENT  .equ 8
LCD_CMD_MIRRORSCREEN                .equ 0x0C
LCD_CMD_POWERSUPPLY_SETLEVEL        .equ 0x10
LCD_CMD_CANCELTESTMODE              .equ 0x18
LCD_CMD_ENTERTESTMODE               .equ 0x1C
LCD_CMD_SETCOLUMN                   .equ 0x20
LCD_CMD_SETZSHIFT                   .equ 0x40
LCD_CMD_SETROW                      .equ 0x80
LCD_CMD_SETCONTRAST                 .equ 0xC0

_init_display:
	; Initialize LCD
	ld a, #1 + LCD_CMD_AUTOINCDEC_SETX
	call lcd_wait
	out (PORT_LCD_CMD), a ; X-Increment Mode

	ld a, #1 + LCD_CMD_SETOUTPUTMODE
	call lcd_wait
	out (PORT_LCD_CMD), a ; 8-bit mode

	ld a, #1 + LCD_CMD_SETDISPLAY
	call lcd_wait
	out (PORT_LCD_CMD), a ; Enable screen

	ld a, #7 + LCD_CMD_POWERSUPPLY_SETLEVEL
	call lcd_wait
	out (PORT_LCD_CMD), a ; Op-amp control (OPA1) set to max (with DB1 set for some reason)

	ld a, #3 + LCD_CMD_POWERSUPPLY_SETENHANCEMENT ; B
	call lcd_wait
	out (PORT_LCD_CMD), a ; Op-amp control (OPA2) set to max

	ld a, #0x2F + LCD_CMD_SETCONTRAST
	call lcd_wait
	out (PORT_LCD_CMD), a ; Contrast

	; XXX: Reserving page 81 for the display buffer is kind of shit but it
	; works out for now because we give 3 pages to each process (up to
	; two), plus one kernel page, plus the display buffer == all 8 pages
	; Clear display buffer
	ld a, #1
	out (5), a
	ld hl, #0xC000
	ld de, #0xC001
	ld bc, #96*64/8-1
	ld (hl), #0
	ldir
	dec a
	out (5), a
	ret

; C interface
_plot_char:
	pop hl
	pop de ; d, e: coords
	pop bc ; c: char
	push bc
	push de
	push hl
	; Fallthrough

; IN: D, E: col, row; C: ASCII character
plot_char:
	; XXX: This assumes that this code lives in low RAM
	; We re-map kernel RAM into bank B so we can access the font and the
	; display RAM simultaneously
	ld a, #0x80
	out (7), a	; Map font
	ld a, #1
	out (5), a	; Map display RAM

	; Look up character in font
	push de
	ld a, c
	ld h, #0
	and #0x7f
	ld l, a
	add hl, hl  ; x 2
	push hl
	add hl, hl  ; x 4
	pop de
	add hl, de  ; x 6
	ld de, #_font4x6 - 0x4000 ; adjusted for unusual mapping
	add hl, de  ; font base
	pop de

	call addr_de
	ex de, hl
	; de: character; hl: coordinates; z: right
	ld b, #6
	jr nz, right

left:	ld a, #0x0F
	and (hl)
	ld (hl), a
	ld a, (de)
	inc de
	and #0xF0
	or (hl)
	ld (hl), a
	ld a, #12
	add l
	jr nc, lnc
	inc h
lnc:	ld l, a
	djnz left
	jr plot_char_out
right:	ld a, #0xF0
	and (hl)
	ld (hl), a
	ld a, (de)
	inc de
	srl a
	srl a
	srl a
	srl a
	and #0x0F
	or (hl)
	ld (hl), a
	ld a, #12
	add l
	jr nc, rnc
	inc h
rnc:	ld l, a
	djnz right
	; Fallthrough
plot_char_out:
	; Restore mappings
	xor a
	out (5), a
	add #2
	out (7), a
	ret

; XXX: Technically we could use the fast version since we only support the
; original TI-84+SE hw revision
lcd_wait:
	push af
wait:	in a, (PORT_LCD_CMD)
	rla
	jr c, wait
	pop af
	ret

; Turn a co-ordinate pair in DE into an address in DE.
; Sets z flag if we're printing the right-hand character
addr_de:
	ld a, d	; X
	and #1
	push af

	ld a, e	; turn Y into a pixel row
	add a
	ld e, a
	add a
	add e	; E * 6 to get E = pixel row

	push hl
	push bc

	ld l, a
	ld h, #0
	add hl, hl ; * 2
	add hl, hl ; * 4
	ld b, h
	ld c, l
	add hl, hl ; * 8
	add hl, bc ; * 12; HL = row address

	; Set BC to base addr + col address
	ld b, #0xC0
	ld a, d
	srl a ; / 2
	ld c, a
	add hl, bc

	ex de, hl
	pop bc
	pop hl
	pop af
	ret

; Assumes display buffer is mapped
_copy_display:
	push hl
	push bc
	push af
	push de

	ld a, #1
	out (5), a ; Map Display RAM
	ld hl, #0xC000

	ld c, #PORT_LCD_CMD
	ld a, #LCD_CMD_SETROW
setrow:
	.db 0xED, 0x70 ; in f, (c) (undocumented instruction)
	jp m, setrow
	out (PORT_LCD_CMD), a
	ld de, #12
	ld a, #LCD_CMD_SETCOLUMN
col:
	.db 0xED, 0x70 ; in f, (c) (undocumented instruction)
	jp m, col
	out (PORT_LCD_CMD),a
	push af
	ld b, #64
row:
	ld a, (hl)
rowwait:
	.db 0xED, 0x70 ; in f, (c) (undocumented instruction)
	jp m, rowwait
	out (PORT_LCD_DATA), a
	add hl, de
	djnz row

	pop af
	dec h
	dec h
	dec h
	inc hl
	inc a
	cp #0x0C + LCD_CMD_SETCOLUMN
	jp nz, col

	xor a
	out (5), a ; Restore mappings

	pop de
	pop af
	pop bc
	pop hl
	ret
