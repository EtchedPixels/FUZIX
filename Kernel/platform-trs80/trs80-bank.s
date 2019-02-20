;
;	    TRS 80 banking logic for the base Model 4 and 4P with
;	    the port 0x94 banking extensions.
;

            .module trs80bank

            ; exported symbols
            .globl init_hardware
	    .globl map_kernel
	    .globl map_kernel_di
	    .globl map_process
	    .globl map_process_di
	    .globl map_process_a
	    .globl map_process_always
	    .globl map_process_always_di
	    .globl map_save_kernel
	    .globl map_restore
	    .globl _opreg
	    .globl _modout

            ; imported symbols
	    .globl _program_vectors
            .globl _ramsize
            .globl _procmem
	    .globl _nbanks

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

            .include "kernel.def"
            .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xE800, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_hardware:
	    call detect94
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

            im 1 ; set CPU interrupt mode

	    ; interrupt mask
	    ; 60Hz timer on

	    ld a, #0x24		; 0x20 for serial
	    out (0xe0), a
            ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

opsave:	    .db 0x06	; used as a word so keep together
save94:	    .db 0x01
_opreg:	    .db 0x06	; kernel map, 80 columns
_modout:    .db 0x50	; 80 column, sound enabled, altchars off,
			; external I/O enabled, 4MHz

detect94:
	     in a,(0x94)
	     cp #0xFF
             jr z, bank94_absent
;
;	Seems we have banking extensions present
;
;	Write 0xA5 into a fixed location in all banks but zero
;
;	For each bank check the byte selected is A5 and write the bank into
;	it. When we see a bank not reporting A5 we have found the wrap.
;
	    ld a, #0x63		; map extended memory into low 32K
	    out (0x84),a
	    ld a, #0xA5
	    ld bc, #0x1f94
	    ld hl, #0x80	; pick somewhere which won't hit us!
markall:
	    out (c), b
	    ld (hl), a
	    djnz markall

scanall:    inc b
	    bit 5, b
	    jr nz, scandone
	    out (c), b
	    cp (hl)		; Still A5 ?
	    jr nz, scandone
	    ld (hl), b
	    jr scanall
scandone:
	    ld a,(_opreg)	; restore mapping of kernel
	    out (0x84),a
	    ld a, b
	    ld (_nbanks), a
	    xor a
	    ; B is the first bank we don't have, 0 based, so B is number of
	    ; 64K banks we found (max 32)
	    srl b	; BA *= 64
	    rra
	    srl b
	    rr a
	    ld l, a	; Report size in HL
	    ld h, b
	    ret
bank94_absent:
	    xor a
	    ld h, a
	    ld l, #128		; We ought to check/abort on a 64K box ?
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
	    push af
	    ld a, (_opreg)
	    and #0x8C		; keep video bits
	    or #0x02		; map 2, base memory
	    ld (_opreg), a
	    out (0x84), a	; base memory so 0x94 doesn't matter
	    pop af
	    ret
;
;	Userspace mapping is mode 3, U64K/L32 mapped at L64K/L32
;	Mapping codes 0x63 / 0x73. 0x94 on a bank expanded TRS80 then
;	selects how the upper bank decodes
;
map_process:
map_process_di:
	    ld a, h
	    or l
	    jr z, map_kernel
map_process_hl:
	    ld a, (_opreg)
	    and #0x8C
	    or #0x63
	    bit 7, (hl)		; low or high ?
	    jr z, maplo
	    or #0x10
maplo:
	    ld (_opreg), a
	    out (0x84), a
	    ld a, (hl)
	    and #0x1F
	    ret z		; not using bank94
	    out (0x94), a
	    ret

map_process_a:			; used by bankfork
	    push af
	    push bc
	    ld b, a
	    ld a, (_opreg)
	    and #0x8C
	    or #0x63
	    bit 7, b
	    jr z, maplo2
	    or #0x10
maplo2:
	    ld (_opreg), a
	    out (0x84), a
	    ld a, b
	    and #0x1f
	    jr z, nobank94	; zero means no bank94 hw present
	    out (0x94), a
nobank94:
	    pop bc
	    pop af
	    ret

map_process_always:
map_process_always_di:
	    push af
	    push hl
	    ld hl, #_udata + U_DATA__U_PAGE
	    call map_process_hl
	    pop hl
	    pop af
	    ret

map_save_kernel:
	    push af
	    ld a, (_opreg)
	    and #0x73
	    ld (opsave), a
	    in a,(0x94)
	    ld (save94), a
	    ld a, (_opreg)
	    and #0x8C		; keep video bits
	    or #0x02		; map 2, base memory
	    ld (_opreg), a
	    out (0x84), a	; base memory so 0x94 doesn't matter
	    pop af
	    ret

map_restore:
	    push af
	    push bc
	    ld bc, (opsave)		; c = opsave b = save94
	    ld a, (_opreg)
	    and #0x8C
	    or c
	    ld (_opreg), a
	    out (0x84), a
	    ld a, b
	    and #0x1f
	    jr z, norestore94
	    out (0x94), a
norestore94:
	    pop bc
	    pop af
	    ret
