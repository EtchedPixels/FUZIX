;
;	SC126 support code. Based closely upon the N8VEM mark IV code by
;	Will Sowerbutts
;
        .module sc126
        .z180

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl _detect_1mb
        .globl inchar
        .globl outchar
        .globl platform_interrupt_all
        .globl _bufpool

        ; imported symbols
        .globl z180_init_hardware
        .globl z180_init_early
        .globl outhl
        .globl outnewline
        .globl _rtc_shadow

        .include "kernel.def"
        .include "../cpu-z180/z180.def"
        .include "../kernel-z80.def"

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


_detect_1mb:
        ; Try to enable and probe additional RAM chip

        ; Select U2 for low half of physical memory
        ld hl, #_rtc_shadow
        ld a, (hl)
        or a, #0x02
        out (0x0C), a
        ld (hl), a

        ; Will need to examine a byte in the top 4K logical region (controlled
        ; by CBR) and this also gives us a convenient 0.
        ld hl, #0xFF00

        in0 c, (MMU_CBR)
        out0 (MMU_CBR), l

        ld a, (hl)
        xor a, #0x55
        ld (hl), a
        ld b, a
        ld a, (hl)

        out0 (MMU_CBR), c

        xor a, b
        ret nz			; return 0 in L
        inc l			; return 1
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

platform_interrupt_all:
        ret
