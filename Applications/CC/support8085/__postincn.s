		.export __postinc1
		.export __postinc2
		.export __postinc1d
		.export __postinc2d

		.setcpu 8085
		.code

__postinc1:
		xchg
__postinc1d:
		lhlx
		inx	h
		shlx
		dcx	h
		ret
__postinc2:
		xchg
__postinc2d:
		lhlx
		inx	h
		inx	h
		shlx
		dcx	h
		dcx	h
		ret


