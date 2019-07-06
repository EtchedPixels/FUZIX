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

		.globl _main
		.globl _exit
		.globl _environ
		.globl ___argv

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
		; These three are set by binman
		.dw 0			; code
		.dw 0			; data
		.dw 0			; bss size
		.dw 0			; spare

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

		.area _GSINIT
gsinit:
		.area _GSFINAL
		ret

		.area _DATA
_environ:	.dw 0
