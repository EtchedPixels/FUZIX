	.arch i8086,jumps
	.code16
	.att_syntax prefix

	.text

/*
 *	We are loaded somewhere high by the bootstrap and need
 *	to move the payload down, set up and enter it. The payload never
 *	returns to us. We are guaranteed not tbe in the payload's way
 *
 *	On entry
 *	cs/ds are set so our address is cs:0 etc
 *	es is undefined
 *	ss:sp is a stack somewhere potentially to be overwritten
 *	We are in the BIOS environment
 *	Interrupts are *off*
 *
 *	Tasks:
 *	Move the stack to 0x500-0x5FF
 *	Wipe 0x600-0x7FF
 *	Copy the data and code starting at 0x800 as two blocks (we skip
 *	the BSS) then jump to the code as cs:0
 */

bootstrap:
	movb	$'B',%al
	call	printchar
	movw	$0x50,%ax	/* 0x500 is base, 0x600->0x7FF is the
				   udata and stack we need to clear */
	movw	%ax,%es		/* Point ES: at the target */

	cld

	xorw	%ax,%ax
	movw	$0x100,%di
	movw	$0x100,%cx
	rep	stosw		/* Wipe 0x600-0x7FF */
	movb	$'o',%al
	call	printchar
	movw	$0x50,%ax
	movw	%ax,%ss		/* Switch stack segment */
	movw	$0x100,%sp	/* Stack pointer */
	movb	$'o',%al
	call	printchar
	sti			/* Interrupts are now safe */
	movb	$'t',%al
	call	printchar
	movw	$0x300,%di	/* es: Offset to start coping image */
	movw	$end,%si	/* ds: Offset of image */
	movw	end,%cx		/* Length to copy (words, 16 byte pad) */
	rep	movsw		/* Copy all but BSS */
	movb	$'i',%al
	call	printchar
	movw	end+2,%cx	/* Code length (words) */
	movw	end+4,%di	/* Code (and discard) base */
	rep	movsw		/* follows on in block so si is ok */
	movb	$'n',%al
	call	printchar
	movw	end+6,%bx	/* Segment value for code */
	movw	$0x50,%ax
	movw	%ax,%ds
	pushw	%bx		/* Segment of code */
	xorw	%ax,%ax
	pushw	%ax
	movb	'g',%al
	call	printchar
	retf			/* Jump to new CS:0 */
printchar:
	pushw	%ax
	pushw	%bx
	movb	$0x0E,%ah
	movb	$7,%bl
	int	$10
	popw	%bx
	popw	%ax
	ret
