;
;    ZX Spectrum 128 hardware support
;
;
;    This goes straight after udata for common. Because of that the first
;    256 bytes get swapped to and from disk with the uarea (512 byte disk
;    blocks). This isn't a problem but don't put any variables in here.
;
;    If you make this module any shorter, check what follows next
;


        .module zx128

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
        .globl platform_interrupt_all

        .globl map_kernel
        .globl map_process
        .globl map_process_always
        .globl map_save
        .globl map_restore

        .globl _fd_bankcmd
        .globl _kernel_flag

        ; exported debugging tools
        .globl _trap_monitor
        .globl outchar

        ; imported symbols
        .globl _ramsize
        .globl _procmem

        .globl outcharhex
        .globl outhl, outde, outbc
        .globl outnewline
        .globl outstring
        .globl outstringhex

        .include "kernel.def"
        .include "../kernel.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_trap_monitor:
        ld a, #128
        ; out (29), a ; TODO: go where? BASIC48?
platform_interrupt_all:
        ret

_trap_reboot:
        rst 0

;
;    We need the right bank present when we cause the transfer
;
_fd_bankcmd:
        ret
        pop de        ; return
        pop bc        ; command
        pop hl        ; bank
        push hl
        push bc
        push de        ; fix stack
        ld a, i
        di
        push af        ; save DI state
        call map_process    ; HL alread holds our bank
        ld a, c        ; issue the command
        ; out (13), a        ;
        call map_kernel    ; return to kernel mapping
        pop af
        ret po
        ei
        ret

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xC000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
        .area _CODE

init_early:
        ld bc, #0x7ffd
        xor a
        ld (current_map), a
        out (c), a            ; set page 0 at 0xC000
        ret

init_hardware:
        ; set system RAM size
        ld hl, #128
        ld (_ramsize), hl
        ld hl, #(128 - 48)        ; 48K for kernel
        ld (_procmem), hl

        ; screen initialization
        ; clear
        ld hl, #0x4000
        ld de, #0x4001
        ld bc, #0x1800
        xor a
        ld (hl), a
        ldir

        ; set color attributes
        ld a, #7            ; black paper, white ink
        ld bc, #0x300 - #1
        ld (hl), a
        ldir

        im 1 ; set CPU interrupt mode
        ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .area _COMMONMEM

        ; our vectors are in ROM, so nothing to do here
_program_vectors:
        ret

        ; bank switching procedure. On entrance:
        ;  A - bank number to set
switch_bank:
        ld (current_map), a
        ld a, b
        ld (place_for_b), a
        ld a, c
        ld (place_for_c), a
        ld bc, #0x7ffd
        ld a, (current_map)
;        out (c), a
        and #0xff
        jr z, sb_restore
        ld (current_process_map), a
sb_restore:
        ld a, (place_for_b)
        ld b, a
        ld a, (place_for_c)
        ld c, a
        ld a, (place_for_a)
        ret

map_kernel:
        ld (place_for_a), a
map_kernel_nosavea:          ; to avoid double reg A saving
        xor a
        jr switch_bank

map_process:
        ld (place_for_a), a
        ld a, h
        or l
        jr z, map_kernel_nosavea
        ld a, (hl)
        jr switch_bank

map_process_always:
        ld (place_for_a), a
        ld a, (current_process_map)
        jr switch_bank

map_save:
        ld (place_for_a), a
        ld a, (current_map)
        ld (map_store), a
        ld a, (place_for_a)
        ret

map_restore:
        ld (place_for_a), a
        ld a, (map_store)
        jr switch_bank

current_map:                ; place to store current page number. Is needed
        .db 0               ; because we have no ability to read 7ffd port
                            ; to detect what page is mapped currently 
map_store:
        .db 0

current_process_map:
        .db 0

place_for_a:                ; When change mapping we can not use stack since it is located at the end of banked area.
        .db 0               ; Here we store A when needed
place_for_b:                ; And BC - here
        .db 0
place_for_c:
        .db 0

; outchar: TODO: add something here (char in A). Current port #15 is emulator stub
outchar:
        out (#0x15), A
        ret
_kernel_flag:
        .db 1