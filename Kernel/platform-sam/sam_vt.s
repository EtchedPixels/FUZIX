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

	.globl map_video
	.globl unmap_video
	.globl ___hard_di
	.globl _fontdata_8x8_exp2

VIDEO_PAGE	.equ	4

_vtattr_cap:
	.byte	0	; for now- we can add funky effects later

base_addr:
	ld a,d		; save X
	ld d,e		; 256 * Y
	ld e,#0
	srl d
	srl e		; this is DE = E * 128
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
	call ___hard_di
	push af
	call map_video
	ld hl,#128
	ld de,#0
	ld bc,#24576-128
ldir_pop:
	ldir
pop_unmap:
	call unmap_video
	pop af
	ret c
	ei
	ret

_scroll_down:
	call ___hard_di
	push af
	call map_video
	ld hl,#24576
	ld de,#24576-128
	ld b,d
	ld c,e
	lddr
	jr pop_unmap

_plot_char:
	pop hl			; return
	pop de			; H = X L = Y
	pop bc			; C = char
	push bc
	push de
	push hl
	call ___hard_di
	push af
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
	ld b,#20		; it gets decremented by 4 by the ldi's
				; and once by djnz, and we need to do it
				; four times
plot_loop:
	ldi			; copy expanded char
	ldi
	dec e
	dec e
	set 7,e			; de += 126
	ldi			; copy second char row
	ldi
	dec e			; Similar idea
	dec e			; toggle the bit back and gives us
	res 7,e
	inc d			; de += 126
	djnz plot_loop
	jr pop_unmap

_clear_lines:
	pop hl
	pop de
	push de
	push hl
	call ___hard_di
	push af
	ld a,d			; count (0-23)
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
	ld e,l
	inc de
	dec bc
	ld (hl),#0
	jr ldir_pop

_clear_across:
	pop hl
	pop de		; DE = X/Y
	pop bc		; C = count
	push bc
	push de
	push hl

	call ___hard_di
	push af

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
	jp pop_unmap

;
;	TODO
;
_cursor_on:
_cursor_off:
_cursor_disable:
_vtattr_notify:
	ret
