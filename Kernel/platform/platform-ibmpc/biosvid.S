/*
 *	Stubs for BIOS video
 *	We need to sort out the logic for blink/underline/attributes
 *
 *	For now we present a simple black and white console although in
 *	many cases we can actually do 8 colours plus bright plus under
 */

	.arch i8086,jumps
	.code16
	.att_syntax prefix
	.text

	.global bios_videomode
	.global bios_setpage
	.global	bios_videopage
	.global bios_setcursor
	.global bios_setchar
	.global bios_scrollup
	.global bios_scrolldown
	.global bios_clearscreen


/* Return mode and columns */
bios_videomode:
	movb $0x0F,%ah
	int $0x10
	andb $0x7F,%al
	movb %ah,dwidth
	ret
/* Set our display page */
bios_setpage:
	pushw %bp
	movw %sp,%bp
	movb 4(%bp),%al
	movb $5,%ah
	int $0x10
	pop %bp
	/* and drop through to see if it worked */
/* Display page */
bios_videopage:
	movb $0x0F,%ah
	int $0x10
	movb %bh,%al
	xorb %ah,%ah
	ret
bios_setcursor:
	pushw %bp
	movw %sp,%bp
	movw 4(%bp),%dx
	movb 6(%bp),%bh
	int $0x10
	popw %bp
	ret
bios_setchar:
	pushw %bp
	movw %sp,%bp
	movw 4(%bp),%ax
	movw 6(%bp),%bx
	movw 8(%bp),%cx
	movb $0x09,%ah
	int $0x10
	popw %bp
	ret
bios_scrollup:
	pushw %ds	/* Trident bug */
	pushw %bp
	movw %sp,%bp
	xorw %cx,%cx
	xorb %bh,%bh	/* Until we add colour/attributes */
	movb $24,%dh	/* Hack for the moment */
	movb dwidth,%dl
	decb %dl
	movw $0x0601,%ax
	int $0x10
	popw %bp
	popw %ds
	ret
bios_scrolldown:
	pushw %ds	/* Trident bug */
	pushw %bp
	movw %sp,%bp
	xorw %cx,%cx
	xorb %bh,%bh	/* Until we add colour/attributes */
	movb $24,%dh	/* Hack for the moment */
	movb dwidth,%dl
	decb %dl
	movw $0x0701,%ax
	int $0x10
	popw %bp
	popw %ds
	ret
bios_clearscreen:
	pushw %ds	/* Trident bug */
	pushw %bp
	movw %sp,%bp
	xorw %cx,%cx
	xorb %bh,%bh	/* Until we add colour/attributes */
	movb $24,%dh	/* Hack for the moment */
	xorb %al,%al
	movb dwidth,%dl
	decb %dl
	movw $0x0600,%ax
	int $0x10
	popw %bp
	popw %ds
	ret

	.data

dwidth:	.byte 0
