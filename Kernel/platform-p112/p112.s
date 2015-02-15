; 2014-12-24 William R Sowerbutts
; P112 hardware specific code

        .module p112
        .z180

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl inchar
        .globl outchar
        .globl outstring
        .globl outcharhex
        .globl platform_interrupt_all
        .globl _trap_monitor

        ; imported symbols
        .globl z180_init_hardware
        .globl z180_init_early
        .globl _ramsize
        .globl _procmem
        .globl outhl
        .globl outnewline

        .include "kernel.def"
        .include "../cpu-z180/z180.def"
        .include "../kernel.def"

; -----------------------------------------------------------------------------
; Initialisation code
; -----------------------------------------------------------------------------
        .area _DISCARD

init_early:
        ; P112: stop the floppy motor in case it is running
        ld a, #0x0c
        out0 (0x92), a

        ; Z80182: disable ROM, entire physical address space maps to RAM
        in0 a, (Z182_SYSCONFIG)
        set 3, a
        out0 (Z182_SYSCONFIG), a

        jp z180_init_early

init_hardware:
        ; set system RAM size
        ld hl, #RAM_KB
        ld (_ramsize), hl
        ld hl, #(RAM_KB-64)        ; 64K for kernel
        ld (_procmem), hl

        ; enable ASCI interrupts
        ; in0 a, (ASCI_STAT0)
        ; or #0x08                ; enable ASCI0 receive interrupts
        ; out0 (ASCI_STAT0), a
        ; in0 a, (ASCI_ASEXT0)
        ; and #0x7f               ; disable RDRF interrupt inhibit
        ; out0 (ASCI_ASEXT0), a
        ; in0 a, (ASCI_STAT1)
        ; or #0x08                ; enable ASCI1 receive interrupts
        ; out0 (ASCI_STAT1), a
        ; in0 a, (ASCI_ASEXT1)
        ; and #0x7f               ; disable RDRF interrupt inhibit
        ; out0 (ASCI_ASEXT1), a

        ; enable ESCC interrupts
        ld bc, #0x0114 ; write register 1, 0x14: enable receive interrupts only
        call write_escc
        ld bc, #0x0908 ; write register 9, 0x08: master interrupt enable
        call write_escc

        jp z180_init_hardware

write_escc:
        out0 (ESCC_CTRL_A), b
        out0 (ESCC_CTRL_A), c
        out0 (ESCC_CTRL_B), b
        out0 (ESCC_CTRL_B), c
        ret

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK
; -----------------------------------------------------------------------------
        .area _COMMONMEM

; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
        push bc
        ld b, a
        ; wait for transmitter to be idle
ocloop:     in0 a, (ESCC_CTRL_A)
        and #0x04       ; test transmit buffer empty
        jr z, ocloop
        out0 (ESCC_DATA_A), b
        pop bc
        ret

; inchar: Wait for character on UART, return in A
inchar:
        in0 a, (ESCC_CTRL_A)    ; bit 0 is "rx ready" (1=ready)
        rrca
        jr nc, inchar
        in0 a, (ESCC_DATA_A)
        ret

platform_interrupt_all:
        ret
