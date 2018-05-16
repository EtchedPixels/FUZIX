;
;	    TRS 80 banking logic for the base Model 4 and 4P
;

            .module trs80bank

            ; exported symbols
            .globl init_hardware
	    .globl map_kernel
	    .globl map_process
	    .globl map_process_a
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore
	    .globl _opreg
	    .globl _modout

            ; imported symbols
	    .globl _program_vectors	
            .globl _ramsize
            .globl _procmem

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

            .include "kernel.def"
            .include "../kernel.def"

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xE800, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_hardware:
	    in a,(0x94)			; Check for the Huffman banking
	    cp #0xFF			; mod. If so set 0x94 bit 0 so 
	    jr z, bank_normal		; we run sanely
	    set 0,a
	    out (0x94),a
bank_normal:
            ; set system RAM size
            ld hl, #128
            ld (_ramsize), hl
            ld hl, #(128-64)		; 64K for kernel
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

opsave:	    .db 0x06
_opreg:	    .db 0x06	; kernel map, 80 columns
_modout:    .db 0x50	; 80 column, sound enabled, altchars off,
			; external I/O enabled, 4MHz
;
;	Mapping set up for the TRS80 4/4P
;
;	The top 32K bank holds kernel code and pieces of common memory
;	The lower 32K is switched between the various user banks. On a
;	4 or 4P without add in magic thats 0x62 and 0x63 mappings.
;
map_kernel:
	    push af
	    ld a, (_opreg)
	    and #0x8C		; keep video bits
	    or #0x02		; map 2, base memory
	    ld (_opreg), a
	    out (0x84), a
	    pop af
	    ret
;
;	Userspace mapping is mode 3, U64K/L32 mapped at L64K/L32
;
map_process:
	    ld a, h
	    or l
	    jr z, map_kernel
map_process_hl:
	    ld a, (_opreg)
	    and #0x8C
	    or (hl)		; udata page
	    ld (_opreg), a
	    out (0x84), a
            ret

map_process_a:			; used by bankfork
	    push af
	    push bc
	    ld b, a
	    ld a, (_opreg)
	    and #0x8C
	    or b
	    ld (_opreg), a
	    out (0x84), a
	    pop bc
	    pop af
	    ret

map_process_always:
	    push af
	    push hl
	    ld hl, #U_DATA__U_PAGE
	    call map_process_hl
	    pop hl
	    pop af
	    ret

map_save:   push af
	    ld a, (_opreg)
	    and #0x73
	    ld (opsave), a
	    pop af
	    ret

map_restore:
	    push af
	    push bc
	    ld a, (opsave)
	    ld b, a
	    ld a, (_opreg)
	    and #0x8C
	    or b
	    ld (_opreg), a
	    out (0x84), a
	    pop bc
	    pop af
	    ret
	    
