;
;	HL = lval
;
		.export __pluseq1
		.export __pluseq2
		.export __pluseq1d
		.export __pluseq2d

		.setcpu 8085
		.code

__pluseq1:
		xchg
__pluseq1d:
		lhlx
		inx	h
		shlx
		ret

__pluseq2:
		xchg
__pluseq2d:
		lhlx
		inx	h
		inx	h
		shlx
		ret
