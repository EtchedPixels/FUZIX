;
;		TOS = lval of object HL = amount
;
		.export __postdec
		.export __postdec1
		.export __postdec2
		.export __postdec3
		.export __postdec4
		.export __postdece
		.export __postdecde

; FIXME: these are less common so move the postdecn forms to another file ?

		.code

__postdec:
		ex	de,hl
		pop	hl
		ex	(sp),hl
__postdecde:
		ld	a,(hl)
		ld	(__tmp),a
		sub	e
		ld	(hl),a
		inc	hl
		ld	a,(hl)
		ld	(__tmp+1),a
		sbc	a,d
		ld	(hl),a
                ld	hl,(__tmp)
		ret

__postdec4:
		ld	de,4
		jr	__postdecde
__postdec3:
		ld	de,3
		jr	__postdecde
__postdec2:
		ld	de,2
		jr	__postdecde
__postdec1:
		ld	e,1
__postdece:
		ld	d,0
		jr	__postdecde
