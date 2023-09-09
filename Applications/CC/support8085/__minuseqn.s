;
;	HL = lval
;
		.export __minuseq1
		.export __minuseq2
		.export __minuseq1d
		.export __minuseq2d

		.setcpu 8085
		.code

__minuseq1:
		xchg
__minuseq1d:
		lhlx
		dcx	h
		shlx
		ret

__minuseq2:
		xchg
__minuseq2d:
		lhlx
		dcx	h
		dcx	h
		shlx
		ret
