;
;        amstrad cpc vt primitives
;
        ;imported symbols
        .globl _int_disabled
        .globl _outputtty
        .globl _inputtty
        
        ; exported symbols
        .globl cpc_plot_char
        .globl cpc_scroll_down
        .globl cpc_scroll_up
        .globl cpc_cursor_on
        .globl cpc_cursor_off
	.globl cpc_cursor_disable
        .globl cpc_clear_lines
        .globl cpc_clear_across
        .globl cpc_do_beep
	.globl _fontdata_8x8
	.globl _curattr
	.globl _vtattr


	.if CPCVID_ONLY
_plot_char:
	.endif
cpc_plot_char:
        pop hl
        pop de              ; D = x E = y
        pop bc
        push bc
        push de
        push hl
	push de
        push bc
        call videopos
        pop bc
        ld h, #0            ; calculating offset in font table
        ld l, c
        add hl,hl
        add hl,hl
        add hl,hl
        ld bc, #_fontdata_8x8
        add hl, bc          ; hl points to first byte of char data
        di
        ld bc, #0x7faa ;RMR ->UROM disable LROM enable
        out (c),c
        ld b,#7
        ld a,(hl)
        VIDEO_MAP
        ld (de),a
plot_char_line:
        ld a,d
        add a,#8             
        ld  d,a              
        inc hl
        ld a,(hl)
        ld (de),a
        djnz plot_char_line
        ld bc, #0x7fae ;RMR ->UROM disable LROM disable
        out (c),c
        ex af,af'
        ld a,(_int_disabled)
        or a
        jr nz,cont_no_int_char
        ei
cont_no_int_char:
        ex af,af'
	; We do underline for now - not clear italic or bold are useful
	; with the font we have.
	ld bc,(_vtattr)	    ; c is vt attributes
	bit 1,c		    ; underline ?
	jr nz, last_ul
plot_attr:
        ld (de), a
	pop de
	VIDEO_UNMAP
        ret
last_ul:
	ld a,#0xff
	jr plot_attr

	.if CPCVID_ONLY
_clear_lines:
	.endif
cpc_clear_lines:
        pop hl
        pop de              ; E = line, D = count
        push de
        push hl
	; This way we handle 0 correctly
	inc d
	jr nextline

clear_next_line:
        push de
        ld d, #0            ; from the column #0
        ld b, d             ; b = 0
        ld c, #64           ; clear 64 cols
        push bc
        push de
        call _clear_across
        pop hl              ; clear stack
        pop hl
        pop de
        inc e

nextline:
        dec d
        jr nz, clear_next_line
        ret

	.if CPCVID_ONLY
_clear_across:
	.endif
cpc_clear_across:
        pop hl
        pop de              ; DE = coords 
        pop bc              ; C = count
        push bc
        push de
        push hl
	ld a,c
	or a
	ret z		    ; No work to do - bail out
	push de
	push bc
        call videopos       ; first pixel line of first character in DE
        pop bc
        push de
        pop hl              ; copy to hl
        ld b,#8
        VIDEO_MAP
clear_line:
        ld a, b
        push af
        xor a
        push bc
clear_scanline:        
        ld (de),a
        inc de
        dec c
        jr nz, clear_scanline
        ex de, hl
        ld a,d               
        add a,#8             
        ld  d,a              
        pop bc
        pop af
        ld b,a
        push de
        pop hl
        djnz clear_line
	pop de
	VIDEO_UNMAP
        ret

	.if CPCVID_ONLY
_cursor_on:
	.endif
cpc_cursor_on:
        pop hl
        pop de
        push de
        push hl
        call videopos
        ld a,d               
        add a,#0x38             
        ld d,a
        VIDEO_MAP
        ld a,(de)
        ld (saved_cursor_byte),a
        ld (cursorpos), de
        ld a,#0xFF
        ld (de),a

	VIDEO_UNMAP
        ret

	.if CPCVID_ONLY
_cursor_disable:
_cursor_off:
	.endif
cpc_cursor_disable:
cpc_cursor_off:
        VIDEO_MAP

        ld de, (cursorpos)              
        ld a,(saved_cursor_byte)
        ld (de),a
	
        VIDEO_UNMAP
	ret

_curattr:
	.db 7
cursorpos:
        .dw 0
saved_cursor_byte:
        .db 0        

.area _VIDEO

	.if CPCVID_ONLY
_scroll_up:
	.endif
cpc_scroll_up:
        ld hl, (CRTC_offset)
        ld bc, #32           ; one crtc character are two bytes
        add hl,bc

set_hardware_scroll:
        ld a,h
        and #3
        ld h,a
        ld (CRTC_offset),hl
        add hl,hl
        ld (scroll_offset),hl   ;prepare scroll_offset for videopos
        ld a,(_inputtty)        ;maybe its simpler check if inputtty == outputtty?
        dec a
        jr z,check_tty1
        ld a,(#screenpage)
        cp #0x20
        jp z,do_crtc_scroll
        ret
check_tty1:
        ld a,(#screenpage)
        cp #0x10
        ret nz
do_crtc_scroll:
        ld bc,#0xbc0c           ;select CRTC R12
        out (c),c
        ld b,#0xbd                
        ld hl,(CRTC_offset)
        or h
        out (c),a
        ld bc,#0xbc0d           ;select CRTC R13
        out (c),c
        ld b,#0xbd
        out (c),l
        ret 

	.if CPCVID_ONLY
_scroll_down:
	.endif
cpc_scroll_down:
        ld hl, (CRTC_offset)
        ld bc, #32           ; one crtc character are two bytes
        or a
        sbc hl,bc
        jr set_hardware_scroll

videopos: ;get x->d, y->e => set de address for top byte of char
        ld l,e
        ld h,#0
        add hl,hl
        add hl,hl
        add hl,hl
        add hl,hl
        add hl,hl
        add hl,hl
        ld e,d
        ld a,(#screenbase)
        ld d,a
        add hl,de
        ld de,(#scroll_offset)
        add hl,de
        cp #0x80
        jr z,tty2
        res 7,h
        set 6,h
        jr last_fix
tty2:        
        set 7,h
        res 6,h
last_fix:
        res 3,h
        ex de,hl
	ret 

_ga_set_active_vt:
	ld a,(_outputtty)
        dec a
        jr z,tty1
        ld a,#0x80
        ld (#screenbase),a
        ld a,#0x20
        ld (#screenpage),a
        jr switch_vt_values

tty1:
        ld a,#0x40
        ld (#screenbase),a
        ld a,#0x10
        ld (#screenpage),a

switch_vt_values:
        ld hl,(#CRTC_offset)
        push hl
        ld hl,(#scroll_offset)
        push hl
        ld hl,(#cursorpos)
        push hl
        ld a,(#saved_cursor_byte)
        push af
        ld a,(#_curattr)
        push af
        ld a,(#curattr_old)
        ld (#_curattr),a
        pop af 
        ld (#curattr_old),a
        ld a,(#saved_cursor_byte_old)
        ld (#saved_cursor_byte),a
        pop af
        ld (#saved_cursor_byte_old),a
        ld hl,(#cursorpos_old)
        ld (#cursorpos),hl
        pop hl
        ld (#cursorpos_old),hl
        ld hl,(#scroll_offset_old)
        ld (#scroll_offset),hl
        pop hl
        ld (#scroll_offset_old),hl
        ld hl,(#CRTC_offset_old)
        ld (#CRTC_offset),hl
        pop hl
        ld (#CRTC_offset_old),hl
        ret    

_ga_set_visible_vt:
        ld a,(_inputtty)
        dec a
        jr z,vtty1
        ld a,#0x20
        jr select_offset
vtty1:
        ld a,#0x10
select_offset:
        push af
        ld a,(_inputtty)
        ld b,a
        ld a,(_outputtty)
        cp b
        jr z, same_offset
        ld hl,(CRTC_offset_old)
        jr out_offset
same_offset:
        ld hl,(CRTC_offset)
out_offset:                     ;can this be shared with hardware scrolling?
        ld bc,#0xbc0c           ;select CRTC R12
        out (c),c
        ld b,#0xbd
        pop af               
        or h
        out (c),a
        ld bc,#0xbc0d           ;select CRTC R13
        out (c),c
        ld b,#0xbd
        out (c),l
	ret

        

screenbase:
	.db 0x40
screenpage:
        .db 0x10
scroll_offset:
        .dw 0
scroll_offset_old:
        .dw 0
CRTC_offset:
        .dw 0
CRTC_offset_old:
        .dw 0
curattr_old:
	.db 7
cursorpos_old:
        .dw 0
saved_cursor_byte_old:
        .db 0

	.if CPCVID_ONLY
_do_beep:
	.endif
cpc_do_beep:
        ld e,#4
        ld d,#38        ;channel C 110Hz
        call write_ay_reg
        ld e,#5
        ld d,#2        ;channel C 110Hz
        call write_ay_reg
        ld e,#7          
        ld d,#0x3b       ;mixer->Only channel C
        call write_ay_reg
        ld e,#0xa
        ld d,#0x10      ;Use envelope on C
        call write_ay_reg
        ld e,#0xb
        ld d,#0x86      ;100ms envelope period
        call write_ay_reg
        ld e,#0xc
        ld d,#0x1      ;100ms envelope period
        call write_ay_reg
        ld e,#0xd
        ld d,#0x9         ;Ramp down in one cicle and remain quiet
write_ay_reg: ; E = register, D = data from https://cpctech.cpc-live.com/source/sampplay.html
        ld b,#0xf4
        out (c),e
        ld bc,#0xf6c0
        out (c),c
        ld c,#0
        out (c),c
        ld b,#0xf4
        out (c),d
        ld bc,#0xf680
        out (c),c
        ld c,#0
        out (c),c
        ret