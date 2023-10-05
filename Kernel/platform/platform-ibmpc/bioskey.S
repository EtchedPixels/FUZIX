/*
 *	BIOS keyboard driver for annoying systems
 *
 *	We are hooked into the int handler for the keyboard and then abuse
 *	that as a way to tell that we are safe from re-entry because we only
 *	do keyboard in the keyboard handler
 *
 *	In addition we include an int 4f handler so that on most machines we
 *	can screen out annoyances like pause and printscreen.
 */

/* Called every keypress. We run the BIOS handler and see if a char is
   present. If not get out. This is the fast path */
bioskbd_handler:
	lcall	%cs:*bioskbd_old
	cli
	pushw	%ax
	movb	$01,%ah
	int	$0x16
	jnz	nochar
	xorw	%ax,%ax
	int	$0x16
	cmpb	$0,%al
	jnz	has_char
nochar:
	popw	%ax
	reti

/* The heavyweight path - dropping into C space */
has_char:
	pushw	%ds
	pushw	%es
	pushw	%bp
	pushw	%bx
	pushw	%cx
	pushw	%dx
	movw	kernel_ds,%ds
	movw	%ss, kbd_ss
	movw	%sp, kbd_sp
	cld
	movw	%ds,%bx
	movw	%bx,%es
	movw	%bx,%ss
	movw	$kbstack,%sp
	pushw	%ax
	movw	$1,%ax
	pushw	%ax
	call	tty_inproc
	popw	%ax
	popw	%ax
	movw	kbd_ss,%ss
	movw	kbd_sp,%sp
	popw	%dx
	popw	%cx
	pop	%bx
	popw	%bp
	popw	%es
	popw	%ds
	popw	%ax
	reti

/*
 *	We screen out printscreen and pause funnies. Some ancient systems
 *	don't have this hook so pause will cause the system to hang until
 *	it is done, print screen will call int 5 (we fix it up there) and
 *	ctrl-alt-del will do the usual
 */

int4f_hook:
	pushw %ds
	pushw %bx
	movw %cs,%bx
	movw %bx,%ds
	movb %al,%ah
	andb $0x7F,%ah
	cmpb $0x1D,%ah
	je cntrl
	cmpb $0x2A,%ah
	je lshift
	cmpb $0x36,%ah
	je rshift
	cmpb $0x38,%ah
	je alt
	cmpb $37,%al		/* prtscrn */
	je magicop1
	cmpb $45,%al
	je magicop2
	cmp $46,%al
	je magicop2
	cmp $0x53,%al
	je magicop3
passthrough:
	popf
	sec
	ret
/*
 *	Prevent the print screen callback and also the pause behaviour
 *	on the pause key.
 */
magicop1:	/* ignore with shift | ctrl */
	movw	shift_1,%bx
	cmpw	$0,%bx
	je	passthrough	
magicop2:	/* igmore with ctrl */
	movb	cntrl_1,%bl
	cmpb	$0,%bl
	je	passthrough
	clc
	ret
magicop3:	/* ignore with ctrl alt */
	movw	alt_1,%bx
	cmpw	$0,%bx
	je	passthrough
	clc
	ret
lshift:
	pushw %ax
	andb $0x80,%al
	movb %al,shift_1
	popw %ax
	sec
	ret
rshift:
	push %ax
	andb $0x80,%al
	movb %al,shift_2
	popw %ax
	sec
	ret
cntrl:
	push %ax
	andb $0x80,%al
	movb %al,cntrl_1
	popw %ax
	sec
	ret
alt:
	push %ax
	andb $0x80,%al
	movb %al,alt_1
	popw %ax
	sec
	ret

/* In CS: for simplicity */
alt_1:
	.byte 0
cntrl_1:
	.byte 0
shift_1:
	.byte 0
shift_2:
	.byte 0

	
bioskbd_old:
	.dword	0

	.data
kbd_ss	.word	0
kbd_sp	.word	0
	.bss	128
kbstack:
