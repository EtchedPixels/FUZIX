;
;	Video layer for the SAM. For the moment we use a fairly primitve
;	set up. We'd really like a mono hi-res mode but you don't get one
;
;	Our display consists of 192 lines of 128 bytes (24K) and is page
;	aligned.
;
;	We really want to do 80 column 6bit chars but start simple
;
;	Our video is 0 based so we don't add an offset
;	Must preserve BC
;

	.module sam_vt

	.area _VIDEO

	.globl _scroll_up
	.globl _scroll_down
	.globl _plot_char
	.globl _clear_lines
	.globl _clear_across

	.globl _cursor_on
	.globl _cursor_off
	.globl _cursor_disable

	.globl _vtattr_notify
	.globl _vtattr_cap

	.globl _vtwipe

	.globl map_video
	.globl unmap_video

_fontdata_8x8_exp2	.equ	0x6000	; in video bank

VIDEO_PAGE	.equ	4

_vtattr_cap:
	.byte	0	; for now- we can add funky effects later

base_addr:
	ld a,d		; save X
	ld d,e		; 256 * Y
	ld e,#0
	sla e
	sla d		; this is DE = E * 512
	sla e
	sla d		; this is DE = E * 1024

	; A is a char 0-63 - need a byte 0-126
	; Aligned so no carry issues
	add a
	add e
	ld e,a
	ret

;
;	This is awkward. We can't map the video in 0000-7FFF without
;	interrupts off, and we can't map it high without confusing the
;	hell out of everything else. For now just di. 32K/32K splits suck
;
_scroll_up:
	call map_video
	ld hl,#1024
	ld de,#0
	ld bc,#24576-1024
	ldir
pop_unmap:
	jp unmap_video

_scroll_down:
	call map_video
	ld hl,#24575-1024
	ld de,#24575
	ld bc,#24576
	lddr
	jp unmap_video

_plot_char:
	pop hl			; return
	pop de			; H = X L = Y
	pop bc			; C = char
	push bc
	push de
	push hl
	call base_addr
	ld h,#0
	ld l,c
	add hl,hl		; char * 16 (expanded 8 x 8)
	add hl,hl
	add hl,hl
	add hl,hl
	ld bc,#_fontdata_8x8_exp2	; plus font base
	add hl,bc
	call map_video
	ld bc,#0x04FF		; it gets decremented by DJNZ while the FF
				; ensures the ldi never decrements B
;
;	Need to add a slow path version that does colour and effects. Colour
;	is a matter of masking with FF, CC, 33 or 00.
;
plot_loop:
	ldi			; copy expanded char
	ld a,(hl)
	ld (de),a
	inc hl
	dec e
	set 7,e			; de += 126
	ldi			; copy second char row
	ld a,(hl)
	ld (de),a
	inc hl
	dec e			; toggle the bit back and gives us
	res 7,e
	inc d			; de += 126
	djnz plot_loop
	jr pop_unmap

;
;	We will need this once we wire in colour support
;
plot_slow:
	ld a,#4
	exx
	ld de,(colours)
	exx
plot_loop_slow:
	ex af,af'
	ld a,(hl)
	exx
	ld b,a
	cpl			; background bits
	and d			; to background colour
	ld c,a
	ld a,e			; foreground bits
	and b			; to foreground colour
	or c			; plus background
	exx
	ld (de),a
	ld a,(hl)
	inc de
	inc hl
	exx
	ld b,a
	cpl			; background bits
	and d			; to background colour
	ld c,a
	ld a,e			; foreground bits
	and b			; to foreground colour
	or c			; plus background
	exx
	ld (de),a
	add hl,bc
	ex af,af'
	dec a
	jr nz, plot_loop_slow
	jp unmap_video

_clear_lines:
	pop hl
	pop de
	push de
	push hl
	ld a,d			; count (0-23)
	or a
	ret z			; zero count is possible - do nothing
	ld d,#0
	; Now how much to copy ?
	; 128 bytes per scan line, 8 scan lines per char = 1024
	add a,a			; x 2
	add a,a			; x 4
	ld b,a			; x 1024
	ld c,d			; d is 0
	call base_addr
	call map_video
	ld h,d
	ld l,e
	inc de
	dec bc
	ld (hl),#0
	ldir
	jp unmap_video

_clear_across:
	pop hl
	pop de		; DE = X/Y
	pop bc		; C = count
	push bc
	push de
	push hl

	call base_addr
	call map_video

	xor a
	ld l,#4	; 4 line pairs
	ld h,e	; save offset start
wipe_line:
	ld b,c
wipe_rowpair:
	ld (de),a
	set 7,e
	ld (de),a
	res 7,e
	inc de
	djnz wipe_rowpair
	ld e,h	; restore offset. As we are 256 byte aligned we don't need
	inc d	; to worry about carries just restore low, inc high
	dec l
	jr nz, wipe_line
	jp unmap_video

_vtwipe:
	call map_video
	ld hl,#0
	ld de,#1
	ld bc,#24575
	ld (hl),#0
	ldir
	jp unmap_video
;
;	TODO
;
_cursor_on:
	pop hl
	pop de
	push de
	push hl
	call base_addr
	ex de,hl
	inc h		; Move on 6 scan lines
	inc h		; So the cursor is bottom 2
	inc h
	ld (cursorpos),hl
	jr xor_cursor
;
;	Remove our cursor
;
_cursor_disable:
_cursor_off:
	ld hl,(cursorpos)
	ld a,h
	inc a
	ret z
	ld a,#255
	ld (cursorpos),a
xor_cursor:
	call map_video
	call xorhl
	ld de,#127
	add hl,de
	call xorhl
	jp unmap_video

xorhl:
	ld a,(hl)
	cpl
	ld (hl),a
	inc hl
	ld a,(hl)
	cpl
	ld (hl),a
	ret

_vtattr_notify:
	ret

colours:
	.word 0
cursorpos:
	.word 0xFFFF
