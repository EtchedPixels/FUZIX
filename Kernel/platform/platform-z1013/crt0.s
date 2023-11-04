# 0 "crt0.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "crt0.S"
# 1 "kernelu.def" 1
; FUZIX mnemonics for memory addresses etc

U_DATA__TOTALSIZE .equ 0x200 ; 256+256 bytes @ 0x0100



PROGBASE .equ 0x6000
PROGLOAD .equ 0x6000

;
; SPI uses the top bit
;
# 2 "crt0.S" 2

 .abs
 .org 0x5FF0

__vectors:
 ; Starts at 0x5FF0

 ; Spot for interrupt vectors
 .word pio0_intr ; pio A
 .word interrupt_handler ; ctc channel 1

 .export init

 .code

init:
        di
 ld sp, 0x8000 ; safe spot

 in a,(4) ; turn on 4MHz and work around bug? in JKCEMU
 or 0x60 ; (wrong in a,(4) if boot with EPROM mapped)
 out (4),a

 im 2
 ld a, >__vectors
 ld i,a

 ; Clear the screen
 ld hl, 0xEC00
 ld de, 0xEC01
 ld bc, 0x03FF
 ld (hl), ' '
 ldir
 ld ix, 0xEC00

 ; Load and map the rest of the image
 ld d, 3 ; FIXME
 ld bc, 0x48
 ld hl, 0x0100 ; Load 0100 upwards
 xor a

 call init_gide
loader:
 ld (ix + 0), '='
 inc ix
 call load_sector
 inc d
 bit 6,d ; load 2 - 63
 jr z, loader

        ; switch to stack in common memory
        ld sp, kstack_top

        ; Zero the data area
        ld hl, __bss
        ld de, __bss + 1
        ld bc, __bss_size - 1
        ld (hl), 0
        ldir

 in a,(4) ; if possible map out the system ROM
 or 0x10 ; and video memory
 out (4),a

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main

        ; fuzix_main() shouldn't return, but if it does...
        di
stop: halt
        jr stop

;
; Load sector d from disk into HL and advance HL accordingly
;
load_sector:
 ld a,d
 out (0x4B),a ; LBA / sector
 ld a, 1
 out (0x4A),a ; 1 sector
 ld a, 0x20
 out (0x4F),a ; command
 ; Wait
wait_drq:
 in a,(0x4F)
 bit 3,a
 jr z, wait_drq
 ; Get data, leave HL pointing to next byte, leaves B as 0 again
 inir
 inir
 ret
wait_ready:
 in a,(0x4F)
 bit 6,a
 jr z,wait_ready
 ret

init_gide:
 ld a, 0xE0
 out (0x4E),a
 xor a
 out (0x4C),a
 out (0x4D),a
 ret
