;
;	This one is a slightly odd Z180 setup due to the limited memory.
;
        .module sc126
        .z180

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl inchar
        .globl outchar
        .globl plt_interrupt_all
        .globl _bufpool

        ; imported symbols
        .globl z180_init_hardware
        .globl z180_init_early
        .globl outhl
        .globl outnewline
        .globl _rtc_shadow

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
	jp z180_init_early

init_hardware:
        ; enable ASCI interrupts
        in0 a, (ASCI_STAT0)
        or #0x08                ; enable ASCI0 receive interrupts
        out0 (ASCI_STAT0), a
        in0 a, (ASCI_ASEXT0)
        and #0x7f               ; disable RDRF interrupt inhibit
        out0 (ASCI_ASEXT0), a
        in0 a, (ASCI_STAT1)
        or #0x08                ; enable ASCI1 receive interrupts
        out0 (ASCI_STAT1), a
        in0 a, (ASCI_ASEXT1)
        and #0x7f               ; disable RDRF interrupt inhibit
        out0 (ASCI_ASEXT1), a

        jp z180_init_hardware


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
ocloop: in0 a, (ASCI_STAT0)
        bit 1, a        ; and 0x02
        jr z, ocloop    ; loop while busy
        ; now output the char to serial port
        ld a, b
        out0 (ASCI_TDR0), a
        pop bc
        ret

; inchar: Wait for character on UART, return in A
; destroys: AF
inchar:
        in0 a, (ASCI_STAT0)
        rlca
        jr nc, inchar
        in0 a, (ASCI_RDR0)
        ret

plt_interrupt_all:
        ret
