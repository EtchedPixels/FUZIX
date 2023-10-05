;
;	SC126 support code. Based closely upon the N8VEM mark IV code by
;	Will Sowerbutts
;
        .module rcbus-z180
        .z180

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl _detect_1mb
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
	.globl ___sdcc_enter_ix

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
	; We have to install the rst helpers ASAP
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

	ld a,#0xC3
	ld hl,#___sdcc_enter_ix
	ld (0x08),a
	ld (0x09),hl
	ld hl,#___spixret
	ld (0x10),a
	ld (0x11),hl
	ld hl,#___ixret
	ld (0x18),a
	ld (0x19),hl
	ld hl,#___ldhlhl
	ld (0x20),a
	ld (0x21),hl
	; This will call C code so the rst vectors must be correct
	; before hand
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

plt_interrupt_all:
        ret

;
;	Stub helpers for code compactness. Note that
;	sdcc_enter_ix is in the standard compiler support already
;
	.area _CODE

;
;	The first two use an rst as a jump. In the reload sp case we don't
;	have to care. In the pop ix case for the function end we need to
;	drop the spare frame first, but we know that af contents don't
;	matter
;
___spixret:
	ld	sp,ix
	pop	ix
	ret
___ixret:
	pop	af
	pop	ix
	ret
___ldhlhl:
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	ret
