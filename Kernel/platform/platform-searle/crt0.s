# 0 "crt0.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "crt0.S"
; 2015-02-20 Sergey Kiselev
; 2013-12-18 William R Sowerbutts

# 1 "kernelu.def" 1
; FUZIX mnemonics for memory addresses etc

U_DATA__TOTALSIZE .equ 0x200 ; 256+256 bytes @ F000
Z80_TYPE .equ 0 ; CMOS

Z80_MMU_HOOKS .equ 0



PROGBASE .equ 0x0000
PROGLOAD .equ 0x0100

NBUFS .equ 4

;
; Select where to put the high code - in our case we need this
; in common
;


HIGHPAGE .equ 0 ; We only have 1 page byte and the low page
    ; isn't used
# 5 "crt0.S" 2

        ; startup code (0x100)
 .code
init:
        di
        ; switch to stack in high memory
        ld sp, kstack_top

        ; move the common memory where it belongs
        ld hl, __bss
        ld de, __common
        ld bc, __common_size
        ldir
        ; and the discard
        ld de, __discard
        ld bc, __discard_size
        ldir

        ld hl, __bss
        ld de, __bss + 1
        ld bc, __bss_size - 1
        ld (hl), 0
        ldir

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main

        ; fuzix_main() shouldn't return, but if it does...
        di
stop: halt
        jr stop
