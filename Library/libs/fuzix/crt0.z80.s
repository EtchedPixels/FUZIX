		.module crt0

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

		.globl ___stdio_init_vars
		.globl _main
		.globl _exit
		.globl _environ
		.globl ___argv

		.globl s__DATA
		.globl l__DATA
		.globl s__INITIALIZED

		.area _CODE

; start at 0x100
start:		jp start2
		.db 'F'
		.db 'Z'
		.db 'X'
		.db '1'

;
;	Borrowed idea from UMZIX - put the info in known places then
;	we can write "size" tools
;
;	This is confusing. SDCC doesn't produce a BSS, instead it
;	produces an INITIALIZED (which everyone else calls DATA) and a
;	DATA which everyone else would think of as BSS.
;
;	FIXME: we need to automate the load page setting
;
		.db 0x01		; page to load at
		.dw 0			; chmem ("0 - 'all'")
		.dw s__INITIALIZED	; gives us code size info
		.dw s__DATA		; gives us data size info
		.dw l__DATA		; bss size info
		.dw 0			; spare

start2:		ld hl, #l__DATA - 1	 ; work around linker limit
		ld b, h
		ld c, l
		ld hl, #s__DATA
		ld de, #s__DATA+1
		ld (hl), #0
		ldir
		call gsinit

		ld hl, #4
		add hl, sp
		ld (_environ), hl
		pop de			; argc
		pop hl			; argv
		push hl
		ld (___argv), hl	; needed for stuff like err()
		push de
		ld hl, #_exit		; return vector
		push hl
		jp _main		; go

		.area _GSINIT
;
;	FIXME: we want to work this into the C code so it's not
; automatically sucking in any of stdio at all.
;
gsinit:
		call ___stdio_init_vars
;
;	Any gsinit code from other modules will accumulate between here
;	and _GSFINAL to provide constructors and other ghastly C++isms
;
		.area _GSFINAL
		ret

		.area _DATA
_environ:	.dw 0
