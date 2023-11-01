# 0 "crt0.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "crt0.S"
# 1 "kernelu.def" 1
; FUZIX mnemonics for memory addresses etc

U_DATA__TOTALSIZE .equ 0x200 ; 256+256 bytes @ 0xC000
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
CTC_CH1 .equ 0x89 ; CTC channel 1 (serial B)
CTC_CH2 .equ 0x8A ; CTC channel 2 (timer)
CTC_CH3 .equ 0x8B ; CTC channel 3 (timer count)
# 2 "crt0.S" 2

 .abs

.org 0

restart0:
 jp init
 .ds 5
restart8:
 .ds 8
restart10:
 .ds 8
restart18:
 .ds 8
restart20:
 .ds 8
restart28:
 .ds 8
restart30:
 .ds 8
restart38:
 jp interrupt_handler
 .ds 5
; 0x40
 .ds 26
; 0x66
 jp nmi_handler
;
; And 0x69 onwards we could use for code
;

 ; Starts at 0x0080 at the moment

 .code

init:
        di
 ld sp, 0xFFFF ; safe spot

 ; Init the ATA CF
 ; For now this is fairly dumb.
 ; We ought to init the UART here first so we can say something
 ; before loading.
 ld a, 0xE0
 out (0x16),a
 xor a
 out (0x14),a
 out (0x15),a
 ; Set 8bit mode
 call wait_ready
 ld a, 1 ; 8bit PIO
 out (0x11),a
 ld a, 0xEF ; SET FEATURES (8bit PIO)
 out (0x17),a
 call wait_ready

 ; Load and map the rest of the image
 ld d, 1
 ld bc, 0x10 ; c = data port b = 0
 ld hl, 0x8200 ; Load 8200-FFFF
loader:
 inc d
 call load_sector
 bit 6,d ; load 64 sectors 2-66
 jr z, loader

        ; switch to stack in high memory
        ld sp, kstack_top

        ; Zero the data area
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

;
; Load sector d from disk into HL and advance HL accordingly
;
load_sector:
 ld a,d
 out (0x13),a ; LBA
 ld a, 1
 out (0x12),a ; 1 sector
 ld a, 0x20
 out (0x17),a ; command
 ; Wait
wait_drq:
 in a,(0x17)
 bit 3,a
 jr z, wait_drq
 ; Get data, leave HL pointing to next byte, leaves B as 0 again
 inir
 inir
 ret
wait_ready:
 in a,(0x17)
 bit 6,a
 jr z,wait_ready
 ret
