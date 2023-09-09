		.data

		.export __tmp
		.export __tmp2
		.export __tmp3
		.export __hireg
		.export __retaddr
		.export __ret
		.export __xchgret
		.export	__callhl

; __tmp must be the word before hireg
__tmp:
		.word	0
__hireg:
		.word	0
__tmp2:
		.word	0
		.word	0
__tmp3:
		.word	0
		.word	0
__xchgret:
		ex	de,hl
__ret:
		.byte	0xC3		; JMP
__retaddr:
		.word	0

		.code

__callhl:	jp	(hl)
