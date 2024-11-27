;
;        amstrad cpc vt primitives
;
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
        ld d,#0
        add hl,de
        ld a,#SCREENBASE
        ld b,a
        ld c,#0
        add hl,bc
        ld de,(#scroll_offset)
        add hl,de
        res 7,h
        set 6,h
        res 3,h
        ex de,hl
        VIDEO_MAP
	ret     

;from cpc firmware: https://github.com/Bread80/CPC6128-Firmware-Source/tree/main
 
scr_next_line:                    ;{{Addr=$0c1f Code Calls/jump count: 10 Data use count: 1}}
        ld      a,d               ;{{0c1f:7c}} 
        add     a,#8             ;{{0c20:c608}} 
        ld      d,a               ;{{0c22:67}} 
;        and     #0x38               ;{{0c23:e638}} 
;        ret     nz                ;{{0c25:c0}} 
;        ld      a,d               ;{{0c26:7c}} 
;        sub     #0x40               ;{{0c27:d640}} 
;        ld      d,a               ;{{0c29:67}} 
;        ld      a,e               ;{{0c2a:7d}} 
;        add     a,#64             ;{{0c2b:c650}} ; number of bytes per line
;        ld      e,a               ;{{0c2d:6f}} 
;        ret     nc                ;{{0c2e:d0}} 
;        inc     d                 ;{{0c2f:24}} 
;        ld      a,d               ;{{0c30:7c}} 
;        and     #0x07               ;{{0c31:e607}} 
;        ret     nz                ;{{0c33:c0}} 
;        ld      a,d               ;{{0c34:7c}} 
;        sub     #0x08               ;{{0c35:d608}} 
;        ld      d,a               ;{{0c37:67}}
        res 7,d
        set 6,d
        ret                 

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

        ld b,#7
        ld a,(hl)
        ld (de),a
plot_char_line:
        call scr_next_line
        inc hl
        ld a,(hl)
        ld (de),a
        djnz plot_char_line

;Fastest way to print a character in Mode 2 : https://www.cpcwiki.eu/index.php/Programming:Next_/_previous_line_calculation
;
;Input: HL=Address of sprite/char.data DE=screen address (works for every first line of screen !! For example: &C000, &C050, &C0A0 etc..)
;
;Destroyed: AF, C
;
;Unchanged: DE returns to its input value (through C register)
;
;'If HL is not page aligned, INC HL must be used instead of INC L (1 NOP SLOWER!)
;
;
;        ld c,d
;        ld a,(hl)
;        ld (de),a
;        inc hl
;        set 3,d
;        ld a,(hl)
;        ld (de),a
;        inc hl
;        ld d,c
;        set 4,d
;        ld a,(hl)
;        ld (de),a
;        inc hl
;        set 3,d
;        ld a,(hl)
;        ld (de),a
;        inc hl
;        ld d,c
;        set 5,d
;        ld a,(hl)
;        ld (de),a
;        inc hl
;        set 3,d
;        ld a,(hl)
;        ld (de),a
;        inc hl
;        set 4,d
;        res 3,d
;        ld a,(hl)
;        ld (de),a
;        inc hl
;        set 3,d
;        ld a,(hl)
        
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

        ; no boundary checks. Assuming that D + C < SCREEN_WIDTH
        

        ;ld c,d
        ;ld (de),a
        ;set 3,d
        ;ld (de),a
        ;ld d,c
        ;set 4,d
        ;ld (de),a
        ;set 3,d
        ;ld (de),a
        ;ld d,c
        ;set 5,d
        ;ld (de),a
        ;set 3,d
        ;ld (de),a
        ;set 4,d
        ;res 3,d
        ;ld (de),a
        ;set 3,d
        ;ld (de),a
        
        ld b,#8
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
        call scr_next_line
        pop bc
        pop af
        ld b,a
        push de
        pop hl
        djnz clear_line
	pop de
	VIDEO_UNMAP
        ret

set_hardware_scroll:
        ld bc,#0xbc0c           ;select CRTC R12
        out (c),c
        ld b,#0xbd
        ld a,#0x30                ;video page at 0x4000, but for crtc remains at &c000
        or h
        out (c),a
        ld bc,#0xbc0d           ;select CRTC R13
        out (c),c
        ld b,#0xbd
        out (c),l
        add hl,hl
        ld (scroll_offset),hl   ;prepare scroll_offset for videopos
        ret
	.if CPCVID_ONLY
_scroll_up:
	.endif
cpc_scroll_up:
        ld hl, (CRTC_offset)
        ld bc, #32           ; one crtc character are two bytes
        add hl,bc
        ld a,h
        and #3
        ld h,a
        ld (CRTC_offset),hl
        jr set_hardware_scroll
 
	.if CPCVID_ONLY
_scroll_down:
	.endif
cpc_scroll_down:
        ld hl, (CRTC_offset)
        ld bc, #32           ; one crtc character are two bytes
        or a
        sbc hl,bc
        ld a,h
        and #3
        ld h,a
        ld (CRTC_offset),hl
        jr set_hardware_scroll
        ret

	.if CPCVID_ONLY
_cursor_on:
	.endif
cpc_cursor_on:
        pop hl
        pop de
        push de
        push hl
        ld (cursorpos), de
        call videopos
        
        ld b,#7
set_cursor_line:
        call scr_next_line
        djnz set_cursor_line
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
        ld de, (cursorpos)
        call videopos

        ld b,#7
reset_cursor_line:
        call scr_next_line
        djnz reset_cursor_line
        xor a
        ld (de),a
	
        VIDEO_UNMAP
	ret

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
        call write_ay_reg
        ret

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

        .area _DATA

cursorpos:
        .dw 0   
scroll_offset:
        .dw 0
CRTC_offset:
        .dw 0

	.area _COMMONMEM
_curattr:
	.db 7
