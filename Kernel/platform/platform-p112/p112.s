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
        .globl plt_interrupt_all
        .globl _plt_reboot
        .globl _bufpool

        ; imported symbols
        .globl z180_init_hardware
        .globl z180_init_early
        .globl outhl
        .globl outnewline

        .include "kernel.def"
        .include "../../cpu-z180/z180.def"
        .include "../../cpu-z80/kernel-z80.def"

; -----------------------------------------------------------------------------
; Buffers
; -----------------------------------------------------------------------------
        .area _BUFFERS
_bufpool:
        .ds (BUFSIZE * 4) ; adjust NBUFS in config.h in line with this

; -----------------------------------------------------------------------------
; Initialisation code
; -----------------------------------------------------------------------------
        .area _DISCARD

init_early:
        ; P112: stop the floppy motor in case it is running
        ld a, #0x08
        out0 (FDC_DOR), a

        ; Z80182: disable ROM, entire physical address space maps to RAM
        in0 a, (Z182_SYSCONFIG)
        set 3, a
        out0 (Z182_SYSCONFIG), a

        jp z180_init_early

init_hardware:
        ; configure ASCI UART
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

        ; configure ESCC UART
        ld bc, #0x0F01 ; write register 15, disable CTS status interrupts, expose WR7'
        call write_escc
        ld bc, #0x0760 ; write register 7', enable TX interrupt only when FIFO is empty, extended read mode
        call write_escc
        ; WRS: Note that WR3 "obey CTS pin" for transmit control is not useful
        ; as it also uses the DCD pin for receiver enable. Shame.
        ld bc, #0x0110 ; write register 1, enable receive interrupts
        call write_escc
        ld bc, #0x0908 ; write register 9, master interrupt enable
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

_plt_reboot:
        in0 a, (Z182_SYSCONFIG)
        res 3, a                    ; re-enable the ROM select line
        out0 (Z182_SYSCONFIG), a
        xor a
        out0 (MMU_BBR), a           ; map ROM into the lower 60K
        jp 0                        ; jump to the boot ROM entry vector

plt_interrupt_all:
        ret
