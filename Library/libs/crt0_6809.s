		.module crt0

		.globl ___stdio_init_vars
		.globl _main
		.globl _exit
		.globl _environ
		.globl ___argv

		.area .header

start:		jmp start2
		.db 'F'
		.db 'Z'
		.db 'X'
		.db '1'

;
;	FIXME: we need to automate the load page setting
;
		.db 0x80			; page to load at
		.dw 0				; chmem ("0 - 'all'")
		.dw __sectionlen_.text__	; gives us code size info
		.dw __sectionlen_.data__	; gives us data size info
		.dw __sectionlen_.bss__		; bss size info
		.dw 0			; spare

		.area .text

start2:
		; we don't clear BSS since the kernel already did
		jsr ___stdio_init_vars

		; pass environ, argc and argv to main
		; pointers and data stuffed above stack by execve()
		leax 4,s
		stx _environ
		leax 2,s
		stx ___argv
		puls x			; argc
		ldy #_exit		; return vector
		pshs y
		jmp _main		; go

		.area .data

_environ:	.dw 0
