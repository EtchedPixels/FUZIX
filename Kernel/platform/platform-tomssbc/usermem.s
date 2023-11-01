# 0 "usermem.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "usermem.S"
;
; We have a custom implementation of usermem. We really need
; to optimize ldir_to/from_user.
;
# 1 "kernelu.def" 1
; FUZIX mnemonics for memory addresses etc

U_DATA__TOTALSIZE .equ 0x200 ; 256+256 bytes @ F000
Z80_TYPE .equ 0 ; CMOS

Z80_MMU_HOOKS .equ 0

CONFIG_SWAP .equ 1

PROGBASE .equ 0x0000
PROGLOAD .equ 0x0100

; Mnemonics for I/O ports etc

CONSOLE_RATE .equ 115200

CPU_CLOCK_KHZ .equ 7372

; Z80 CTC ports
CTC_CH0 .equ 0x88 ; CTC channel 0 and interrupt vector
CTC_CH1 .equ 0x89 ; CTC channel 1 (periodic interrupts)
CTC_CH2 .equ 0x8A ; CTC channel 2
CTC_CH3 .equ 0x8B ; CTC channel 3

ACIA_C .equ 0x80
ACIA_D .equ 0x81
ACIA_RESET .equ 0x03
ACIA_RTS_HIGH_A .equ 0xD6 ; rts high, xmit interrupt disabled
ACIA_RTS_LOW_A .equ 0x96 ; rts low, xmit interrupt disabled

NBUFS .equ 4

;
; Select where to put the high code - in our case we need this
; in common
;


HIGHPAGE .equ 0 ; We only have 1 page byte and the low page
    ; isn't used
# 6 "usermem.S" 2
# 1 "../../cpu-z80u/kernel-z80.def" 1
# 7 "usermem.S" 2

        ; exported symbols
        .export __uget
        .export __ugetc
        .export __ugetw

        .export __uput
        .export __uputc
        .export __uputw
        .export __uzero

 .common

uputget:
        ; load DE with the byte count
        ld c, (ix + 8) ; byte count
        ld b, (ix + 9)
 ld a, b
 or c
 ret z ; no work
        ; load HL with the source address
        ld l, (ix + 4) ; src address
        ld h, (ix + 5)
        ; load DE with destination address
        ld e, (ix + 6)
        ld d, (ix + 7)
 ret

__uget:
 push ix
 ld ix, 0
 add ix, sp
 push bc
 call uputget
 jr z, uget_out
 push de
 pop ix
 call ldir_from_user
uget_out:
 pop bc
 pop ix
 ld hl, 0
 ret

__uput:
 push ix
 ld ix, 0
 add ix,sp
 push bc
 call uputget
 jr z, uget_out
 push de
 pop ix
 call ldir_to_user
 jr uget_out

;
; The kernel IRQ code will restore the bank if it interrupts this
; logic
;
__uzero:
 ld hl,#2
 add hl,sp
 push bc
 ld e,(hl)
 inc hl
 ld d,(hl)
 inc hl
 ld c,(hl)
 inc hl
 ld b, (hl)
 ld a,b
 or c
 jr z, done
 ld l,e
 ld h,d
 ld a, 0x01
 out (0x3E),a ; user bank
 ld (hl), 0
 dec bc
 ld a,b
 or c
 jr z, uout
 inc de
 ldir
uout:
 xor a
 out (0x3E),a
done:
 pop bc
 ret

__ugetc:
 pop de
 pop hl
 push hl
 push de
 ld a, 0x01
 out (0x3E),a
 ld l,(hl)
 ld h, 0
ugout:
 xor a
 out (0x3E),a
 ret

__ugetw:
 pop de
 pop hl
 push hl
 push de
 ld a, 0x1
 out (0x3E),a
 ld a,(hl)
 inc hl
 ld h,(hl)
 ld l,a
 jr ugout

__uputc:
 ld hl, 2
 add hl,sp
 ld a,(hl)
 inc hl
 inc hl
 ld e,(hl)
 inc hl
 ld d,(hl)
 ld l,a
 ld a, 0x01
 out (0x3E),a
 ex de,hl
 ld (hl), e
 ld hl,0
 jr ugout

__uputw:
 ld hl, 2
 add hl,sp
 ld e,(hl)
 inc hl
 ld d,(hl)
 inc hl
 ld a,(hl)
 inc hl
 ld h,(hl)
 ld l,a
 ld a, 0x01
 out (0x3E),a
 ld (hl),e
 inc hl
 ld (hl),d
 ld hl,0
 jr ugout
