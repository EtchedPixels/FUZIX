;
;	Boot blocks for FUZIX on the NC100
;
;
;	FIXME: we need a valid NMI vector in the low 0x100 at all times
;

;
;	These must match the actual nc100.s code
;
entry_sp	.equ 0x1BE
kernel_sp	.equ 0x1C0
resume_vector	.equ 0x1C2
suspend_map	.equ 0x1C4
entry_banks	.equ 0x1C8
suspend_stack	.equ 0x1FE

.area BOOTBLOCK (ABS)

; Stub to switch into the resume bank and code
.org 0xE0
	out (0x13),a
	jp (hl)

; On entry, the map looks like this:
; 	0x0000: OS workspace
;   0x4000: us (on the NC200 w/ floppy boot)
;   0x8000: OS common
;   0xc000: us (on either w/ PCMCIA boot)
; We expect to run in the 0x0000 page, so the first thing we need to do is
; remap ourselves.

.org 0x100
	di
	ld a, #0x80
	out (0x10), a				; map boot page to 0x0000
	jp relocated
relocated:
	; At this point the OS common page is in 0x8000-0xbfff.
	ld (entry_sp),sp
	ld sp, #suspend_stack
	ld hl, (0xB000)	; Save MMU bank copies from the OS
	ld (entry_banks),hl
	ld hl, (0xB002)
	ld (entry_banks+2),hl

	; Resume or new ?
	ld hl, (resume_vector)
	ld de, #0
	ld (resume_vector), de
	ld a, h
	or l
	jr z, no_resume

; To resume, we map the suspend page at 0xc000 and jump to the resume
; vector. As we're running from 0x0000, we can do that directly.

	ld a, (suspend_map + 3)
	out (0x13), a
	jp (hl)

no_resume:
	; We want to boot proper now. To do this, we need to copy the clean
	; copy of the kernel (in pages 0x80, 0x81, 0x82) into the working copy
	; (in 0x83, 0x84, 0x85).
	ld a, #0x80
	ld b, #0x83
	call copy_page
	ld a, #0x81
	ld b, #0x84
	call copy_page
	ld a, #0x82
	ld b, #0x85
	call copy_page

	; Now map the working copy.
	ld a, #0x83
	out (0x10), a ; THIS IS SAFE because pages 0x80 and 0x83 contain the same code
	inc a
	out (0x11), a
	inc a
	out (0x12), a
	inc a
	out (0x13), a ; Page 0x86 contains the common.

	; And go.
	jp 0x0223

	; Copies a single page. Source is in A, dest is in B.
	; Uses 0x4000 as the source and 0x8000 as the dest.
copy_page:
	out (0x11), a
	ld a, b
	out (0x12), a
	ld hl, #0x4000 ; source
	ld de, #0x8000 ; dest
	ld bc, #0x4000 ; size
	ldir
	ret

		.org 0x200
;
;	We should hide a logo in here ...
;
signature:	.ascii "NC100PRG"
padding2:	.db	0,0,0,0,0,3,0,1
		jp 0xC220		; start as seen from top bank
		.asciz	"FUZIX LOADER"

;
;	At this point we are mapped at 0xC000 so this code is running from
;	0xC220 in truth. Only at the jp to switch do we end up mapped low
;
; This is 0xC220 -> jump to next byte but in right page
		jp 0xC100		; start of our code
