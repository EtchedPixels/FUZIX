;
;		hireg:HL += TOS
;
		.export __plusl
		.code

__plusl:
		ex	de,hl
		pop	hl
		ld	(__retaddr),hl
		pop	hl
		add	hl,de		; HL is now low part
		pop	de		; High part
		push	hl		; Save low part
		ld	hl,(__hireg)	; Do high part
		jr	nc,nocarry	; We need to carry from the first dad
		inc	hl		; Carry
nocarry:
		add	hl,de
		ld	(__hireg),hl	; Save high part
		pop	hl		; Recover result
		jp	__ret		; Out
