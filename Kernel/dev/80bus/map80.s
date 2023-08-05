;
;	NASCOM/Gemini with MAP80 memory in Nascom or GM811 (not GM813)
;
;	The MAP80 sits on 0xFE and can map either 32K banks or the full 64K
;	(minus the mainboard memory). We use the latter so we get fewer but
;	larger processes
;
;
;	0xFE:
;	7: set for 32K mode
;	6: set for upper 32K common clear for lower (32K mode only)
;	5-1: page number
;	0: upper/lower 32K (32K mode only)
;
;
;	Each MAP80 provides 256K so we could in theory test only every 4th
;	bank. Multiple cards provide a single larger range up to 2MB
;	although neither the slots or PSU make that actually practical.
;
;	It is possible to combine MAP80 with page mode but this results in
;	contortions we don't want to support at this point.
;

            .module map80

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
	    .globl size_ram

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
;	to bank flip as we probe. At this point common is not in all banks
;	so we use the 32K/32K mode and pin high memory
;
	    .area _COMMONMEM

size_bank:
	    ld a,#0xC2
next_bank:
	    out (0xFE),a		; select our bank
	    ld (hl),a
	    cp (hl)
	    jr nz, scan_done		; bank absent
	    inc (hl)
	    inc (hl)
	    inc a
	    inc a
	    cp (hl)
	    jr nz, scan_done
	    or a			; C0 + all bank bits
	    jr nz, next_bank
scan_done:
	    sub #2
	    and #0x3F
	    ; There are A/2 banks of 60K (F000-FFFF is base memory)
	    ; plus the base 4K ROM/VIDEO
	    ld h,#0
	    ld l,a
	    ld d,h
	    ld e,l
	    add hl,hl			; x 2
            add hl,hl			; x 4
	    add hl,hl			; x 8
	    add hl,hl			; x 16
	    or a
	    sbc hl,de			; x 15
	    add hl,hl			; x 30
	    inc hl
	    inc hl
	    inc hl
	    inc hl			; + 4K base
	    srl a
	    ; A banks - save HL and shuffle the common into each
	    ld b,a
	    push hl
	    ;
	    ;	We use 32K bank mode to set up the common copies by putting
	    ;	the top of each 64K bank into the low 32K and using ldir
	    ;
next_bank:
	    ld a,b
	    add a
	    or #0xC1			; 32K mode, upper 32K fixed
					; low 32K is upper half of target
	    out (0xFE),a
	    ld hl,#0xC000
	    ld de,#0x4000
	    ld bc,#0x4000
	    ldir
	    djnz next_bank
	    xor a
	    out (0xFE),a		; 64K mode, bank 0 mapped - normal kernel map
	    pop hl
	    ret

	    .area _COMMONMEM
size_ram:
	    ld hl,#0x0004		; Free byte in the vectors
	    xor a
	    out (0xFE),a		; Kernel map
	    ld (hl),a			; Write current bank (0) with 0
	    inc a			; Next bank
	    out (0xFE),a
	    ld (hl),a			; Write to bank 1 we hope
	    dec a
	    out (0xFE),a		; back to 0
	    cp (hl)			; Did write go to other bank ?
	    jr z, size_bank		; If it did we get NZ and skip
	    ld hl,#64			; No paged memory, just base
	    xor a
	    out (0xFE),a		; Back to memory normality
	    ret

            .area _CODE
	    
init_hardware:
	    call size_ram
            ld (_ramsize), hl
	    ld de, #64		; for kernel
	    or a
	    sbc hl, de
            ld (_procmem), hl
	    ld hl,#0
	    push hl
	    call _program_vectors
	    pop af
	    ; FIXME: interrupt mode is per port target ?
            ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONDATA

pagereg:    .db 0x00		; kernel map (bank 0)
pagesave:   .db 0x00		; saved copy

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
	    xor a
	    ld (pagereg),a
	    out (0xFE), a
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
	    out (0xFE),a
	    ret

map_process_always:
map_process_always_di:
	    push af
	    push hl
	    ld hl, #_udata + U_DATA__U_PAGE
	    ld a, (hl)
	    ld (pagereg),a
	    out (0xFE),a
	    pop hl
	    pop af
	    ret

map_save_kernel:
	    push af
	    ld a,(pagereg)
	    ld (pagesave), a
	    xor a
	    ld (pagereg),a
	    out (0xFE), a
	    pop af
	    ret

map_restore:
	    push af
	    ld a, (pagesave)
	    ld (pagereg), a
	    out (0xFE), a
	    pop af
	    ret
