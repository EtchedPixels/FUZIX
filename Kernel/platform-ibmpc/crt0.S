	.arch i8086,jumps
	.code16
	.att_syntax prefix

	.text

	.global kstack_top
	.global init_early
	.global init_hardware
	.global fuzix_main
	.global __bss_start
	.global __end

start:
	movw	$kstack_top,%sp		/* Set the stack */
	movw	$__bss_start,%si	/* Wipe the BSS */
	movw	$__bss_start+1,%di
	movw	$__end,%cx
	subw	$__bss_start+1,%cx
	xorb	%al,%al
	rep
	stosb

	call	init_early
	call	init_hardware
	call	fuzix_main
1:
	jmp	1b

