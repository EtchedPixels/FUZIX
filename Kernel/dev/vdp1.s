            .module vdp

	    ; video driver
	    .globl scroll_up
	    .globl scroll_down
	    .globl plot_char
	    .globl clear_lines
	    .globl clear_across
	    .globl cursor_on
	    .globl cursor_off
	    .globl _cursor_disable


	    .globl cursorpos

	    .globl _int_disabled

	    .globl _vdpport

	    .globl _vdp_init
	    .globl _vdp_type
	    .globl _vdp_load_font
	    .globl _vdp_wipe_consoles
	    .globl _vdp_restore_font
	    .globl _vdp_text40
	    .globl _vdp_text32
	    .globl _vdp_setup40
	    .globl _vdp_setup32
	    .globl _vdp_set
	    .globl _vdp_readb
	    .globl _vdp_out
	    .globl _vdp_setup
	    .globl _vdp_set_console
	    .globl _vdp_setborder
	    .globl _vdp_setcolour

	    .globl plt_interrupt_all

	    .globl _fontdata_6x8
	    .globl _vtattr_notify

	    .globl _inputtty
	    .globl _outputtty
	    .globl _vidmode
	    .globl outcharhex


	    .area _CODE

;
;	These are with the IRQ on. Subtract 0x20 from R1 for IRQ off
;

.if VDP_IRQ
_vdp_text40:
	    .byte 0x00, 0xF0, 0x00, 0x00, 0x02, 0x00, 0x00, 0xF4
_vdp_text32:
	    .byte 0x00, 0xE2, 0x00, 0x80, 0x02, 0x76, 0x03, 0xF4

.else

_vdp_text40:
	    .byte 0x00, 0xF0, 0x00, 0x00, 0x02, 0x00, 0x00, 0xF4
_vdp_text32:
	    .byte 0x00, 0xE2, 0x00, 0x80, 0x02, 0x76, 0x03, 0xF4

.endif

;
;	vdp_setup(uint8_t *table)
;
;	Initialize the VDP using the table
;
_vdp_setup:
	    ld d,#0x7F			; 0x80 is first register
	    ld bc, (_vdpport)
	    ld b,#16
setupl:	    outi
	    inc d
	    VDP_DELAY
	    out (c),d
	    djnz setupl
	    ret
;
;	Read a VDP byte - C
;
_vdp_readb:
	    call _vdp_set
	    dec c
	    VDP_DELAY			; Data is not immediately available
	    in l,(c)
	    ret

_vdp_out:
	    ld bc,(_vdpport)
	    dec c
	    out (c),l			; Write will be queued but must
	    ret				; not do another op too soon after
;
;	vdp_setup40(void)
;	vdp_init(void)
;	vdp_setup32(void)
;
;	Video mode setup
;
_vdp_init:
_vdp_setup40:
	    ld hl,#_vdp_text40
	    call _vdp_setup
	    ld hl, #scrollconf40
setscroll:
	    ld de, #_scrolld_base
	    ld bc, #10
	    ldir
	    ret

_vdp_setup32:
	    ld hl,#_vdp_text32
	    call _vdp_setup
	    ld hl, #scrollconf32
	    jr setscroll
;
;	vdp_type(void)	-	return the type of VDP present
;
_vdp_type:
	    ; Program the video engine

	    ld bc,(_vdpport)
	    ; Play with status register 2
	    ld a,#0x07
	    out (c),a
	    ld a,#0x8F
	    out (c),a
	    VDP_DELAY2
	    in a,(c)
	    ld a,#2
	    out (c),a
	    ld a,#0x8F
	    out (c),a
	    VDP_DELAY2
vdpwait:    in a,(c)
	    and #0xE0
	    add a
	    jr nz, not9918a	; bit 5 or 6
	    jr nc, vdpwait	; not bit 7
	    ; we vblanked out - TMS9918A
	    ld l,#0
	    ret
not9918a:   ; Use the version register
	    ld a,#1
	    out (c),a
	    ld a,#0x8F
	    out (c),a
	    VDP_DELAY2
	    in a,(c)
	    rrca
	    and #0x1F
	    inc a
	    ld l,a
	    xor a
	    out (c),a
	    ld a,#0x8F
	    out (c),a
	    ret

;
;	Load the font into the stash area
;
_vdp_load_font:
	    ld bc,(_vdpport)
	    ld de,#0x7C00
	    out (c),e			; set write and font area
	    VDP_DELAY3
	    out (c),d
	    ld hl,#_fontdata_6x8
	    ld b,e
	    dec c
wipelow:			; below font
	    out (c),e
	    VDP_DELAY
	    djnz wipelow
	    call lf256
	    call lf256
lf256:	    ; b is 0 at this point
	    VDP_DELAY
            ld a,(hl)
	    inc hl
	    add a
	    add a
            out (c),a
            djnz lf256
	    ret

;
;	vdp_restore_font(void)
;
;	Reset the font from the cache at 3C00-3FFF in the video memory
;
_vdp_restore_font:
	    ld de,#0x3c00
	    ld hl,#0x5054
	    ld bc,(_vdpport)
	    in a,(c)
	    VDP_DELAY		; needed for devices that don't waitstate
				; properly
fontnext:
	    out (c),e
	    VDP_DELAY3
	    out (c),d
	    VDP_DELAY2
	    dec c
	    in a,(c)
	    inc c
	    VDP_DELAY		; shouldn't need a normal delay here
				; but this covers devices without
				; proper wait states
	    out (c),e
	    VDP_DELAY3
	    out (c),h
	    dec c
	    out (c),a
	    VDP_DELAY
	    inc c
	    out (c),e
	    VDP_DELAY3
	    out (c),l
	    cpl
	    dec c
	    out (c),a
	    VDP_DELAY
	    inc c
	    inc e
	    jr nz, fontnext
	    inc d
	    inc h
	    inc l
	    bit 6,d
	    jr z, fontnext
	    ret

_vdp_wipe_consoles:
	    ld bc, (_vdpport)
	    ld b,#0
	    ld a,#0x40
	    out (c),b			; 0x0000 for writing
 	    VDP_DELAY3
	    out (c),a
	    dec c
	    ld a,#32
	    ld d,#16			; 4K
wipe_con:
	    out (c),a
	    VDP_DELAY
	    djnz wipe_con
	    dec d
	    jr nz, wipe_con
	    ret

_vdp_setcolour:
	    ld bc,(_vdpport)
	    ld b,#0
	    ld a,#0x60			; 0x2000 in the VDP, for write
	    out (c),b
	    VDP_DELAY3
	    out (c),a
	    dec c
	    ld b,#32
	    ld a,l			; Hardcoded for now
set_cmap:
	    out (c),a
	    VDP_DELAY
	    djnz set_cmap
	    ret
;
;	Register write value E to register A
;
_vdp_setborder:
	    ld h,#0x87			; colour register
	    ; fall through
_vdp_set:
	    ex de, hl
vdpout:	    ld bc, (_vdpport)
	    out (c), e			; Write the data
	    VDP_DELAY3
	    out (c), d			; and then the register | 0x80
	    ret

;
;	FIXME: need to IRQ protect the pairs of writes
;


videopos:	; turn E=Y D=X into HL = addr
	        ; pass B = 0x40 if writing
	        ; preserves C
	    ld a, (_outputtty)
	    dec a
	    add a			; 1K per screen
	    add a
	    add b
	    ld b,a
	    ld a, e			; 0-24 Y
	    add a, a
	    add a, a
	    add a, a			; x 8
	    ld l, a
	    ld h, #0
	    ld a, (_vidmode)
	    or a
	    jr nz, pos32
	    push hl
	    add hl, hl			; x 16
	    add hl, hl			; x 32
	    ld a, d
	    pop de
	    add hl, de			; x 40
	    ld e, a
	    ld d, b			; 0 for read 0x40 for write
	    add hl, de			; + X
	    ret
pos32:
	    add hl,hl
	    add hl,hl
            ld e,d
	    ld d,b
	    add hl,de
	    ret
;
;	Eww.. wonder if VT should provide a hint that its the 'next char'
;
		.if VDP_DIRECT
_plot_char:
		.endif
plot_char:  pop hl
	    pop de			; D = x E = y
	    pop bc
	    push bc
	    push de
	    push hl
	    ld a,(_int_disabled)
	    push af
	    di
plotit:
	    ld b, #0x40			; writing
	    call videopos
	    ld a, c
plotit2:
	    ld bc, (_vdpport)
	    out (c), l			; address
	    VDP_DELAY3
	    out (c), h			; address | 0x40
	    dec c
	    out (c), a			; character
popret:
	    pop af
	    or a
	    ret nz
	    ei
	    ;
	    ;	We assume our worst case char-char delay is sufficient to
	    ;   skip a VDP delay here. This ought to be true even on a fast
	    ;	Z180 box.
	    ;
	    ret

;
;	Painful
;
	    .area	_DATA

scrollbuf:   .ds		40

	    .area	_CODE
;
;	scroll_down(void)
;
		.if VDP_DIRECT
_scroll_down:
		.endif
scroll_down:
	    ld a, (_outputtty)
	    dec a
	    add a			; 1K per screen
	    add a
	    ld h,a
	    ld l,#0
	    ld b, #23
	    ld de, (_scrolld_base) ; start of bottom line
	    add hl,de
	    ex de,hl
upline:
	    push bc
	    ld bc, (_vdpport)	; vdpport + 1 always holds #80
	    ld hl, #scrollbuf
	    out (c), e		; our position
	    VDP_DELAY3
	    out (c), d
	    dec c
down_0:
	    VDP_DELAY2
	    ini
	    jp nz, down_0
	    inc c
	    ld hl, (_scrolld_s1); go down one line and into write mode
	    add hl, de		; relative to our position
	    out (c), l
	    VDP_DELAY3
	    out (c), h
	    ld b, #40
	    ld hl, #scrollbuf
	    dec c
down_1:
	    outi		; video ptr is to the line below so keep going
	    VDP_DELAY
	    jp nz,down_1
	    pop bc		; recover line counter
	    ld hl, (_scrolld_mov)
	    add hl, de		; up 40 bytes
	    ex de, hl		; and back into DE
	    djnz upline
	    jp popret

;
;	scroll_up(void)
;
		.if VDP_DIRECT
_scroll_up:
		.endif
scroll_up:
	    ld a,(_int_disabled)
	    push af
	    di
	    ld a, (_outputtty)
	    dec a
	    add a			; 1K per screen
	    add a
	    ld h,a
	    ld l,#0
	    ld b, #23
	    ld de, (_scrollu_w)		; start of second line (base = width)
	    add hl,de
	    ex de,hl
downline:   push bc
	    ld bc, (_vdpport)
	    ld hl, #scrollbuf
	    out (c), e
	    VDP_DELAY3
	    out (c), d
	    dec c
up_0:
	    VDP_DELAY2
	    ini
	    jp nz, up_0
	    inc c
	    ld hl, (_scrollu_mov); up w bytes in the low 12 bits, add 0x40
				; for write ( we will carry one into the top
				; nybble)
	    add hl, de
	    out (c), l
	    VDP_DELAY3
	    out (c), h
	    dec c
	    ld hl, #scrollbuf
	    ld a,(_scrollu_w)	; get width
	    ld b, a
up_1:
	    outi
	    VDP_DELAY
	    jp nz,up_1
	    pop bc
	    ld hl, (_scrollu_w)	; get width
	    add hl, de
	    ex de, hl
	    djnz downline
	    jp popret

		.if VDP_DIRECT
_clear_lines:
		.endif
clear_lines:
	    pop hl
	    pop de	; E = line, D = count
	    push de
	    push hl
	    xor a
	    cp d
	    ret z
	    ld a,(_int_disabled)
	    push af
	    di
	    ld c, d
	    ld d, #0
	    ld b, #0x40
	    call videopos
	    ld e, c
	    ld bc, (_vdpport)
	    out (c), l
	    VDP_DELAY3
	    out (c), h
				; Safe on MSX 2 to loop the data with IRQ on
				; but *not* on MSX 1
	    dec c
l2:	    ld a, (_scrollu_w)
	    ld b,a
            ld a, #' '
l1:	    out (c), a		; Inner loop clears a line, outer counts
				; need 20 clocks between writes. DJNZ is 13,
				; out is 11
	    VDP_DELAY
            djnz l1
	    dec e
	    jr nz, l2
	    jp popret

		.if VDP_DIRECT
_clear_across:
		.endif
clear_across:
	    pop hl
	    pop de	; DE = coords
	    pop bc	; C = count
	    push bc
	    push de
	    push hl
	    xor a
	    cp c
	    ret z
	    ld a,(_int_disabled)
	    push af
	    di
	    ld b, #0x40
	    call videopos
	    ld a, c
	    ld bc, (_vdpport)
	    out (c), l
	    VDP_DELAY3
	    out (c), h
	    ld b, a
            ld a, #' '
	    dec c
l3:	    out (c), a
	    VDP_DELAY
            djnz l3
	    jp popret

;
;	cursor_on()		-	cursor on save old char
;
		.if VDP_DIRECT
_cursor_on:
		.endif
cursor_on:
	    pop hl
	    pop de
	    push de
	    push hl
	    ld a,(_outputtty)
	    ld c,a
	    ld a,(_inputtty)
	    cp c
	    ret nz
	    ld a,(_int_disabled)
	    push af
	    di
	    ld (cursorpos), de
	    ld b, #0x00			; reading
	    call videopos
	    ld a, c
	    ld bc, (_vdpport)
	    out (c), l			; address
	    VDP_DELAY3
	    out (c), h			; address
	    dec c
	    VDP_DELAY2
	    in a, (c)			; character
	    ld (cursorpeek), a		; save it away
	    set 6, h			; make it a write command
	    xor #0x80			; write the cursor in inverse
	    jp plotit2

;
;	cursor_off(void)	-	turn the cursor off, restore char
;
		.if VDP_DIRECT
_cursor_off:
		.endif
cursor_off:
	    ld a,(_outputtty)
	    ld c,a
	    ld a,(_inputtty)
	    cp c
	    ret nz
	    ld a,(_int_disabled)
	    push af
	    di
	    ld de, (cursorpos)
	    ld a, (cursorpeek)
	    ld c, a
	    jp plotit

		.if VDP_DIRECT
_vtattr_notify:
_cursor_disable:
	    ret
		.endif

_vdp_set_console:
	    ld a,(_inputtty)
	    ld bc, (_vdpport)
	    dec a
	    out (c), a
	    ld a,#0x82
	    out (c),a
	    ret

;
;	Read write low level helpers (fastcall). Need IRQ off
;
;	These are designed to run with the user space mapped so we
;	directly burst rectangles to and from the VDP. We check for
;	writes into the 3C00-3FFF area reserved for the font cache. It's
;	perhaps a silly check on an unprotected Z80 but when we get to Z280
;	it might matter rather more!
;
;	This implementation won't work with thunked memory
;
		.if VDP_ROP

	    ; graphics API
	    .globl _vdp_rop
	    .globl _vdp_wop

	    .globl map_proc_always
	    .globl map_kernel

_vdp_rop:
	    push ix
	    push hl
	    pop ix
	    ld l, 2(ix)	; VDP offset
	    ld a, 3(ix)
	    cp #0x3C
	    ld h, a
	    jr nc, bounds	; Offset exceeds boundary
	    ld e, 6(ix)		; Stride
	    ld d, #0
	    ld bc, (_vdpport)
	    out (c), l
	    VDP_DELAY3
	    out (c), h		; Set starting pointer
	    exx
	    ld l, (ix)		; User pointer
	    ld h, 1(ix)
	    ld d, 5(ix)		; cols
	    ld e, 4(ix)		; lines
	    ld bc,(_vdpport);
	    inc c		; data port
	    call map_proc_always
ropl:
	    ld b,d
ropc:
	    ini
	    VDP_DELAY
	    djnz ropc
	    exx
	    add hl, de		; stride
	    ld a, #0x3B		; we don't care about a read overruning 3C
	    cp h		; just 3F
	    jr c, bounds
	    out (c), l		; next line
	    VDP_DELAY3
	    out (c), h
	    exx
	    dec e
	    jr nz, ropl
	    pop ix
	    ld hl, #0
	    jp map_kernel
bounds:
	    pop ix
	    ld hl, #-1
            jp map_kernel

_vdp_wop:
	    push ix
	    push hl
	    pop ix
	    ld l, 2(ix)		; VDP offset
	    ld a, 3(ix)
	    or #0x40		; Write operation
	    cp #0x7C
	    ld h, a
	    jr nc, bounds	; Offset exceeds boundary
	    ld bc, (_vdpport)
	    ld b, 5(ix)		; cols
	    ld e, 6(ix)		; Stride
	    ld d, #0
	    out (c),l
	    VDP_DELAY3
	    out (c),h		; Set starting pointer
	    exx
	    ld l,(ix)		; User pointer
	    ld h,1(ix)
	    ld d,5(ix)		; cols
	    ld e,4(ix)		; lines
	    ld bc,(_vdpport)	;
	    inc c		; data port
	    call map_proc_always
wopl:
	    ld b,d
wopc:
	    outi
	    VDP_DELAY
	    djnz wopc
	    exx
	    add hl,de		; stride
	    ld a,#0x7C
	    cp h
	    jr nc, boundclear
	    jr c, bounds	; going over boundary
	    ; We could cross the boundary during the line
	    ld a, l
	    add b
	    jr c, bounds
boundclear:
	    out (c), l		; next line
	    out (c), h
	    exx
	    dec e
	    jr nz, ropl
	    pop ix
	    ld hl,#0
	    jp map_kernel

		.endif

;
;	This must be in data or common not code
;	Note: do not put them in common space on a thunked platform or any
;	other where the common data is per instance
;
	    .area _VIDEO
cursorpos:  .dw 0
cursorpeek: .db 0

_scrolld_base:
	    .dw		0x03C0		; Start of bottom line
_scrolld_mov:
	    .dw		0xFFD8		; -40
_scrolld_s1:
	    .dw		0x4028		; 4000 turns on write 40 chars
_scrollu_w:
	    .dw		0x0028		; width
_scrollu_mov:
	    .dw		0x3FD8		; up 40 bytes in low 12, add 0x4000
					; carry 11->12

scrollconf40:
	    .dw 0x03C0, 0xFFD8, 0x4028, 0x0028, 0x3FD8
scrollconf32:
	    .dw 0x02E0, 0xFFE0, 0x4020, 0x0020, 0x3FE0
