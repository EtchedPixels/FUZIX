;
;		TOS = lval of object HL = amount
;
		.export __postinc
		.export __postincde
		.export __postince
		.export __postinc1
		.export __postinc2
		.export __postinc3
		.export __postinc4
		.code

__postinc:
		ex	de,hl		; amount into DE
		pop	hl		; return address
		ex	(sp),hl		; swap to get addr and return on stack
__postincde:
		ld	a,(hl)
		ld	(__tmp),a
		add	a,e
		ld	(hl),a
		inc	hl
		ld	a,(hl)
		ld	(__tmp+1),a
		adc	a,d
		ld	(hl),a
                ld	hl,(__tmp)
		ret

__postinc4:
		ld	de,4
		jr	__postincde
__postinc3:
		ld	de,3
		jr	__postincde
__postinc2:
		ld	de,2
		jr	__postincde
__postinc1:
		ld	e,1
__postince:
		ld	d,0
		jr	__postincde
