		.export __postdec1
		.export __postdec2
		.export __postdec1d
		.export __postdec2d

		.setcpu 8085
		.code

__postdec1:
		xchg
__postdec1d:
		lhlx
		dcx	h
		shlx
		inx	h
		ret
__postdec2:
		xchg
__postdec2d:
		lhlx
		dcx	h
		dcx	h
		shlx
		inx	h
		inx	h
		ret


