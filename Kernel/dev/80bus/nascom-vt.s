# 0 "../../dev/80bus/nascom-vt.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "../../dev/80bus/nascom-vt.S"
;
; The Nascom has a fairly simple default video console except
; that each line has margins of space that are used for other
; stuff
;

  .export _cursor_off
  .export _cursor_disable
  .export _cursor_on
  .export _plot_char
  .export _clear_lines
  .export _clear_across
  .export _vtattr_notify
  .export _scroll_up
  .export _scroll_down

# 1 "../../dev/80bus/../../build/kernelu.def" 1
; FUZIX mnemonics for memory addresses etc
# 13 "../../dev/80bus/../../build/kernelu.def"
;
; SPI uses the top bit
;
# 18 "../../dev/80bus/nascom-vt.S" 2

  .code
;
; Required to preserve BC
;
addr:
 ld a,e ; get Y
 dec a ; weird line wrapping
 and #15
 add a,a
 add a,a
 add a,a
 add a,a ; x16: as far as we can 8bit add
 ld l,a
 ld h,0
 ld e,d
 ld d,h ; DE is now 00xx where xx is the X value
 add hl,hl ; x 32
 add hl,hl ; x 64
 add hl,de
 ld de,0xF800 +10
 add hl,de
 ret

_cursor_off:
 ld hl,(cpos)
 bit 7,h ; all valid cpos values are > 0x8000
 ret z
 ld a,(csave)
 ld (hl),a
 xor a
 ld (cpos+1),a
_cursor_disable:
 ret

;
; TOS is Y X
;
_cursor_on:
 ld hl,#4
 add hl,sp
 ld d,(hl)
 dec hl
 dec hl
 ld e,(hl)
 call addr
 ld a,(hl)
 ld (hl),'_'
 ld (csave),a
 ld (cpos),hl
 ret

;
; TOS is Y X C
;
_plot_char:
 ld hl,#6
 add hl,sp
 ld a,(hl)
 dec hl
 dec hl
 ld d,(hl)
 dec hl
 dec hl
 ld e,(hl)
 ; Y in E X in D char in A
 push af
 call addr
 pop af
 ld (hl),a
 ret

;
; The weird layout means we can't do a single ldir
;
; TOS is Y count
;
_clear_lines:
 push bc ; save register variable
 ld hl,#6
 add hl,sp
 ld c,(hl) ; count
 dec hl
 dec hl
 ld e,(hl) ; Y
 ld d,0 ; X
 call addr
 ld a,c ; A is now line count
 or a
 jr z, noclear
wipeline:
 ld b,48
wiper: ld (hl),' '
 inc hl
 djnz wiper
 ld bc,16
 add hl,bc
 dec a
 jr nz, wipeline
noclear:
 pop bc
 ret

; TOS is Y X count
_clear_across:
 push bc
 ld hl,#8
 add hl,sp
 ld b,(hl)
 dec hl
 dec hl
 ld d,(hl)
 dec hl
 dec hl
 ld e,(hl)
 ; Y in E X in D count in B
 ld a,b
 or a
 jr z, noclear
 call addr
 ld a,' '
clear2: ld (hl),a
 inc hl
 djnz clear2
 pop bc
_vtattr_notify:
 ret

_scroll_up:
 push bc
 ; Do the special case first
 ld hl,0xF80A
 ld de,0xFBCA
 ld bc,48
 ldir
 ld de,0xF80A
 ld hl,0xF84A
 ; BC is always 0 here
 ld a,15
nextup:
 ld c,48
 ldir
 ld c,16
 add hl,bc
 ex de,hl
 add hl,bc
 ex de,hl
 dec a
 jr nz,nextup
 pop bc
 ret

_scroll_down:
 push bc
 ld hl,0xFB79
 ld de,0xFBB9
 ld a,15
nextdown:
 ld bc,48
 lddr
 ld bc,0xFFF0 ; - 16
 add hl,bc
 ex de,hl
 add hl,bc
 ex de,hl
 dec a
 jr nz, nextdown
 ; Top line special case
 ld hl,0xFBC0
 ld de,0xF800
 ld bc,48
 ldir
 pop bc
 ret

 .data

csave:
 .byte 0
cpos:
 .word 0
