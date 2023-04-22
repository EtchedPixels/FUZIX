		.module crt0nostdio_z80_rel

		.area _CODE
		.area _HOME
		.area _CONST
		; The _GS can be blown away after startup. We don't yet
		; but we should do FIXME
		.area _GSINIT
		.area _GSFINAL
		.area _INITIALIZED
		.area _BSEG
		.area _DATA
		.area _BSS
		; note that the binary builder moves the initialized data
		; from initializer
		.area _INITIALIZER

		.globl _main
		.globl _exit
		.globl _environ
		.globl ___argv
		.globl s__DATA
		.globl l__DATA
		.globl _brk
		.globl __call_sys

		.area _CODE

__call_sys:				; Because the stubs overlay this
					; executable header
start:
		.dw 0x80A8		; Magic number
		.db 0x01		; 8080 family
		.db 0x02		; Z80 featureset required
		.db 0x01		; Load at 0x0100

		.db 0x00		; No hints
		.dw 0x0000		; Text size (updated by tools)
		.dw 0x0000		; Data size (updated by tools)
		.dw 0x0000		; BSS size (updated by tools)
		.db 18			; Start address
		.db 0			; Default hint for grab all space
		.db 0			; Default no stack hint
		.db 0			; No zero page on Z80

		.dw __sighandler	; Signal handling vector

		; 18 bytes in ...

start2:
;
;	On entry we are page aligned and de is our base
;
;	s__DATA is the BSS base computed by the compiler. It will get
;	modified by the relocatable binary maker in the header but not
;	in the code. Thus this is actually a pointer to our relocation
;	bytestream.
;
		push de			; base for relocation in DE
		exx			; get it into DE and DE'
		pop de			;
		ld hl,#s__DATA		; will be pre-reloc value (0 based)
		add hl,de		; hl is now the relocations
					; de is the code base
		ld b,#0			; on the code base bits
		ex de,hl		; de is relocatios as loop swaps
relnext:
		; Read each relocation byte and zero it (because it's really
		; stolen BSS so should start zero)
		ex de,hl
		ld a,(hl)
		ld (hl),#0
		inc hl
		ex de,hl
		; 0 means done, 255 means skip 254, 254 or less means
		; skip that many and relocate  (runs are 255,255,....)
		or a		; 0 ?
		jr z, relocdone
		ld c,a
		inc a		; 255 ?
		jr z, relocskip
		add hl,bc
		ld a,(hl)
		exx
		add d
		exx
		ld (hl),a
		jr relnext
relocskip:	add hl,bc
		dec hl
		jr relnext
relocdone:
		ld hl,#s__DATA		; Where our BSS should start
		ld de,#l__DATA		; The size
		add hl,de
		push hl
		call _brk		; Break back to the right place
		pop af
		; At this point our calls are relocated so this will go to
		; the right place
		call gsinit

		ld hl, #4
		add hl, sp
		ld (_environ), hl
		pop de			; argc
		pop hl			; argv
		push hl
		ld (___argv), hl	; needed for stuff like err()
		push de
		call _main		; go
		push hl
		call _exit

;
;	Simple for Z80 case (the 8080 binaries have a lot more to do)
;
__sighandler:
		ex de,hl
		jp (hl)


		.area _GSINIT
;
;	FIXME: we want to work this into the C code so it's not
; automatically sucking in any of stdio at all.
;
gsinit:
;
;	Any gsinit code from other modules will accumulate between here
;	and _GSFINAL to provide constructors and other ghastly C++isms
;
		.area _GSFINAL
		ret

		.area _DATA
_environ:	.dw 0
