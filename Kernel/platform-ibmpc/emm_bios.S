/*
 *	EMM bios crap for emulator stuff
 */


	.arch i8086,jumps
	.code16
	.att_syntax prefix

	.global emm_detect
	.global emm_page_frame_segment
	.global emm_free_pages
	.global emm_allocate
	.global emm_map_memory

	.data
emm_string:
	.byte "EMMXXXX0"
	.even

	.text
emm_detect:
	pushw %es
	pushw %di
	pushw %si
	xorw %ax,%ax
	movw %ax,%es	; set segment to 0
	movw 0x19E:%ax	; read the vector
	movw ax,%es	; segment is now the driver segment

	movw $10,%di
	movw $emm_string,%si
	movw $4,%cx	
	repe cmpsw
	
	cmpw $0,%cx
	je emm_ok
	movw %cx,%ax
emm_det_done:
	popw %si
	popw %di
	popw %es
	ret
emm_ok:
	movw $1,%cx
	jmp emm_det_done

emm_page_frame_segment:
	movb $0x41,%ah
	int $0x67
	movw %bx,%ax
	ret

emm_free_pages:
	movb %0x42,%ah
	int $0x67
	movw %bx,%ax
	ret

emm_allocate:
	pushw %bp
	movw %sp,%bp
	movw 4(%bp),%bx
	movb $0x43,%ah
	int $0x67
	cmpw $0,%ax
	jne failed_alloc
	movw %dx,%ax
failed_alloc:
	popw %bp
	ret

emm_map_memory:
	pushw %bp
	movw %sp,%bp
	movw 4(%bp),%al		; handle id
	movw 6(%bp),%dx		; handle
	movw 8(%bp),%bx		; page

	movb $0x43,%ah
	int $0x67
	xorb %al,%al
	popw %bp
	ret
