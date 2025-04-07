		.module crt0

		.globl ___stdio_init_vars
		.globl _main
		.globl _exit
		.globl _environ
		.globl ___argv

		.area .header

start:
		.dw 0x80A8
		.db 0x04			; 6809
		.db 0x00			; 6309 not needed
		.db __sectionbase_.header__/256	; page to load at
		.db 0				; no hints
		.dw __sectionbase_.data__-__sectionbase_.header__ ; gives us header + all text segments
		.dw __sectionlen_.data__	; gives us data size info
		.dw __sectionlen_.bss__		; bss size info
		.db 16				; entry relative to start
		.db 0				; no chmem hint
		.db 0				; no stack hint
		.db 0				; ZP not used on 6809

		jmp start2

		.area .text

start2:
		; we don't clear BSS since the kernel already did

		; pass environ, argc and argv to main
		; pointers and data stuffed above stack by execve()
		leax 4,s
		stx _environ
		ldx 2,s
		stx ___argv
		puls x			; argc
		ldy #_exit		; return vector
		pshs y
		jmp _main		; go

		.area .data

_environ:	.dw 0
