;
;	Z180 support for Z80 style systems without linear memory. For
;	full on Z180 platforms use the cpu-z180 code instead.
;
	.module z180_support
	.z180

        .include "../../cpu-z180/z180.def"

	.globl _z180_setup
        .globl interrupt_table ; not used elsewhere but useful to check correct alignment
        .globl hw_irqvector
        .globl _irqvector

;
;	Set up the Z180 timers, interrupt table etc
;
	.area	_CODE1

_z180_setup:
	ld e,l		; save timer flag

        ; program Z180 interrupt table registers
	; assumes that for Z180 at least we run in IM1

        ld hl, #interrupt_table ; note table MUST be 32-byte aligned!
        out0 (INT_IL), l
	ld a,h
	ld i,a
	im 1

        ; set up system tick timer
	;
	; If we have another source we will use that (because the other
	; sources don't require we know the CPU clock)
	;
        xor a
	cp e
	jr z, no_timer

        out0 (TIME_TCR), a
        ld hl, #((CPU_CLOCK_KHZ * (1000/Z180_TIMER_SCALE) / TICKSPERSEC) - 1)
        out0 (TIME_RLDR0L), l
        out0 (TIME_RLDR0H), h
        ld a, #0x11         ; enable downcounting and interrupts for timer 0 only
        out0 (TIME_TCR), a

no_timer:
        ; Enable illegal instruction trap (vector at 0x0000)
        ; Enable external interrupts (INT0/INT1/INT2) 
        ld a, #0x87
        out0 (INT_ITC), a
	ret

	.area _VECTORS
        ; MUST arrange for this table to be 32-byte aligned

interrupt_table:
        .dw z180_irq1               ;     1    INT1 external interrupt
        .dw z180_irq2               ;     2    INT2 external interrupt
        .dw z180_irq3               ;     3    Timer 0
        .dw z180_irq_unused         ;     4    Timer 1
        .dw z180_irq_unused         ;     5    DMA 0
        .dw z180_irq_unused         ;     6    DMA 1
        .dw z180_irq_unused         ;     7    CSI/O
        .dw z180_irq8               ;     8    ASCI 0
        .dw z180_irq9               ;     9    ASCI 1
        .dw z180_irq_unused         ;     10
        .dw z180_irq_unused         ;     11
        .dw z180_irq_unused         ;     12
        .dw z180_irq_unused         ;     13
        .dw z180_irq_unused         ;     14
        .dw z180_irq_unused         ;     15
        .dw z180_irq_unused         ;     16

	.area _COMMONDATA

hw_irqvector:	.db 0
_irqvector:	.db 0

	.area _COMMONMEM
z80_irq:
        push af
        xor a
        jr z180_irqgo

z180_irq1:
        push af
        ld a, #1
        jr z180_irqgo

z180_irq2:
        push af
        ld a, #2
        jr z180_irqgo

z180_irq3:
        push af
        ld a, #3
        ; fall through -- timer is likely to be the most common, we'll save it the jr
z180_irqgo:
        ld (hw_irqvector), a
        ; quick and dirty way to debug which interrupt is jamming us up ...
        ;    add #0x30
        ;    .globl outchar
        ;    call outchar
        pop af
        jp interrupt_handler

; z180_irq4:
;         push af
;         ld a, #4
;         jr z180_irqgo
; 
; z180_irq5:
;         push af
;         ld a, #5 
;         jr z180_irqgo
; 
; z180_irq6:
;         push af
;         ld a, #6
;         jr z180_irqgo
; 
; z180_irq7:
;         push af
;         ld a, #7
;         jr z180_irqgo

z180_irq8:
        push af
        ld a, #8
        jr z180_irqgo

z180_irq9:
        push af
        ld a, #9
        jr z180_irqgo

; z180_irq10:
;         push af
;         ld a, #10
;         jr z180_irqgo
; 
; z180_irq11:
;         push af
;         ld a, #11
;         jr z180_irqgo
; 
; z180_irq12:
;         push af
;         ld a, #12
;         jr z180_irqgo
; 
; z180_irq13:
;         push af
;         ld a, #13
;         jr z180_irqgo
; 
; z180_irq14:
;         push af
;         ld a, #14
;         jr z180_irqgo
; 
; z180_irq15:
;         push af
;         ld a, #15
;         jr z180_irqgo
; 
; z180_irq16:
;         push af
;         ld a, #16
;         jr z180_irqgo
z180_irq_unused:
        push af
        ld a, #0xFF
        jr z180_irqgo
