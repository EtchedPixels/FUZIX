;
;	Low level glue for the HFDC
;

		.export _hfdc_read_switches
		.export	_hfdc_copy_to_device
		.export _hfdc_copy_from_device
		.export _hfdc_execute_simple
		.export _hfdc_execute

		.common

_hfdc_read_switches:
	mov	r12,r2
	li	r12,0x1100	; TODO support multiple controllers
	sbo	0		; visible
	sbo	4		; switches
	ldcr	r1,8
	sbz	4
	sbz	0		; vanish card
	mov	r2,r12
	rt

map_hfdc:
	mov 	r12,r2
	li	r12,0x1100
	sbo	0
	mov	r2,r12
	;	Maps 0x4000-0x5FFF to the I/O space matching it.
	b	@map_iowindow

unmap_hfdc:
	mov 	r12,r2
	li	r12,0x1100
	sbo	0
	mov	r2,r12
	b	@map_kernel	

;
;	r4 = pointer 5 = size, *r13 = userflag
;
;	TODO: mapping setup for user mode copies. For now we are set up
;	never to use user memory copies. When we add this it's a bit tricky
;	because we need to map the user pages somewhere (6000-9FFF ?) then
;	do the copy to the memory we mapped in from the I/O space at
;	4000-5FFF. Swap will require similar if not identical work
;
_hfdc_copy_to_device:
	mov	r11, r5
	bl	@map_hfdc
	li	r0,0x5000
hfdc_to_l:
	mov	*r1+,*r0+
	dect	r2
	jne	hfdc_to_l
	bl	@unmap_hfdc
	bl	*r5

hfdc_copy_from_device
	mov	r11,r5
	bl	@map_hfdc
	li	r0,0x5000
hfdc_from_l:
	mov	*r0+,*r1+
	dect	r2
	jne	hfdc_to_l
	bl	@unmap_hfdc
	bl	*r5

_hfdc_execute_simple:
	dect	r13
	mov	r11,*r13
	mov	r12,r2

	li	r12,0x1100
	bl	@map_hfdc

cmd:
	mov	r4,@0x4FD6

	clr	r1
	inc	@_hfdc_busy
	ci	r5,0
	jeq	no_wait
	; Need to think about timeouts
wait_done:
	movb	@0x4FD4,r0
	mov	r0,r1
	andi	r0,0x20
	jeq	wait_done
	andi	r1,0x0c		; set bits means an error
	clr	@_hfdc_busy
no_wait:
	bl	@unmap_hfdc
	mov	r2,r12
	mov	*r13+,r11
	rt

_hfdc_execute:
	dect	r13
	mov	r11,*r13
	mov	r12,r2
	li	r12,0x1100
	bl	@map_hfdc
	li	r0,0x4000
	movb	r0,@0x4FD6
	; ?? do we need a wait on this command
	li	r0,10
hfdc_execute_n:
	li	r1,_hfdc_block
hfdc_load:
	movb	*r1+,@0x4FD2
	dec	r0
	jne	@hfdc_load
	jmp	@cmd

_hfdc_read_error:
	dect	r13
	mov	r11, *r13
	mov	r12,r2
	li	r12,0x1100
	bl	@map_hfdc
	li	r0,0x4700
	movb	r0,@0x4FD6
	; ?? do we need a wait on this command
	movb	@0x4FD0,*r4+
	movb	@0x4FD0,*r4+
	movb	@0x4FD0,*r4
	jmp	@no_wait

_hfdc_do_sync:
	dect	r13
	mov	r11,*r13
	mov	r12,r2
	li	r12,@0x1100
	bl	@map_hfdc
sync_wait:
	mov	@0x4FD0,r1
	mov	r1,r0
	andi	r0,0x20
	jeq	@sync_wait
	clr	@_hfdc_busy
	jmp	@no_wait

;
;	So we can be sure of accessing it with the controller mapped
;
_hfdc_busy
	.word	0
_hfdc_block
	.ds	12
