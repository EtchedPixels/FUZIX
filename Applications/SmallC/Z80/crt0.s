		.code

; start at 0x100
start:		jp start2
		.byte 'F'
		.byte 'Z'
		.byte 'X'
		.byte '1'

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
		.byte 0x01		; page to load at
		.word 0			; chmem ("0 - 'all'")
		.word __code_size		; gives us code size info
		.word __data_size		; gives us data size info
		.word __bss_size		; bss size info
		.word 0			; spare

start2:
		ld hl, 4
		add hl, sp
		ld (_environ), hl
		pop de			; argc
		pop hl			; argv
		push hl
		ld (___argv), hl	; needed for stuff like err()
		push de
		ld hl, _exit		; return vector
		push hl
		jp _main		; go

		.data

_environ:	.word 0
___argv:	.word 0
