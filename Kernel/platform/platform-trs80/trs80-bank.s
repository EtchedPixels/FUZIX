;	   TRS 80 banking logic for the base Model 4 and 4P
;
;	Handles
;	- 128K base machine
;	- Hypermem (to test)
;	- Huffman style expander
;
;	The MegaMem is a 16K window so could only be used as a ramdisk
;	if added.
;
;	TODO; fix emulation of hypermem (should be >> 16 +1 not 15 + 2)
;

        .module trs80bank

        ; exported symbols
        .globl init_hardware
	.globl map_kernel
	.globl map_kernel_di
	.globl map_kernel_restore
	.globl map_proc
	.globl map_proc_di
	.globl map_proc_a
	.globl map_proc_always
	.globl map_proc_always_di
	.globl map_save_kernel
	.globl map_restore
	.globl _opreg
	.globl _modout

            ; imported symbols
	.globl _program_vectors
        .globl _ramsize
        .globl _procmem
	.globl _nbanks
	.globl _banktype
	.globl ___sdcc_enter_ix

	.globl s__COMMONMEM
	.globl l__COMMONMEM

        .include "kernel.def"
        .include "../../cpu-z80/kernel-z80.def"

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xE800, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_hardware:
	call detect_ext
        ld (_ramsize), hl
	ld de, #64		; for kernel
	or a
	sbc hl, de
        ld (_procmem), hl

        ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
        ld hl, #0
        push hl
        call _program_vectors
        pop hl

	; Compiler helper vectors - in kernel bank only

	ld	hl,#rstblock
	ld	de,#8
	ld	bc,#32
	ldir

        im 1 ; set CPU interrupt mode

	; interrupt mask
	; 60Hz timer on

	ld a, #0x24		; 0x20 for serial
	out (0xe0), a
        ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

opsave:
	.db	0x06	; used as a word so keep together
extsave:
	.db	0x01
_opreg:
	.db	0x86	; kernel map, 80 columns, linear video memory
_modout:
	.db	0x50	; 80 column, sound enabled, altchars off,
			; external I/O enabled, 4MHz
extmem:
	.db	0x00

detect_ext:
	 in	a,(0x94)
	 cp	#0xFF
         jr	z, bank94_absent
;
;	Seems we have banking extensions present
;
;	Write 0xA5 into a fixed location in all banks but zero
;
;	For each bank check the byte selected is A5 and write the bank into
;	it. When we see a bank not reporting A5 we have found the wrap.
;
	ld	a,#0x63		; map extended memory into low 32K
	out	(0x84),a
	ld	a, #0xA5
	ld	bc, #0x1f94
	ld	hl, #0x80	; pick somewhere which won't hit us!
markall:
	out	(c), b
	ld	(hl), a
	djnz	markall

scanall:
	inc	b
	bit	5, b
	jr	nz, scandone
	out	(c), b
	cp	(hl)		; Still A5 ?
	jr	nz, scandone
	ld	(hl), b
	jr	scanall
scandone:
	ld	a,#1
expander_found:
	ld	(_banktype),a	; Huffman on 0x94
	ld	a,(_opreg)	; restore mapping of kernel
	out	(0x84),a
	ld	a, b
	ld	(_nbanks), a
	xor	a
	; B is the first bank we don't have, 0 based, so B is number of
	; 64K banks we found (max 32)
	srl	b	; BA *= 64
	rra
	srl	b
	rra
	ld	l, a	; Report size in HL
	ld	h, b
	ret
bank94_absent:
	; Look for Hypermem. Controlled by port 0x90 bits 4-1 but write only
	ld	a, #0x63		; map extended memory into low 32K
	out	(0x84),a
	xor	a
	ld	bc,#0x0290
	ld	hl,#0x80	; safe scribble spot
	out	(c),a
	ld	(hl),a
	out	(c),b
	; If switching bank 0 to 2 changed this then it is present
	cp	(hl)
	jr	nz, is_hyper
	; Secondary check
	inc	(hl)
	out	(c),a
	; Back to original, second bank changed
	cp	(hl)
	; The inc (hl) adjusted in both maps - not present
	jr	nz, no_hyper
	;
	; Now count how many banks we have
	;
is_hyper:
	ld	bc,#0x3E90
markall90:
	out	(c),b
	ld	(hl),a
	dec	b
	djnz	markall90
scanall90:
	inc	b
	inc	b
	bit	6,b
	jr	nz, scandone90
	out	(c),b
	cp	(hl)
	jr	nz, scandone90
	ld	(hl),b
	jr	scanall90
scandone90:
	; Change from nop out (0x94),a to
	; add a out (0x94),a
	ld	a,#0x90
	ld	(bankpatch + 2),a
	ld	a,#0x87		; ADD A,A
	ld	(bankpatch),a
	ld	a,#2		; Hypermem
	jr	expander_found

no_hyper:
	; 128K RAM
	ld	a,(_opreg)	; restore mapping of kernel
	out	(0x84),a
	ld	a,#0xC9		; RET
	ld	(set_extmem),a	; Blank the expansion RAM set
	xor	a
	ld	h, a
	ld	l, #128
	; We ought to check/abort on a 64K box ?
	ret
;
;	Mapping set up for the TRS80 4/4P
;
;	The top 32K bank holds kernel code and pieces of common memory
;	The lower 32K is switched between the various user banks and the
;	base kernel bank
;
map_kernel:
map_kernel_di:
map_kernel_restore:
	push	af
	ld	a, (_opreg)
	and	#0x8C		; keep video bits
	or	#0x02		; map 2, base memory
	ld	(_opreg), a
	out	(0x84), a	; base memory so 0x94 doesn't matter
	pop	af
	ret
;
;	Userspace mapping is mode 3, U64K/L32 mapped at L64K/L32
;	Mapping codes 0x63 / 0x73. 0x94 on a bank expanded TRS80 then
;	selects how the upper bank decodes
;
map_proc:
map_proc_di:
	ld	a, h
	or	l
	jr	z, map_kernel
map_proc_hl:
	ld	a, (_opreg)
	and	#0x8C
	or	#0x63
	bit	7, (hl)		; low or high ?
	jr	z, maplo
	or	#0x10
maplo:
	ld	(_opreg), a
	out	(0x84), a
	ld	a, (hl)		; self modified
set_extmem:
	and	#0x1F
	ret	z		; not using extended bank
	ld	(extmem),a
bankpatch:
	nop			; self modified
	out	(0x94), a	; self modified
	ret

map_proc_a:			; used by bankfork
	push	af
	push	bc
	ld	b, a
	ld	a, (_opreg)
	and	#0x8C
	or	#0x63
	bit	7, b
	jr	z, maplo2
	or	#0x10
maplo2:
	ld	(_opreg), a
	out	(0x84), a
	ld	a, b
	call	set_extmem
	pop	bc
	pop	af
	ret

map_proc_always:
map_proc_always_di:
	    push af
	    push hl
	    ld hl, #_udata + U_DATA__U_PAGE
	    call map_proc_hl
	    pop hl
	    pop af
	    ret

map_save_kernel:
	push af
	ld	a, (_opreg)
	and	#0x73
	ld	(opsave), a
	ld	a,(extmem)
	ld 	(extsave), a
	ld	a, (_opreg)
	and	#0x8C		; keep video bits
	or	#0x02		; map 2, base memory
	ld	(_opreg), a
	out	(0x84), a	; base memory so 0x94 doesn't matter
	pop	af
	ret

map_restore:
	push	af
	push	bc
	ld	bc, (opsave)		; c = opsave b = extsave
	ld	a, (_opreg)
	and	#0x8C
	or	c
	ld	(_opreg), a
	out	(0x84), a
	ld	a, b
	call	set_extmem
	pop	bc
	pop	af
	ret
;
;	Stub helpers for code compactness. Note that
;	sdcc_enter_ix is in the standard compiler support already
;
	.area _DISCARD

;
;	The first two use an rst as a jump. In the reload sp case we don't
;	have to care. In the pop ix case for the function end we need to
;	drop the spare frame first, but we know that af contents don't
;	matter
;

rstblock:
	jp	___sdcc_enter_ix
	.ds	5
___spixret:
	ld	sp,ix
	pop	ix
	ret
	.ds	3
___ixret:
	pop	af
	pop	ix
	ret
	.ds	4
___ldhlhl:
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	ret
