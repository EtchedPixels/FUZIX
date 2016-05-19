;;;
;;;  Low-Level Video Routines
;;;
;;;
;;; [NAC HACK 2016Apr23] this also has memset and memcpy .. which should really be elsewhere
;;; because I don't need the other stuff
	.module	videoll


	;; exported
	.globl	_memset
	.globl	_memcpy

	include "kernel.def"
	include "../kernel09.def"

	.area .video

;;;   void *memset(void *d, int c, size_t sz)
_memset:
	pshs	x,y
	ldb	7,s
	ldy	8,s
a@	stb	,x+
	leay	-1,y
	bne	a@
	puls	x,y,pc



;;;   void *memcpy(void *d, const void *s, size_t sz)
_memcpy:
	pshs	x,y,u
	ldu	8,s
	ldy	10,s
a@	ldb	,u+
	stb	,x+
	leay	-1,y
	bne	a@
	puls	x,y,u,pc
