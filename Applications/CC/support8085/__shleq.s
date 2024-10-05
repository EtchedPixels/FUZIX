;
;	TOS is the lval, hl is the shift amount
;
;
		.export __shleq
		.setcpu 8085
		.code

__shleq:
	mov	a,l
	pop	h		; return
	xthl			; HL is now the lval
	xchg			; DE is now the lval
	lhlx			; HL is now the bits
	ani	15
	rz
	cpi	8
	jc	notquick
	mov	h,l
	mvi	l,0
	sbi	8
	jz	done
notquick:
	dad	h
	dcr	a
	jnz	notquick
done:	shlx
	ret
