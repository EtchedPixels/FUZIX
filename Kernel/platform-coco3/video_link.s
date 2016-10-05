;;;
;;;  This file contains stubs to bank in the video code, call it,
;;;  and return.  This is not thread safe.
;;;

	.globl	_vtoutput
	.globl	_vt_ioctl
	.globl	_vt_inproc
	.globl	_vt_save
	.globl	_vt_load
	.globl  _gfx_draw_op
	.globl  _video_init

	.area	.text

ret	.dw	0


_vtoutput
	pshs	y
	lda	#11
	sta	$ffa9
	ldy	2,s
	sty	ret
	ldy	#return
	sty	2,s
	puls	y
	jmp	[$2000]

_vt_ioctl
	pshs	y
	lda	#11
	sta	$ffa9
	ldy	2,s
	sty	ret
	ldy	#return
	sty	2,s
	puls	y
	jmp	[$2002]

_vt_inproc
	pshs	y
	lda	#11
	sta	$ffa9
	ldy	2,s
	sty	ret
	ldy	#return
	sty	2,s
	puls	y
	jmp	[$2004]

_vt_save
	pshs	y
	lda	#11
	sta	$ffa9
	ldy	2,s
	sty	ret
	ldy	#return
	sty	2,s
	puls	y
	jmp	[$2008]

_vt_load
	pshs	y
	lda	#11
	sta	$ffa9
	ldy	2,s
	sty	ret
	ldy	#return
	sty	2,s
	puls	y
	jmp	[$200a]

_gfx_draw_op
	pshs	y
	lda	#11
	sta	$ffa9
	ldy	2,s
	sty	ret
	ldy	#return
	sty	2,s
	puls	y
	jmp	[$200c]

_video_init
	pshs	y
	lda	#11
	sta	$ffa9
	ldy	2,s
	sty	ret
	ldy	#return
	sty	2,s
	puls	y
	jmp	[$200e]

return
	lda	#1
	sta	$ffa9
	jmp	[ret]
