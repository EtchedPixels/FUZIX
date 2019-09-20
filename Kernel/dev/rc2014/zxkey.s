;
;	Low level driver for the ZXKey Interface. We scan at 60Hz or so
;	thus we do this in asm to avoid extra interrupt overhead
;
;

		.module zxkeyasm


		.area _CODE1

		.globl _zxkey_scan
		.globl _zxkey_init

		.globl _keyrepeat
		.globl _keyboard_grab
		.globl _keyboard
		.globl _shiftkeyboard
		.globl _keybits

		.globl _zxkey_queue_key


SYMCOL		.equ	7
SYMROW		.equ	3
CAPSCOL		.equ	5
CAPSROW		.equ	4
SYMBYTE		.equ	SYMROW
CAPSBYTE	.equ	CAPSROW

;
;	Must match vt.h
;
KRPT_FIRST	.equ	0
KRPT_CONT	.equ	1

;
;	These two must match input.h
;
KEYPRESS_SHIFT	.equ	2
KEYBIT_SHIFT	.equ	1
KEYPRESS_CTRL	.equ	4
KEYBIT_CTRL	.equ	2
;
;	On exit we return to C with HL indicating the key code and shift
;	type so that C can decide what to do. 0 is used to indicate
;	no work to be done
;

_zxkey_scan:
		; We scan EFFF F7FF FBFF FDFF FEFF
		; The ZX81 keyboard matrix doesn't use the other 3 extra
		; lines
		ld bc,#0xEFFF
		ld hl,#keybuf
		ld de,#0	; E = Row 0  D = no changes
		xor a
		ld (newkey),a	; No key found so far
		;
		; Scan each row looking for changes. An unchanged line is
		; uninteresting, the normal case and can be skipped for
		; speed
		;
scannext:
		in a,(c)
		cp (hl)
		jr z, nomods
		ld (hl),a
		inc d
nomods:		inc hl		; keymap
		rrc b
		; If the 0 bit has hit C then we are done
		jr c,scannext

		; Check if we have keys held down (repeat etc still matter)
		ld a,(keysdown)
		or d
		ld hl,#0
		; No change, no keys down, nothing to do - exit
		ret z

		;
		; Changes happened. Walk the data to see what occurred
		; and update the old map as we go so that next time
		; we see only new changes
		;
		ld a,#0xFF
		out (0xFD),a

		push ix

		ld ix,#_keymap
		ld hl,#keybuf
		ld b,#5
		ld e,#0
keyscan:
		ld a,(hl)	; key buf - what is down now
		ld c,a
		xor (ix)	; existing key state as the OS sees it
		; A is now the changes
		; Save the new state
		ld (ix),c
		call nz, zxkey_eval
		inc hl
		inc ix
		inc e
		djnz keyscan

		pop ix

		xor a
		out (0xFD),a

		;
		;	Final bits of work
		;

		ld hl,#0
		ld a,(keysdown)
		or a
		ret z
		cp #3
		ret nc

		;
		;	Valid key down status
		;
		ld a,(newkey)
		or a
		jr z, checkrpt
		ld a,(_keyrepeat + KRPT_FIRST)
		ld (kbd_timer),a
		push af
		call _keydecode
		pop af
		ret
		;
		;	Our keycode is still down check if it is
		;	repeating yet
		;
checkrpt:
		ld a,(kbd_timer)
		dec a
		ld (kbd_timer),a
		ret nz
		ld a,(_keyrepeat + KRPT_CONT)
		ld (kbd_timer),a
		push af
		call _keydecode
		pop af
		ret


		; Scan the bits and work out what changed for this
		; keyboard row
		;
		; (HL) = keymap entry
		; A = changed bits
		; E = row id, D free
		; B = used
		; C = keybuf value
		; 
zxkey_eval:
		ld d,#0
zxkey_loop:
		; All changes done ?
		or a
		ret z
		add a
		jr nc, zxkey_next
		push af
		; This bit changed
		; E = row, D = bit num
		call key_is_shift
		; Shift doesn't affect our counts and tables
		jr z, zxkey_pop_next

		bit 7,c		; key up or down ?
		jr z, zxkey_down
		ld a,(_keyboard_grab)
		cp #3
		jr nz, nonotify
		push bc
		push hl
		push de		; row and col
		push af		; banked
		call _zxkey_queue_key
		pop af
		pop de
		pop hl
		pop bc
nonotify:
		ld a,(keysdown)
		dec a
		ld (keysdown),a
		jr zxkey_pop_next
zxkey_down:
		ld a,#1
		ld (newkey),a
		ld (_keybits),de	; row and col
		ld a,(keysdown)
		inc a
		ld (keysdown),a
zxkey_pop_next:
		pop af
zxkey_next:
		inc d
		rlc c
		jr zxkey_loop



;
;	Turn key D,E into a keycode
;
keylookup_shift:
		ld hl,#_shiftkeyboard
		jr keylookup
keylookup_main:
		ld hl,#_keyboard
keylookup:
		ld a,e			; 0-4 so won't overflow
		add a
		add a
		add a
		add d
		ld e,a
		ld d,#0
		add hl,de
		ld a,(hl)
		ret


_keydecode:
		ld de,(_keybits)
		ld a,(keybuf + SYMBYTE)		; keymap for shift
		ld c,a
		ld a,(keybuf + CAPSBYTE)	; keymap for caps
		ld b,a
		bit SYMCOL,c
		jr nz, not_sym			; symbol shift not pressed
		bit CAPSCOL,b
		jr z, not_sym			; caps is pressed
		; Shift only - so look up the shifted symbol
		call keylookup_shift
		ld e,#KEYPRESS_SHIFT		; type shift
		jr checkctrl
		;
		; Check for caps shift
		;
not_sym:
		call keylookup_main
		ld e,#0
		bit CAPSCOL,b
		jr nz,keyqueue			; caps is up - no changes
		ld e,#KEYPRESS_SHIFT
		;
		; Capitalization and other caps shift oddities for the ZX
		; keyboard, notably caps-0 being backspace and to fit the
		; spectrum convention mapping caps-space as ^C
		;
		cp #'a'
		jr c, notlc
		cp #'z'+1
		jr nc,notlc
		sub #32
		jr keyqueue
notlc:		cp #'0'
		jr nz, notkeybs
		ld a,#8
		jr keyqueue
notkeybs:	cp #' '
		jr nz,notkeystop
		ld a,#3			; control-C
		jr keyqueue
notkeystop:	; TODO cursor keys
		cp #'1'
		jr c,not_switch
		cp #'5'
		jr nc,not_switch
		sub #'0'
		ld h,#0x40		; special code for console change
		ld l,a
		ret

not_switch:
		;
		; There is no control key, so we map caps/sym together as
		; control. This is less than ideal as it has to be a sticky
		; toggle because of the rollover. At least it resembles
		; the way the Spectrum works 8)
		;
checkctrl:
		bit SYMCOL,b
		jr nz, keyqueue
checkctrl2:
		bit CAPSCOL,c
		jr nz, keyqueue

		; Toggle the control flag, and absorb the key
		ld a,(ctrl)
		cpl
		ld (ctrl),a
		ld hl,#0
		ret

		;
		;	Return the shift info and key symbol to
		;	the calling C code
		;
keyqueue:
		ld h,e
		ld l,a
		; Finally check for the control toggle
		ld a,(ctrl)
		or a
		ret z
		; Control was pressed
		set KEYBIT_CTRL,e
		ld a,#0x1F
		and l
		ld l,a
		xor a
		ld (ctrl),a
		ret


;
;	Helper to check if a key is a shift key
;
;	On entry D,E are the symbols to check
;	Returns Z if a shift key
;
key_is_shift:
		ld a,#SYMROW
		cp e
		jr nz, notsymsh
		ld a,#7-SYMCOL
		cp d
		ret
notsymsh:
		ld a,#CAPSROW
		cp e
		ret nz
		ld a,#7-CAPSCOL
		cp d
		ret

		.area _DISCARD

_zxkey_init:
		; Set up both arrays
		ld hl,#keybuf
		ld a,#0xFF
		ld b,#10
l1:
		ld (hl),a
		inc hl
		djnz l1
		ret


		.area _CONST

;
;	5 x 8 key matrix. The eights go up and down the keyboard not
;	across it.
;
_keyboard:
		; Decode 0: 0xFEFF
		.ascii 'bhvyg6t5'
		; Decode 1: 0xFDFF
		.ascii 'njcuf7r4'
		; Decode 2: 0xFBFF
		.ascii 'mkxid8e3'
		; Decode 3: 0xF7FF
		.byte 0		; symbol shift
		.ascii 'lzos9w2'
		; Decode 4: 0xEFFF
		.ascii ' '
		.byte 10
		.byte 0		; caps shift
		.ascii 'pa0q1'

_shiftkeyboard:
		; Decode 0: 0xFEFF
		.ascii '*^/[}&>%'
		; Decode 1: 0xFDFF
		.ascii ",-?]{'<$"
		; Decode 2: 0xFBFF
		.ascii 'm+$i\(e#'	; FIXME pound not dollar
		; Decode 3: 0xF7FF
		.byte 0		; symbol shift
		.ascii '=:;[)w@'
		; Decode 4: 0xEFFF
		.byte ' ',10, 0	; space, enter, caps
		.ascii '"~_q!'

		.area _DATA
;
;	Working variables
;
keysdown:	.ds 1		; Keys currently pressed (rollover protection)
keybuf:		.ds 5		; Current matrix state
_keymap:	.ds 5		; Previous matrix state
newkey:		.ds 1		; Have we seen a new key ?
_keybits:	.ds 2		; If so what was its position ?
ctrl:		.ds 1		; Sticky control toggle
kbd_timer:	.ds 1		; Timer for repeat
