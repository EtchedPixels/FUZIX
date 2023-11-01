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
 ld hl,0
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
 ld hl,2
 add hl,sp
 push bc
 ld e,(hl)
 inc hl
 ld d,(hl)
 inc hl
 ld c,(hl)
 inc hl
 ld b,(hl)
 ld a,b
 or c
 jr z,zout
 di
 ld a,1
 out (0x03),a ; Port B WR 1
 ld a,0x58
 out (0x03),a ; port B WR 1 to 0x58
 ld l,e
 ld h,d
 ld (hl),0
 ld a,(_int_disabled)
 or a
 jr nz, noteiz
 ei
noteiz:
 dec bc
 ld a,b
 or c
 jr z, uout
 inc de
 ldir
uout:
 di
 ld a,1 ; port B WR 1
 out (0x03),a
 ld a,0x18
 out (0x03),a ; back to kernel
zout:
 pop bc
 ld a,(_int_disabled)
 or a
 ret nz
 ei
 ret

__ugetc:
 push bc
 di
 ld a,1
 out (0x03),a
 ld a,0x58
 out (0x03),a
 ld l,(hl)
 ld h,0
 jr uout

__ugetw:
 push bc
 di
 ld a,1
 out (0x03),a
 ld a,0x58
 out (0x03),a
 ld a,(hl)
 inc hl
 ld h,(hl)
 ld l,a
 jr uout

__uputc:
 ld hl,2
 add hl,sp
 push bc
 ld c,(hl)
 inc hl
 inc hl
 ld e,(hl)
 inc hl
 ld d,(hl)
 ex de,hl
 di
 ld a,1
 out (0x03),a
 ld a,0x58
 out (0x03),a
 ld (hl),c
 ld hl,0
 jr uout

__uputw:
 ld hl,2
 add hl,sp
 push bc
 ld c,(hl)
 inc hl
 ld b,(hl)
 inc hl
 ld e,(hl)
 inc hl
 ld d,(hl)
 ex de,hl
 ld e,a
 di
 ld a,1
 out (0x03),a
 ld a,0x58
 out (0x03),a
 ld (hl),c
 inc hl
 ld (hl),b
 ld hl,0
 jr uout
