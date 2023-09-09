		.export __cmpneb
		.code

;
;	Tighter version with the other value in DE
;
__cmpneb:
		ld	a,l
		cp	e
		jp	nz,__true
		jp	__false
