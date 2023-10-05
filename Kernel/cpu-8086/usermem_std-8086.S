#include "kernel-8086.def"

	.arch i8086,jumps
	.arch .8087
	.code16
	.att_syntax prefix

	.text

	.globl	_uget
	.globl	_ugetc
	.globl	_ugetw

	.globl	_uput
	.globl	_uputc
	.globl	_uputw

	.globl	_uzero

	.globl	_uget_ispace
	.globl	_uput_ispace

/*
 *	We assume a single segment model for now. p->page is CS p->page2
 *	is DS/ES/SS
 */

_ugetc:
	popw	%cx
	movw	udata_ptr, %bx
	movw	U_DATA__U_PAGE+2(%bx),%ax
	popw	%bx
	movw	%ax,%es
	movb	%es:(%bx),%al
	pushw	%bx
	jmp	*%cx

_ugetw:
	popw	%cx
	movw	udata_ptr, %bx
	movw	U_DATA__U_PAGE+2(%bx),%ax
	popw	%bx
	movw	%ax,%es
	movw	%es:(%bx),%ax
	movw	%ds,%ax
	movw	%ax,%es
	pushw	%bx
	jmp	*%cx

_uputc:
	pushw	%bp
	movw	%sp,%bp
	movw	udata_ptr,%bx
	movw	U_DATA__U_PAGE+2(%bx),%ax
	movw	%ax,%es
	movw	4(%bp),%bx
	movb	6(%bp),%al
	movb	%al,(%bx)
	movw	%ds,%ax
	movw	%ax,%es
	popw	%bp
	ret

_uputw:
	pushw	%bp
	movw	%sp,%bp
	movw	udata_ptr,%bx
	movw	U_DATA__U_PAGE+2(%bx),%ax
	movw	%ax,%es
	movw	4(%bp),%bx
	movw	6(%bp),%ax
	movw	%ax,(%bx)
	movw	%ds,%ax
	movw	%ax,%es
	popw	%bp
	ret

_uzero:
	pushw	%bp
	movw	%sp,%bp
	movw	udata_ptr,%bx
	movw	U_DATA__U_PAGE+2(%bx),%ax
	movw	%ax,%es
	pushw	%di
	movw	4(%bp),%di
	movw	6(%bp),%cx
	xorw	%ax,%ax
	cmpw	%ax,%cx
	je	nowork
	rep	stosb
nowork:
	movw	%ds,%ax
	movw	%ax,%es
	popw	%bp
	ret

_uget:
	pushw	%bp
	movw	%sp,%bp
	movw	udata_ptr,%bx
	movw	U_DATA__U_PAGE+2(%bx),%ax
_uget_1:
	movw	%ax,%ds
	pushw	%di
	pushw	%si
	movw	4(%bp),%si		/* User */
	movw	6(%bp),%di		/* Kernel */
	movw	8(%bp),%cx		/* Length */
	rep	movsb
	movw	%es,%ax
	movw	%ax,%ds
	popw	%si
	popw	%di
	popw	%bp
	ret

_uget_ispace:
	pushw	%bp
	movw	%sp,%bp
	movw	udata_ptr,%bx
	movw	U_DATA__U_PAGE(%bx),%ax
	jmp	_uget_1

_uput:
	pushw	%bp
	movw	%sp,%bp
	movw	udata_ptr,%bx
	movw	U_DATA__U_PAGE+2(%bx),%ax
_uput_1:
	movw	%ax,%es
	pushw	%di
	pushw	%si
	movw	4(%bp),%si		/* Kernel */
	movw	6(%bp),%di		/* User */
	movw	8(%bp),%cx		/* Length */
	rep	movsb
	movw	%ds,%ax
	movw	%ax,%es
	popw	%si
	popw	%di
	popw	%bp
	ret

_uput_ispace:
	pushw	%bp
	movw	%sp,%bp
	movw	udata_ptr,%bx
	movw	U_DATA__U_PAGE(%bx),%ax
	jmp	_uget_1
