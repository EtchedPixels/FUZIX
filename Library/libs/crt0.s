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
		call ___stdio_init_vars
;
;	Any gsinit code from other modules will accumulate between here
;	and _GSFINAL to provide constructors and other ghastly C++isms
;
		.area _GSFINAL
		ret

		.area _DATA
_environ:	.dw 0
