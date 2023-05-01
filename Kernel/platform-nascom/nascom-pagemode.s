;
;	NASCOM/Gemini 'page mode' banking
;

            .module nascom-pagemode

            ; exported symbols
            .globl init_hardware
	    .globl map_kernel
	    .globl map_kernel_di
	    .globl map_kernel_restore
	    .globl map_process
	    .globl map_process_di
	    .globl map_process_a
	    .globl map_process_always
	    .globl map_process_always_di
	    .globl map_save_kernel
	    .globl map_restore
	    .globl map_for_swap
	    .globl map_buffers

            ; imported symbols
	    .globl _program_vectors
            .globl _ramsize
            .globl _procmem
	    .globl _bankmap

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

            .include "kernel.def"
            .include "../kernel-z80.def"

;
;	We keep the probe routine in the common copy area as we need
;	to bank flip as we probe
;
	    .area _COMMONMEM
banktest:
	    out (0xFF),a
	    ld a,#0xAA
	    cp (hl)
	    ret nz
	    inc (hl)
	    ld (de),a		; any old load, just cause bus traffic
	    inc a
	    cp (hl)
	    ret

size_ram:
	    ld hl,#0xE7FF
	    ld de,#0xE7FE
	    ld a,#0xF1
	    out (0xFF),a		; write all
	    ; Now test each bank and see who is present. There can be holes
	    xor a
	    ld c,a
	    ld b,a
	    ld (de),a
	    ld a,#0x11
size_loop:
	    call banktest
	    jr z,nobank
	    set 0,c		; mask of banks present
	    inc b		; count of banks present
nobank:
	    sla a
	    sla c
	    jr nc,size_loop  
	    ld a,#0x11
	    out (0xFF),a	; back to kernel map
	    srl c
	    ld (_bankmap),bc	; count and mask
	    ; Compute 64 * B
	    ld hl,#0
	    srl b
	    rr l
	    srl b
	    rr l
	    ld h,b
	    ret

            .area _CODE
	    
init_hardware:
	    ld a,#0xF1		; Write all banks read kernel
	    out (0xFF),a
            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
	    ld hl, #s__COMMONMEM
	    ld d,h
	    ld e,l
	    ld bc, #l__COMMONMEM
	    ldir		; Common copy into each bank
	    ld a,#0x11
	    out (0xFF),a
	    ; Only now is it safe to call into common space
	    call size_ram
            ld (_ramsize), hl
	    ld de, #64		; for kernel
	    or a
	    sbc hl, de
            ld (_procmem), hl
	    ld hl,#0
	    call _program_vectors
;
;	Need to work out how we find the right IRQ source
;
            ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONDATA

pagereg:    .db 0x11		; bank 1 R/W
pagesave:   .db 0x11		; saved copy

	    .area _COMMONMEM
;
;	Mapping set up for the Nascom 'Page mode'
;
;	The kernel is in bank 1, the user processes in the other banks. We
;	take care to keep common writables in COMMONDATA which is the area
;	of unbanked memory.
;
map_kernel:
map_kernel_di:
map_kernel_restore:
map_buffers:
	    push af
	    ld a,#0x11
	    ld (pagereg),a
	    out (0xFF), a
	    pop af
	    ret
;
;	Do the page mode switch
;
map_process:
map_process_di:
	    ld a, h
	    or l
	    jr z, map_kernel
	    ld a, (hl)
map_for_swap:
map_process_a:			; used by bankfork
	    ld (pagereg),a
	    out (0xFF),a
	    ret

map_process_always:
map_process_always_di:
	    push af
	    push hl
	    ld hl, #_udata + U_DATA__U_PAGE
	    ld a, (hl)
	    ld (pagereg),a
	    out (0xFF),a
	    pop hl
	    pop af
	    ret

map_save_kernel:
	    push af
	    ld a,(pagereg)
	    ld (pagesave), a
	    ld a,#0x11
	    ld (pagereg),a
	    out (0xFF), a
	    pop af
	    ret

map_restore:
	    push af
	    ld a, (pagesave)
	    ld (pagereg), a
	    out (0xFF), a
	    pop af
	    ret
