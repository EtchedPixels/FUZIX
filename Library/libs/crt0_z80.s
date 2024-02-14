		.code

.export __text		; We will need this for the syscall changes
			; to get at the stubs
__text:
		.word 0x80A8	; Fuzix executable
		.byte 1			; 8080 series
		.byte 0x02		; Z80 feature set
		.byte 0			; Base 0x0000
		.byte 0			; No hints

		.word __data		; code + literal (0 based)
		.word __data_size	; data
		.word __bss_size	; bss size

		.byte 18		; Run 18 bytes in
		.byte 0			; No size hint
		.byte 0			; No stack hint
		.byte 0			; No ZP on 8080

		.word __sighandler		; signal handler vector

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
		ld hl,__bss		; will be pre-reloc value (0 based)
		add hl,de		; hl is now the relocations
					; de is the code base
		ld b,#0			; on the code base bits
		ex de,hl		; de is relocations as loop swaps
relnext:
		; Read each relocation byte and zero it (because it's really
		; stolen BSS so should start zero)
		ex de,hl
		ld a,(hl)
		ld (hl),0
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
		add a,d
		exx
		ld (hl),a
		jr relnext
relocskip:	add hl,bc
		dec hl
		jr relnext
relocdone:
		ld hl,__bss		; Where our BSS should start
		ld de,__bss_size	; The size
		add hl,de
		push hl
		call _brk		; Break back to the right place
		pop af
		call ___stdio_init_vars
		ld hl,4
		add hl,sp
		ld (_environ),hl
		pop de			; argc
		pop hl			; argv
		push hl
		ld (___argv),hl		; needed for stuff like err()
		push de
		call _main		; go
		push hl
		call _exit


;
;	Signal handling glue. Called to indirect signals from the kernel
;	through code that saves the non-rentrant OS glue. Our stack
;	on entry has the return frame and we are passed C=signal number
;
__sighandler:
		; DE holds the vector, the stack is as we want it
		; but we have temporaries so must save them first
		ld	hl,(__tmp)
		push	hl
		ld	hl,(__hireg)
		push	hl
		ld	hl,(__tmp2)
		push	hl
		ld	hl,(__tmp2+2)
		push	hl
		ld	hl,(__tmp3)
		push	hl
		ld	hl,(__tmp3+2)
		push	hl

		ld	b,0
		push	bc		; signal number
		ex	de,hl
		call	__callhl
		pop	bc

		; Recover compiler temporaries

		pop	hl
		ld	hl,(__tmp3+2)
		pop	hl
		ld	hl,(__tmp3)
		pop	hl
		ld	hl,(__tmp2+2)
		pop	hl
		ld	hl,(__tmp2)
		pop	hl
		ld	hl,(__hireg)
		pop	hl
		ld	hl,(__tmp)
		ret			; to the previous frame via the

		.data
					; kernel helpers
.export _environ
_environ:	.word 0

