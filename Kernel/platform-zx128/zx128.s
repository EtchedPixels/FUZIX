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
        .globl _system_tick_counter
        .globl platform_interrupt_all

        .globl map_kernel
        .globl map_process
        .globl map_process_always
        .globl map_save
        .globl map_restore

        .globl _fd_bankcmd

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

        ; below is code which was copied from z80pack, so it's useless for zx128
map_kernel:
        push af
        xor a
        ; out (21), a
        pop af
        ret

map_process:
        ld a, h
        or l
        jr z, map_kernel
        ld a, (hl)
        ; out (21), a
        ret

map_process_always:
        push af
        ld a, (U_DATA__U_PAGE)
        ; out (21), a
        pop af
        ret

map_save:
        push af
        in a, (21)
        ld (map_store), a
        pop af
        ret

map_restore:
        push af
        ld a, (map_store)
        ; out (21), a
        pop af
        ret

map_store:
        .db 0

; outchar: TODO: add something here (char in A). Current port #15 is emulator stub
outchar:
        out (#0x15), A
        ret
