;
;	    TRS 80 banking logic for the selector and alpha supermem
;

            .module trs80bank

            ; exported symbols
            .globl init_hardware
	    .globl map_kernel
	    .globl map_proc
	    .globl map_proc_a
	    .globl map_proc_hl
	    .globl map_proc_always
	    .globl map_kernel_di
	    .globl map_proc_di
	    .globl map_proc_always_di
	    .globl map_proc_save
	    .globl map_kernel_restore
	    .globl map_save_kernel
	    .globl map_restore
	    .globl map_save_kmap
	    .globl map_restore_kmap
	    .globl fork_mapsave
	    .globl mapper_init

            ; imported symbols
	    .globl _program_vectors	
            .globl _ramsize
            .globl _procmem
	    .globl vtbufinit
	    .globl _trs80_mapper
	    .globl ___hard_di
	    .globl ___hard_irqrestore

	    .globl bankpatch1
	    .globl bankpatch2

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM


	    .globl __uget
            .globl __ugetc
	    .globl __ugetw
            .globl __uput
            .globl __uputc
            .globl __uputw
            .globl __uzero

            .include "kernel.def"
            .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (above 08000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _BOOT

init_hardware:
	    push af
	    call vtbufinit
	    pop af
	    ld a,(_trs80_mapper)
	    or a
	    ld hl,#0xFFFF		; FFFF is free in all our pages
	    jr nz, memsize_selector
            ; set system RAM size
	    ld bc,#0xFF43		; kernel included
	    xor a
mark_pages:
	    out (c),b
	    ld (hl),a
	    djnz mark_pages
scan_pages:
	    out (c),a			; bank 0
	    ld (hl),#0xff		; so we catch any wrapping
	    ld b,#2
scan_pages_l:
	    out (c),b
	    cp (hl)			; still 0 ?
	    jr nz, mismatch
	    ld (hl),b
	    inc b
	    jr nz, scan_pages_l
mismatch:				; B holds the first page above
					; the limit (4 for 128K etc)
	    ld e,b
	    dec e
	    jr scan_done

memsize_selector:
            ; set system RAM size	- HL already points to FFFF
	    ld bc,#0x381f
	    ld e,#4			; only 4 pages to test
mark_up:
	    out (c),b
	    ld (hl),e
	    ld a,#0xF0			; subtract 0x10
	    add b
	    ld b,a
	    dec e
	    jr nz, mark_up
	    ld bc,#0x081f
	    ld e,#1
scan:
	    out (c),b
	    ld a,(hl)
	    cp e
	    jr nz,scan_done
	    ld a,#0x10
	    add b
	    ld b,a
	    inc e
	    ld a,#5
	    cp e
	    jr nz,scan
	    dec e
scan_done:
;
;	At this point E is the number of external banks we have
;
	    dec e			; one holds kernel code
	    ld h,e
	    ld l,#0			; effectively * 256
	    srl h
	    rr  l			; * 128
	    srl h
	    rr	l			; * 64
	    srl h
	    rr  l			; * 32 so now in Kbytes
            ld (_procmem), hl
	    ld de,#80			; Kernel costs us 80K due to bank
	    add hl,de			; annoyances
            ld (_ramsize), hl

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

            im 1 ; set CPU interrupt mode
            ret

;
;	Must live here as we need the mapper set up before we make any
;	banked calls.
;
mapper_init:
	    ld (_trs80_mapper),a
	    or a
	    jr nz, mapper_selector
	    ld hl,#0x0043
	    ld (ksave_map),hl
	    ld (map_reg),hl
	    ld (map_bank1),hl
	    inc h
	    ld (map_bank2),hl
	    ret
mapper_selector:
	    ; Patch the fast copier for our port numbering
	    ld a,#0x1f
	    ld (bankpatch1 + 1),a
	    ld (bankpatch2 + 1),a
	    ld (bankpatch3 + 1),a
	    ld (bankpatch4 + 1),a
	    ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

_trs80_mapper:
	    .db 0x01	; Tell kernel mapper is Selector
ksave_map:  .db 0x1f
	    .db 0x00	; saved kernel map version
map_reg:    .db 0x1f	; so we can write a reg pair into map_reg
	    .db 0x00	; current value written to map register
map_bank1:  .db 0x1f	; port
	    .db 0x00	; kernel map using system RAM, assuming no low 16K
			; and high banking with I/O mapped
map_bank2:  .db 0x1f	; port
	    .db 0x08	; kernel map using first 32K of expansion memory
			; and high banking with I/O mapper
map_store:  .db 0x1f
	    .db 0x00	; save value from map_save

;
;	Map in the kernel. We are in common and all kernel entry points are
;	in common. Thus someone will always have banked in the right page
;	before jumping out of common. Thus we don't need to do anything but
;	restore the last saved kernel map!
;
map_kernel_di:
map_kernel:
map_kernel_restore:
	    push bc
	    ld bc, (ksave_map)
	    ld (map_reg), bc
	    out (c), b
	    pop bc
	    ret

fork_mapsave:
	    push bc
	    ld bc,(map_reg)
	    ld (ksave_map),bc
	    pop bc
	    ret
;
;	Select the bank for the relevant process. Update the ksave_map so we
;	can restore the correct kernel mapping when banked.
;
map_proc_di:
map_proc:
	    ld a, h
	    or l
	    jr z, map_kernel
map_proc_hl:
            ld bc, (map_reg)
	    ld (ksave_map),bc	; for map_kernel_restore
	    ld b,(hl)		; udata page
	    ld (map_reg),bc
	    out (c), b
            ret

map_proc_a:			; used by bankfork
	    push bc
	    ld bc, (map_reg)
	    ld (ksave_map), bc
	    ld b, a
	    ld (map_reg), bc
	    out (c), b
	    pop bc
	    ret

map_proc_save:
map_proc_always:
map_proc_always_di:
	    push af
	    push bc
	    ld bc, (map_reg)
	    ld (ksave_map), bc
	    ld a,(_udata + U_DATA__U_PAGE)
	    ld b,a
	    ld (map_reg),bc
	    out (c),b
	    pop bc
	    pop af
	    ret

map_save_kernel:
	    push bc
	    ld bc, (map_reg)
	    ld (map_store), bc
	    ld bc, (ksave_map)
	    ld (map_reg), bc
	    out (c), b
	    pop bc
	    ret

map_restore:
	    push bc
	    ld bc, (map_store)
	    ld (map_reg), bc
	    out (c), b
	    pop bc
	    ret

map_save_kmap:
	    ld a,(map_reg + 1)
	    ret

map_restore_kmap:
	    ld (map_reg + 1),a
	    push bc
	    ld bc, (map_reg)
	    out (c),b
	    pop bc
	    ret

;
;	This lot is tricky.
;
	.globl __bank_0_1
	.globl __bank_0_2
	.globl __bank_1_2
	.globl __bank_2_1
	.globl __stub_0_1
	.globl __stub_0_2

;
;	Start with the easy ones - we are going from common
;	to a bank. We can use registers here providing the compiler expects
;	them to be destroyed by the called function, and on the return
;	providing they are also not a return value. In practice that means
;	we need to avoid IX going in and IX HL DE coming out.
;
;	The linker rewrote
;			push af call foo pop af
;	into
;			call __bank_0_1 defw foo
;
;	We expand these into separate functions as they are executed a fair
;	bit
;
;	The only hard case to understand logically here is calls and stubs
;	from common to a bank. In those cases we must save the previous bank
;	and restore it. We do this because we optimise the performance by
;	making calls into COMMON or CODE free of bank work so we can put
;	performance sensitive widely used routines (like compiler helpers)
;	there. The cost of that is that if we do call back out of common
;	space we have to do the restore as we return.
;
__bank_0_2:			; Call from common to bank 2
	ld bc,(map_bank2)
	jr bankina
__bank_0_1:			; Call from common to bank 1
	ld bc,(map_bank1)
bankina:
	pop hl			; Get the return address
	ld e,(hl)		; The two bytes following it are the true
	inc hl			; function to call
	ld d,(hl)
	inc hl
	push hl
	ld (map_reg),bc
	out (c),b
	ex de,hl
	ld a,(map_bank1 + 1)
	cp b
	jr z, retbank1		; Arrange that we put the banks back as they
	call callhl		; were before the call. Needed because calls
	ld bc,(map_bank2)	; to common routines don't do banking so
	ld (map_reg),bc		; the return won't either.
	out (c),b
	ret
retbank1:
	call callhl
	ld bc,(map_bank1)
	ld (map_reg),bc
	out (c),b
	ret

__bank_1_2:
	ld bc,(map_bank2)
	pop hl			; Get the return address
	ld e,(hl)		; The two bytes following it are the true
	inc hl			; function to call
	ld d,(hl)
	inc hl
	push hl
	ld (map_reg),bc
	out (c),b
	ex de,hl
	call callhl
	ld bc,(map_bank1)
	ld (map_reg),bc
	out (c),b
	ret

__bank_2_1:
	ld bc,(map_bank1)
	pop hl			; Get the return address
	ld e,(hl)		; The two bytes following it are the true
	inc hl			; function to call
	ld d,(hl)
	inc hl
	push hl
	ld (map_reg),bc
	out (c),b
	ex de,hl
	call callhl
	ld bc,(map_bank2)
	ld (map_reg),bc
	out (c),b
	ret

;
;	Stubs are similar. Where there is a function pointer the linker adds
;	a call in common and resolves the function pointer to the code in
;	common. The function stub it adds is simply
;
;		ld de,#realaddr
;		call __stub_0_%d   where %d is the target bank
;
;	For the same reason as bank calls we need to restore the previous
;	banking setup.
;
__stub_0_1:
	ld bc,(map_bank1)
	jr stub_call
__stub_0_2:
	ld bc,(map_bank2)
stub_call:
	pop hl			; return address
	ex (sp),hl		; swap the junk word for the return
	ld a,(map_reg+1)	; where are we going back to ?
	ld (map_reg),bc
	out (c),b
	ex de,hl
	ld bc,(map_bank1)
	cp b
	jr z, stub_ret_1
	call callhl
	ld bc,(map_bank2)
	jr stub_ret
stub_ret_1:
	call callhl		; invoke code in other bank
	ld bc,(map_bank1)	; and back to bank 1
stub_ret:
	ld (map_reg),bc
	out (c),b
	pop bc			; bc is now the return
	push bc			; stack it twice
	push bc
	ret			; and ret - can't use jp (ix) or jp (hl) here
callhl:	jp (hl)

;
;	Fast inter-bank copier. Copies from kernel common, data or banked
;	data (logical bank2) to/from userspace
;
	.area _COMMONMEM

;
;	We could use sp and make this faster still but it gets ugly and
;	complicated. Might be worth doing pairs of bytes though.
;
b2bcopy:
	ld a, b
	ld (patch0 + 1), a	; source bank
	ld a, c
	ld (patch1 + 1), a	; destination bank
	exx
b2b_loop:
patch0:
	ld a,#0
bankpatch3:
	out (0x43),a
	ld a,(hl)
	inc hl
	ex af,af'
patch1:
	ld a,#0
bankpatch4:
	out (0x43),a
	ex af,af'
	ld (de),a
	inc de
	djnz b2b_loop
	dec c
	jr nz, b2b_loop
	ret

uputget:
        ; load DE with the byte count
        ld e, 10(ix) ; byte count
        ld d, 11(ix)
	ld a, d
	or e
	ret z		; no work
	dec de		; we return BC as a count for two 8bit loops
	ld b, e		; not a 16bit value
	inc b		; See http://map.grauw.nl/articles/fast_loops.php
	inc d
	ld c, d
        ; load HL with the source address
        ld l, 6(ix) ; src address
        ld h, 7(ix)
        ; load DE with destination address (in userspace)
        ld e, 8(ix)
        ld d, 9(ix)
	ld a, b
	or c
	ret

__uput:
	push ix
	ld ix,#0
	add ix,sp
	call uputget
	jr z, uput_out
	exx
	; Disable interrupts, returns old state in HL
	; Use helper as NMOS Z80 has bugs in this area
	call ___hard_di
	; Our banks
	ld a, (_udata + U_DATA__U_PAGE)
	ld c, a
	ld a, (map_bank2 + 1)	; kernel is source
	ld b, a
ucopier:
	; Save the mapping
	ld de, (map_reg)	; save old mapping
	push hl
	push de
	call b2bcopy
	pop bc			; port and value
	out (c), b
	call ___hard_irqrestore
	pop hl
uput_out:
	pop ix
	ld hl,#0
	ret

__uget:
	push ix
	ld ix, #0
	add ix, sp
	call uputget
	jr z, uput_out
	exx
	call ___hard_di
	ld a, (_udata + U_DATA__U_PAGE)
	ld b, a
	ld a, (map_bank2 + 1)	; kernel is destination
	ld c, a
	jr ucopier

__uputc:
	pop iy	;	bank
	pop bc	;	return
	pop de	;	char
	pop hl	;	dest
	push hl
	push de
	push bc
	push iy
	call map_proc_save
	ld (hl), e
uputc_out:
	jp map_kernel_restore			; map the kernel back below common

__uputw:
	pop iy
	pop bc	;	return
	pop de	;	word
	pop hl	;	dest
	push hl
	push de
	push bc
	push iy
	call map_proc_save
	ld (hl), e
	inc hl
	ld (hl), d
	jp map_kernel_restore

__ugetc:
	call map_proc_save
        ld l, (hl)
	ld h, #0
	jp map_kernel_restore

__ugetw:
	call map_proc_save
        ld a, (hl)
	inc hl
	ld h, (hl)
	ld l, a
	jp map_kernel_restore

__uzero:
	pop iy
	pop de	; return
	pop hl	; address
	pop bc	; size
	push bc
	push hl
	push de
	push iy
	ld a, b	; check for 0 copy
	or c
	ret z
	call map_proc_save
	ld (hl), #0
	dec bc
	ld a, b
	or c
	jp z, uputc_out
	ld e, l
	ld d, h
	inc de
	ldir
	jp uputc_out
