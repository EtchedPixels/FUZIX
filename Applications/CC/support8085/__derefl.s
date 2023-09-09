;
;		H holds the pointer
;
		.export __derefl
		.export __dereflsp
		.setcpu	8085
		.code

__dereflsp:
		dad	sp
__derefl:
		ldhi	2
		lhlx
		shld	__hireg
		dcx	d
		dcx	d
		lhlx
		ret
