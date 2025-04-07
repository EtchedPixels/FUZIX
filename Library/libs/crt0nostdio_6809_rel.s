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
		.db 18				; entry relative to start
		.db 0				; no chmem hint
		.db 0				; no stack hint
		.db 0				; ZP not used on 6809

		.dw 0				; Patched for reloc ptr

		; We can be at any page aligned address but our base
		; is passed in Y

		tfr y,x				; Base into both
		ldd 16,x			; Relocation offset from
						; our header
		leay d,y			; To relocation base

		tfr x,d				; Base into D so we can get
						; the high byte for
						; relocating into A

		;
		; A is the relocation amount in pages
		; B is scratch
		; U is not used
		; X is the binary as we walk it relocating
		; Y is the relocation table pointer
		;
relocnext:
		ldb ,y				; Relocation byte
		clr ,y+				; Turn into BSS
		tstb				; 0 is end marker
		beq  relocdone
		cmpb #255			; 255 is a long skip
		beq reloc254
		abx				; 1-254 is that many
						; bytes on and relocate
		tfr a,b				; Shuffle to keep the
		addb ,x				; A value unchanged
		stb ,x				; Relocate the byte
		bra relocnext

reloc254:	; 255 means move on 254 but do not relocate
		decb				; We know B is 255
		abx
		bra relocnext

		;
		; Correct the brk base of the binary as we can now discard
		; the relocation table from memory if it grew the binary
		; size
		;

relocdone:
		; Fix up the BSS base
		; This will be relocated before it is run
		ldx #__sectionbase_.bss__+__sectionlen_.bss__
		ldd #30				; brk(x)
		swi				; and syscall
		;
		;  This jmp was relocated by the relocation loop above
		;
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
