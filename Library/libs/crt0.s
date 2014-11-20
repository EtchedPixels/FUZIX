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

		.globl s__DATA
		.globl s__BSS
		.globl l__DATA
		.globl l__BSS

		.area _CODE

		.ds 0x100
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
		.dw 0			; chmem ("0 - 'all'")
		.dw s__DATA		; gives us code size info
		.dw s__BSS		; gives us data size info
		.dw l__BSS		; bss size info
		.dw 0			; spare

start2:		ld hl, #l__DATA - 1	 ; work around linker limit
		ld de, #l__BSS
		add hl, de
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
		ld hl, #_exit		; return vector
		push hl
		jp _main		; go

		.area _GSINIT
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
