;
;	We are loaded at 0xFC00 so relocate
;	Note we link with the post copy address so no absolutes in the
;	pre or relocation loop!
;
;	0000-7FFF are RAM 8000-FFFF ROM (except I/O)
;
;	We load an image from 0200-FDFF missing out F000-F1FF
;	and then jump to 0202 if the marker is right
;
	.code

	.byte 0x95
	.byte 0x99
start:
	; Map the full 64K to RAM
	li	r0,0x2021
	movb	r0,@0xFE78
	swpb	r0
	movb	r0,@0xFE79

	; Our map is now 32 33 34 35

	li	r0,0xFC00
	li	r1,0x0080
	li	r2,0x00C0
	; Move to 0x0080-0x01FF
reloc:
	mov	*r0+,*r1+
	dec	r2
	jne	@reloc
	b	@go

go:

	li	r1, launch
	bl	@outstring

	li	r13, 0x01FF	; Stack out of the way

	;
	;	Main loop
	;
	;	Registers
	;
	;	R0 = scratch
	;	R1 = sector number
	;	R2 = address
	;	R3 = scratch
	;	R8 = reserved for outchar and wait routines
	;	R13 = stack pointer

	li	r2, 0x0200
	li	r1, 0x0001	; 0 is the partition/boot block

	li	r0, 0xE000
	movb	r0, @0xFE16	; Make sure we are in LBA mode
dread:
	bl	@waitready
	li	r0,'.' * 256	; '.' byte
	bl	@outchar
	ci	r2, 0xFE00
	jeq	@load_done
	ci	r2, 0xF000
	jne	@noskip
	ai	r2, 0x0200	; Skip F000-F1FF (I/O space)
	li	r0,'-' * 256
	bl	@outchar
noskip:
	swpb	r1
	movb	r1, @0xFE13
	movb	r1, @0xFE80
	swpb	r1
	inc r1
	li	r0, 0x0100	; 0x01 byte
	movb	r0, @0xFE12	; num sectors (drives may clear this each I/O)
	bl	@waitready
	li	r0, 0x2000	; 0x20 byte
	movb	r0, @0xFE17	; READ command

	bl	@waitdrq

	li	r3, 256
bytes:
	movb	@0xFE10,r0
	swpb	r0
	movb	@0xFE10,r0
	swpb	r0
	mov	r0,*r2+
	dec	r3
	jne	@bytes
	jmp	@dread

load_done:
	mov	@0x0200,r0
	ci	r0,0x9995
	jne	@bad_load

	li	r1, running
	bl	@outstring
	b	@0x0202		; jump to byte following

bad_load:
	li	r1, badimg
	bl	@outstring
stop:
	jmp	@stop

waitready:
	li	r8, 0x4000
waitreadyl:
	movb	@0xFE17,r0
	coc	r8,r0
	jne	@waitreadyl
wait_drq_done:
	rt

waitdrq:
	li	r8, 0x0900
waitdrql:
	movb	@0xFE17,r0
	czc	r8,r0
	jeq	@waitdrql
	mov	r0,r8
	andi	r8,0x0100
	jeq	@wait_drq_done
	movb	@0xFE11,r0
	movb	r0,@0xFE80
	bl	@outcharhex
	jmp 	@bad_load

outstring:
	mov	r11,r3
outstringl:
	movb	*r1+,r0
	jeq	@outsd
	bl	@outchar
	jmp	@outstringl
outsd:	b	*r3

outcharhex:
	mov	r11,r3
	swpb	r0
	mov	r0,r5
	mov	r5,r1
	srl	r1,4
	bl	@outhexdigit
	mov	r5,r1
	bl	@outhexdigit
	b	*r3

outhexdigit:
	ai	r1,0x0F
	movb	@hextab(r1),r0
;
;	R0 character, R8 scratch private
;
outchar:
	movb	@0xFEC5,r8
	coc	@lit_2000,r8
	jne	@outchar
	movb	r0,@0xFEC0
	rt

lit_2000:
	.word	0x2000

badimg:
	.byte 13,10
	.ascii "Image not bootable."
running:
	.byte 13,10,0
launch:
	.ascii "Loading Fuzix "
	.byte 0

hextab:
	.ascii	"0123456789ABCDEF"
end:
