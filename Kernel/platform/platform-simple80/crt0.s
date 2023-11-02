# 0 "crt0.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "crt0.S"
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
# 2 "crt0.S" 2

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
