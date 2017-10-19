	.arch i8086,jumps
	.code16
	.att_syntax prefix

	.text

	.global xmm_probe
	.global xmm_transfer

/*
 *	BIOS probe for extended memory in Kb. This doesn't work on modern
 *	systems but it does on 286 era boxes just fine.
 */
xmm_probe:
	scf
	movb	%0x88,%ah
	int	$0x15
	jnc	xmm_ok
	xorw	%ax,%ax
xmm_ok:
	ret

/*
 *	Perform an extended memory transfer
 */
xmm_transfer:
	pushw	%bp
	movw	%sp,%bp
	movw	4(%bp),%cx		; count
	pushw	%si
	movw	$xmm_gdt,%si
	movb	$0x87,%ah
	scf
	int	$0x15
	jc	no_xmm_xfer
	popw	%si
	popw	%bp
	ret

	.data

xmm_gdt:
	.byte	0,0,0,0,0,0,0,0
	.byte	0,0,0,0,0,0,0,0
	.byte	0xFFFF,0,0,0,0,0x93,0,0
	.byte	0xFFFF,0,0,0,0,0x93,0,0
	.byte	0,0,0,0,0,0,0,0
	.byte	0,0,0,0,0,0,0,0
	.byte	0,0,0,0,0,0,0,0
	.byte	0,0,0,0,0,0,0,0
