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
CTC_CH0 .equ 0xD0 ; CTC channel 0 and interrupt vector
CTC_CH1 .equ 0xD1 ; CTC channel 1 (periodic interrupts)
CTC_CH2 .equ 0xD2 ; CTC channel 2
CTC_CH3 .equ 0xD3 ; CTC channel 3

NBUFS .equ 4

;
; Select where to put the high code - in our case we need this
; in common
;


HIGHPAGE .equ 0 ; We only have 1 page byte and the low page
    ; isn't used
# 7 "usermem.S" 2
# 1 "../../cpu-z80u/kernel-z80.def" 1
# 8 "usermem.S" 2

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
 ld ix,0
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
 ld hl,#0
 ret

__uput:
 push ix
 ld ix,0
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
 ld hl,5
 add hl,sp
 ld d,(hl)
 dec hl
 ld e,(hl)
 dec hl
 ld a,(hl)
 dec hl
 ld l,(hl)
 ld h,a
 ; DE is count HL is pointer
 ld a,d
 or e
 ret z ; no work

 push bc
 di
 ld bc,(sio_reg)
 out (c),b ; Port B WR 1
 ld a,(bits_to_user+1)
 out (c),a ; port B WR 1 to 0x58
 ld (hl),0
 ld a,(_int_disabled)
 or a
 jr nz, noteiz
 ei
noteiz:
 ld b,d
 ld c,e
 dec bc
 ld a,b
 or c
 jr z, uout
 ld e,l
 ld d,h
 inc de
 ldir
uout:
 di
uoutd:
 ld bc,(sio_reg) ; port B WR 1
 out (c),b
 ld a,(bits_to_user)
 out (c),a ; back to kernel
 pop bc
 ld a,(_int_disabled)
 or a
 ret nz
 ei
 ret

__ugetc:
 pop de
 pop hl
 push hl
 push de
 push bc
 di
 ld bc,(sio_reg)
 out (c),b
 ld a,(bits_to_user+1)
 out (c),a
 ld l,(hl)
 ld h,0
 jr uoutd

__ugetw:
 pop de
 pop hl
 push hl
 push de
 push bc
 di
 ld bc,(sio_reg)
 out (c),b
 ld a,(bits_to_user+1)
 out (c),a
 ld a,(hl)
 inc hl
 ld h,(hl)
 ld l,a
 jr uoutd

__uputc:
 ld hl,2
 add hl,sp
 ld e,(hl)
 inc hl
 inc hl
 ld a,(hl)
 inc hl
 ld h,(hl)
 ld l,a
 push bc
 di
 ld bc,(sio_reg)
 out (c),b
 ld a,(bits_to_user+1)
 out (c),a
 ld (hl),e
 jr uoutd

__uputw:
 ld hl,2
 add hl,sp
 ld e,(hl)
 inc hl
 ld d,(hl)
 inc hl
 ld a,(hl)
 inc hl
 ld h,(hl)
 ld l,a
 push bc
 di
 ld bc,(sio_reg)
 out (c),b
 ld a,(bits_to_user+1)
 out (c),a
 ld (hl),e
 inc hl
 ld (hl),d
 jr uoutd
