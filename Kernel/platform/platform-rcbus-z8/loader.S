;
;	The bootblock is loaded at address 0 and runs
;	with interrupts disabled from 12 so there is room
;	to load vectors. RAM is fulled mapped (MMU is 0x11). If split I/D
;	support is present the alt MMU is also 0x11
;
;	Stack is at FF00, SRP is 0x10
;

		.code

	.word	'Z8'		; bootable marker
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0

start:
	di
	ld	r11,#2		; disk block number count
	ld	r12,#0xFF
	ld	r13,#0x10	; used for I/O
	ld	r14,#2
	clr	r15		; destination address for reads

	and	2,#0xBF		; enable I/O


	; I/O is on so access memory via C space (C and E space are the
	; same at this point)

next:
	ld	r13,#0x17
	lde	r0,@rr12
	and	r0,#0x80
	jr	nz, next	; wait for busy to clear

	ld	r13,#0x12
	ld	r0,#1
	lde	@rr12,r0	; 1 sector
	inc	r13
	lde	@rr12,r11	; LBA.0
	clr	r0
	inc	r13
	lde	@rr12,r0	; LBA.8
	inc	r13
	lde	@rr12,r0	; LBA.16.
	inc	r13
	ld	r0,#0xE0	; LBA.24 | Device
	lde	@rr12,r0	; LBA high
	inc	r13		; Command/Status
	ld	r0,#0x20	; READ
	lde	@rr12,r0

wait_drq:			; Wait for drive to provide data
	lde	r0,@rr12
	and	r0,#8
	jr	z, wait_drq

	clr	r0		; count
	ld	r13,#0x10	; data
dblock:
	lde	r1,@rr12	; read CF card
	ldc	@rr14,r1
	incw	rr14
	lde	r1,@rr12	; read CF card
	ldc	@rr14,r1
	incw	rr14
	djnz	r0,dblock

	; Move on
	inc	r11		; next disk block
	cp	r14,#0		; top of memory ?
	jr	nz,next

	; All loaded
	or	2,#0x10		; I/O mode off
	jp	0x0202		; Launch
	; TODO check 0200 is 58 38
