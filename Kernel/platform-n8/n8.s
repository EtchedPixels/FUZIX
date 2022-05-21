;
;	N8 platform specific code. Not too much here as the ROM did a lot of
;	work for us
;
        .module n8
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


	.globl _far_read
	.globl _far_write
	.globl _far_writable
;
;	Helpers for poking memory to test during early boot up. L is the
;	bank and for writing also the code we use
;
_far_read:
	; Flip the top 4K (incldes our stack!!!)
	in0 c,(MMU_CBR)
	out0(MMU_CBR),l
	ld hl,(0xFFFE)
	out0(MMU_CBR),c
	ret
_far_write:
	in0 c,(MMU_CBR)
	out0(MMU_CBR),l
	ld (0xFFFE),hl
	out0(MMU_CBR),c
	ret
_far_writable:
	in0 c,(MMU_CBR)
	out0(MMU_CBR),l
	ld hl,#0xFFFE
	ld a,(hl)
	inc (hl)
	cp (hl)
	jr z, no_write
	dec (hl)
	cp (hl)
	jr nz, no_write
	ld l,#1
	out0(MMU_CBR),c
	ret
no_write:
	ld l,#0
	out0(MMU_CBR),c
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
plt_interrupt_all:
        ret

