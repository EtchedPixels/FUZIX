# 0 "z1013.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "z1013.S"
;
; Minimal Z1013 support
;
        ; exported symbols
        .export init_hardware
 .export _program_vectors
 .export map_kernel
 .export map_proc
 .export map_proc_always
 .export map_kernel_di
 .export map_kernel_restore
 .export map_proc_di
 .export map_proc_always_di
 .export map_save_kernel
 .export map_restore
 .export map_for_swap
 .export map_buffers
 .export plt_interrupt_all
 .export _plt_reboot
 .export _plt_monitor
 .export _bufpool
 .export _int_disabled

 ; exported debugging tools
 .export outchar

# 1 "kernelu.def" 1
; FUZIX mnemonics for memory addresses etc

U_DATA__TOTALSIZE .equ 0x200 ; 256+256 bytes @ 0x0100



PROGBASE .equ 0x6000
PROGLOAD .equ 0x6000

;
; SPI uses the top bit
;
# 28 "z1013.S" 2
# 1 "../../cpu-z80u/kernel-z80.def" 1
# 29 "z1013.S" 2

;=========================================================================
; Buffers
;=========================================================================
 .buffers
_bufpool:
        .ds 520 * 4 ; adjust NBUFS in config.h in line with this

;=========================================================================
; Initialization code
;=========================================================================
        .discard
init_hardware:
 ld hl, 64
 ld (_ramsize), hl
 ld hl, 35
 ld (_procmem), hl
 ld hl, null_handler
 ld (1),hl
 ld a, 0xC3
 ld (0),a
 jp _video_init

;=========================================================================
; Common Memory (mapped low below ROM)
;=========================================================================
 .common

_plt_monitor:
 in a,(4) ; ensure the monitor is mapped
 and 0xEF
 out (4),a
 rst 0x38 ; To monitor
 .word 0x01 ; Wait for key
_plt_reboot:
 call map_kernel ; ROM in
 jp init

;=========================================================================

_int_disabled:
 .byte 1

; install interrupt vectors
_program_vectors:
; platform fast interrupt hook
plt_interrupt_all:
 ret

;=========================================================================
; Memory management
;=========================================================================

;
 .export rom_state ; for debug

rom_state:
 .byte 1 ; ROM starts mapped
save_rom:
 .byte 0 ; For map_save/restore

;
; Centralize all control of the toggle in one place so we can debug it
;
rom_in:
 in a,(4)
 set 5,a
 out (4),a
 ld (rom_state),a
 ret

rom_out:
 in a,(4)
 res 5,a
 out (4),a
 xor a
 ld (rom_state),a
 ret


;=========================================================================
; map_proc - map process or kernel pages
; Inputs: page table address in HL, map kernel if HL == 0
; Outputs: none; A and HL destroyed
;=========================================================================
map_proc:
map_proc_di:
 ld a,h
 or l ; HL == 0?
 jr z,map_kernel ; HL == 0 - map the kernel

 ; fall through

;=========================================================================
; map_for_swap - map a page into a bank for swap I/O
; Inputs: none
; Outputs: none
;
; The caller will later map_kernel to restore normality
;
;=========================================================================
map_for_swap:

 ; fall through

;=========================================================================
; map_proc_always - map process pages
; Inputs: page table address in #2
; Outputs: none; all registers preserved
;=========================================================================
map_proc_always:
 di
 push af
 call rom_out
;
; Restore interrupt status based upon the flag (called with interrupts
; disabled). AF is currently stacked
;
irqfix:
 ld a,(_int_disabled)
 or a
 jr nz, still_di
 ei
still_di:
 pop af
 ret

map_proc_always_di:
 push af
was_u:
 call rom_out
 pop af
 ret

;=========================================================================
; map_kernel - map kernel pages
; map_buffers - map kernel and buffers (no difference for us)
; Inputs: none
; Outputs: none; all registers preserved
;=========================================================================
map_buffers:
map_kernel:
map_kernel_restore:
 push af
 di
 call rom_in
 jr irqfix

map_kernel_di:
 push af
was_k:
 call rom_in
 pop af
 ret

;=========================================================================
; map_restore - restore a saved page mapping
; Inputs: none
; Outputs: none, all registers preserved
;=========================================================================
map_restore:
 push af
 ld a,(save_rom)
 or a
 jr nz, was_k
 jr was_u

;=========================================================================
; map_save - save the current page mapping to map_savearea
; Inputs: none
; Outputs: none
;=========================================================================
map_save_kernel:
 push af
 ld a,(rom_state)
 ld (save_rom),a
 call rom_in
 pop af
 ret


;=========================================================================
; Basic console I/O
;=========================================================================

;=========================================================================
; outchar - Wait for UART TX idle, then print the char in A
; Inputs: A - character to print
; Outputs: none
;=========================================================================
outchar:
 push hl
 ld hl,(outpt)
 ld (hl),a
 inc hl
 ld (outpt),hl
 pop hl
 ret
outpt: .word 0xEC00

;
; The second set of very performance sensitive routines are accesses
; to user space. We thus provide our own modified versions of these
; for speed
;
; We put all kernel writable space and kernel data/const into the low
; block (0x0000-0x1FFF), so that when user space is mapped all
; valid kernel addresses are reachable directly.
;

        ; exported symbols
        .export __uget
        .export __ugetc
        .export __ugetw

        .export __uput
        .export __uputc
        .export __uputw
        .export __uzero

 .export map_proc_always
 .export map_kernel
;
; We need these in common as they bank switch
;
 .common

;
; The basic operations are copied from the standard one. Only the
; blk transfers are different. uputget is a bit different as we are
; not doing 8bit loop pairs.
;
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
        ; load DE with destination address (in userspace)
        ld e, (ix + 6)
        ld d, (ix + 7)
 ret ; Z is still false

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
 call map_proc_always
 ld (hl), e
uputc_out:
 jp map_kernel ; map the kernel back below common

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
 call map_proc_always
 ld (hl), e
 inc hl
 ld (hl), d
 jp map_kernel

__ugetc:
 pop de
 pop hl
 push hl
 push de
 call map_proc_always
        ld l, (hl)
 ld h, #0
 jp map_kernel

__ugetw:
 pop de
 pop hl
 push hl
 push de
 call map_proc_always
        ld a, (hl)
 inc hl
 ld h, (hl)
 ld l, a
 jp map_kernel

__uput:
 push ix
 ld ix, #0
 add ix, sp
 push bc
 call uputget ; source in HL dest in DE, count in BC
 jr z, uput_out ; but count is at this point magic
 call map_proc_always
 ldir
uput_out:
 call map_kernel
 pop bc
 pop ix
 ld hl, #0
 ret

__uget:
 push ix
 ld ix, #0
 add ix, sp
 push bc
 call uputget ; source in HL dest in DE, count in BC
 jr z, uput_out ; but count is at this point magic
 call map_proc_always
 ldir
 jr uput_out

;
__uzero:
 ld hl,2
 add hl,de
 push bc
 ld e,(hl)
 inc hl
 ld d,(hl)
 inc hl
 ld c,(hl)
 inc hl
 ld b,(hl)
 ld a, b ; check for 0 copy
 or c
 jr z, popout
 call map_proc_always
 ld h,d
 ld l,e
 ld (hl), 0
 dec bc
 ld a, b
 or c
 jr z, popout
 inc de
 ldir
popout: pop bc
 ret

 .common
;
; Character pending or 0. These routines use the ROM so we need to be
; careful they don't re-enter. keycheck is always called with
; interrupts off.
;
 .export _keycheck
;
_keycheck:
 in a,(4) ; ROM in if paged out
 and 0xEF
 out (4),a
 push bc
 push ix
 push iy
 rst 0x20
 .byte 4
 pop iy
 pop ix
 pop bc
 ld l,a
 in a,(4) ; Reverse the ROM page if supported
 or 0x10
 out (4),a
 ret

;
; Beeper (if there is a jump table)
;
 .export _do_beep

_do_beep:
 di
 push bc
 push ix
 push iy
 in a,(4) ; ROM in
 and 0xEF
 out (4),a
 call do_beep
 in a,(4)
 or #0x10
 out (4),a
 pop iy
 pop ix
 pop bc
 ld a,(_int_disabled)
 or a
 ret nz
 ei
 ret

do_beep:
 ld hl, 0xFFD6
 ld de, 0x03
 ld a, 0xC3
 cp (hl)
 ret nz
 add hl,de
 cp (hl)
 ret nz
 add hl,de
 cp (hl)
 ret nz
 ; Looks like a valid jump table
 push bc
 push ix
 push iy
 call callhl
 pop iy
 pop ix
 pop bc
 ret


callhl: jp (hl)

 .export _rd_dptr
 .export _rd_page
 .export _rd_block

_rd_dptr:
 .word 0
_rd_page:
 .byte 0
_rd_block:
 .word 0

ramconf5:
 ld hl,(_rd_block) ; Requested 512 byte block
 add hl,hl ; Turn HL into 256 byte blocks
 ld a,l
 out (0x5e),a ; Set the middle byte
 xor a
 out (0x5f),a ; Clear the counter
 ld a, 0x58 ; Work out which port to use
ramconf:
 add a,h
 ld c,a ; port
 ld a,(_rd_page) ; check if we need to page user space in
 or a
 call nz, map_proc_always
 ld b, 0 ; Set up b for the caller
 ld a,l
 inc a ; second port info
 ld hl,(_rd_dptr) ; Set up HL for the caller
 ret

ramconf9:
 ld hl,(_rd_block) ; Requested 512 byte block
 add hl,hl ; Turn HL into 256 byte blocks
 ld a,l
 out (0x9e),a ; Set the middle byte
 xor a
 out (0x9f),a ; Clear the counter
 ld a, 0x98 ; Work out which port to use
 jr ramconf

 .export _ramread5

_ramread5:
 call ramconf5
 inir
 out (0x5e),a
 inir
 ld (_rd_dptr),hl
 jp map_kernel

 .export _ramwrite5

_ramwrite5:
 call ramconf5
 otir
 out (0x5e),a
 otir
 ld (_rd_dptr),hl
 jp map_kernel

 .export _ramread9

_ramread9:
 call ramconf9
 inir
 out (0x9e),a
 inir
 ld (_rd_dptr),hl
 jp map_kernel

 .export _ramwrite9

_ramwrite9:
 call ramconf9
 otir
 out (0x9e),a
 otir
 ld (_rd_dptr),hl
 jp map_kernel

 .discard
 .export _ramdet5

_ramdet5:
 xor a ; Point to start of ram disc
 ld l,a ; Clear return
 out (0x5f),a ; Clear counter
 in a, (0x5f) ; Should read back as 0
 or a
 ret nz
 in a,(0x58) ; Read a byte
 in a,(0x5f) ; Counter now reads back as 1
 dec a
 ret nz
 inc l
 ret

 .export _ramdet9

_ramdet9:
 xor a ; Point to start of ram disc
 ld l,a ; Clear return
 out (0x9f),a ; Clear counter
 in a, (0x9f) ; Should read back as 0
 or a
 ret nz
 in a,(0x98) ; Read a byte
 in a,(0x9f) ; Counter now reads back as 1
 dec a
 ret nz
 ld l, 2
 ret

;
; Hooks for PIO provided timer tickery. We never have both CTC
; and PIO enabled.
;
 .common

 .export pio0_intr

;
; This is level triggered so we need to do a bit of work
;
pio_polarity:
 .byte 0
;
; The PIO interrupts on low or high state. We do a little bit of
; stage management to instead use it to spot one of the edges of
; the square wave input
;
pio0_intr:
 push af
 ld a,(pio_polarity)
 or a
 ; low to high or high to low ?
 jr nz, hilo
 inc a
 ; low to high edge - timer interrupt
 ld (pio_polarity),a
 ; Set the next interrupt to be high so we don't keep firing
 ld a, 0xB7
 out (0x02),a
 ld a, 0x08
 out (0x02),a
 pop af
 ; We saw a timer tick edge for our 10Hz clock (will do the reti for
 ; us)
 jp interrupt_handler

hilo:
 xor a
 ld (pio_polarity),a
 ; Set the next interrupt to be low
 ld a, 0x97
 out (0x02),a
 ld a, 0x08
 out (0x02),a
 pop af
 reti

 .common
;
; IDE disk helpers
;
 .export _devide_read_data
 .export _devide_write_data

ide_map:
 ld a,(_td_raw)
 ld bc,0x10 ; port 10, 256 times
 or a
 jp z, map_buffers
 dec a
 jp z, map_proc_always
 ld a,(_td_page)
 jp map_for_swap

_devide_read_data:
 pop de
 pop hl
 push hl
 push de
 call ide_map
 inir
 inir
 jp map_kernel_restore

_devide_write_data:
 pop de
 pop hl
 push hl
 push de
 call ide_map
 otir
 otir
 jp map_kernel_restore
