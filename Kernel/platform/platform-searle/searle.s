# 0 "searle.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "searle.S"
;
; Grant Searle SBC with timer hack and A16 wired for SIO control
; Also requires the revised boot rom that places the initial bank flip
; helper at 0xFFxx in both banks before the ROM is paged out (as it
; can't be paged back in).
;

        ; exported symbols
        .export init_hardware
        .export _program_vectors
 .export _kernel_flag
 .export map_page_low
 .export map_kernel_low
 .export map_user_low
 .export map_save_low
 .export map_restore_low
 .export _plt_doexec
 .export _plt_reboot
 .export _int_disabled
 .export syscall_platform

        ; exported debugging tools
        .export _plt_monitor
        .export outchar

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
# 27 "searle.S" 2
# 1 "../../cpu-z80u/kernel-z80.def" 1
# 28 "searle.S" 2


; Base address of SIO/2 chip 0x00

SIOA_D equ 0x00
SIOB_D equ 0x01
SIOA_C equ 0x02
SIOB_C equ 0x03



;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

 .export _bufpool

 .buffers
_bufpool:
 .ds 520 * NBUFS

; -----------------------------------------------------------------------------
;
; Because of the weird memory model this is a bit different to the
; usual Z80 setup.
;
; -----------------------------------------------------------------------------
 .common

_plt_monitor:
_plt_reboot:
 di ; No way back to the ROM
 halt

_int_disabled:
 .byte 1

; -----------------------------------------------------------------------------
;
; Our MMU is write only, but if we put a value in each bank in a fixed
; place we have a fast way to see which bank is which
;
; -----------------------------------------------------------------------------

banknum:
 .byte 0x01 ; copied into far bank then set to 0

; -----------------------------------------------------------------------------
; All of discard gets reclaimed when init is run
;
; Discard must be above 0x8000 as we need some of it when the ROM
; is paged in during init_hardware
; -----------------------------------------------------------------------------
 .discard

init_hardware:
 ld hl, sio_setup
 ld bc, 0x0A00 + SIOA_C ; 10 bytes to SIOA_C
 otir

 ld hl, sio_setup
 ld bc, 0x0A00 + SIOB_C ; and to SIOB_C
 otir

 ld hl, #128
        ld (_ramsize), hl
 ld hl, 60 ; We lose 4K to common space
        ld (_procmem), hl

 ld hl, bankhelper
 call outstring

 ld hl, __common
 ld de, __common_size

 ; This is run once so can be slow and simple
farput:
 ld a,(hl)
 call 0xFF00 ; put byte in a into (HL) in far memory
    ; uses BC/HL/AF
 inc hl
 dec de
 ld a,d
 or e
 jr nz,farput


 xor a
 ld (banknum),a ; and correct page

 ; We now have our common in place. We can do the rest ourselves

 ; Put the low stubs into place in the kernel
 ld hl, stubs_low
 ld de, 0
 ld bc, 0x68
 ldir
 ld hl, stubs_low
 ld ix, 0
 ld bc, 0x68
 call ldir_to_user

 im 1 ; set Z80 CPU interrupt mode 1

 ret

bankhelper:
 .ascii 'Invoking bank helper'
 .byte 13,10
 .byte 0


RTS_LOW equ 0xEA

sio_setup:
 .byte 0x00
 .byte 0x18 ; Reset
 .byte 0x04
 .byte 0xC4
 .byte 0x01
 .byte 0x19 ; We want carrier events
 .byte 0x03
 .byte 0xE1
 .byte 0x05
 .byte RTS_LOW


;
; Our memory setup is weird and common is kind of meaningless here
;
 .code
_kernel_flag:
 .byte 1 ; We start in kernel mode
map_save_low:
map_kernel_low:
map_restore_low:
map_user_low:
map_page_low:
 ret

_program_vectors:
 ret

 .export _plt_idle

_plt_idle:
 halt
 ret
;
; A little SIO helper
;
 .export _sio2_otir

_sio2_otir:
 pop de
 pop hl
 push hl
 push de
 push bc
 ld b, 0x06
 ld c,l
 ld hl, _sio_r
 otir
 pop bc
 ret

 .common
;
; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
;
outchar:
 push af
 ; wait for transmitter to be idle
ocloop_sio:
        xor a ; read register 0
        out (SIOA_C), a
 in a,(SIOA_C) ; read Line Status Register
 and #0x04 ; get THRE bit
 jr z,ocloop_sio
 ; now output the char to serial port
 pop af
 out (SIOA_D),a
 ret


; Don't be tempted to put the symbol in the code below ..it's relocated
; to zero. Instead define where it ends up.

 .abs
 .org 0x18
_plt_doexec:

 .discard

 .export rst38
 .export stubs_low
; This exists at the bottom of each page. We move these into place
; from discard.
;
stubs_low:
 .byte 0
stub0: .word 0 ; cp/m emu changes this
 .byte 0 ; cp/m emu I/O byte
 .byte 0 ; cp/m emu drive and user
 jp 0 ; cp/m emu bdos entry point
rst8:
 ret
 nop
 nop
 nop
 nop
 nop
 nop
 nop
rst10:
 ret
 nop
 nop
 nop
 nop
 nop
 nop
 nop
rst18:
 ld a, 0x01
 out (0x03),a
 ld a, 0x58
 out (0x03),a
rst20: ei
 jp (hl)
 nop
 nop
 nop
 nop
 nop
 nop
rst28: ret
 nop
 nop
 nop
 nop
 nop
 nop
 nop
rst30: jp syscall_platform
 nop
 nop
 nop
 nop
 nop
;
; We only have 38-4F available for this in low space
;
rst38: jp interrupt_high ; Interrupt handling stub
 nop
 nop
 nop
 nop
 nop
 .ds 0x26
nmi_handler: ; Should be at 0x66
 retn

;
; This stuff needs to live somewhere, anywhere out of the way (so we
; use common). We need to copy it to the same address on both banks
; so place it in common as we will put common into both banks
;

 .common

 .export ldir_to_user
 .export ldir_from_user

ldir_to_user:
 ld de, 0x1858 ; from bank 0 to bank 1
ldir_far:
 push bc
 ld bc, 0x0103
 exx
 pop bc ; get BC into alt bank
 di ; annoyingly : FIXME
far_ldir_1:
 exx
 out (c),b
 out (c),d
 ld a,(hl)
 inc hl
 out (c),b
 out (c),e
 ld (ix + 0),a
 inc ix
 exx
 dec bc
 ld a,b
 or c
 jr nz, far_ldir_1
 exx
 out (c),b
 ld b, 0x18
 out (c),b
 exx
 ret
ldir_from_user:
 ld de, 0x5818
 jr ldir_far
;
; High stubs. Present in each bank in the top 256 bytes
;
interrupt_high:
 push af
 push de
 push hl
 ex af,af'
 push af
 push bc
 exx
 push bc
 push de
 push hl
 push ix
 push iy
 ld a,(banknum)
 ld c,a
 ;
 ; Switch to kernel
 ;
 ld a, 0x01
 out (0x03),a
 ld a, 0x18
 out (0x03),a
 ld (istack_switched_sp),sp ; istack is positioned to be valid
 ld sp, istack_top ; in both banks. We just have to
 ;
 ; interrupt_handler may come back on a different stack in
 ; which case bc is junk. Fortuntely we never pre-empt in
 ; kernel so the case we care about bc is always safe. This is
 ; not a good way to write code and should be fixed! FIXME
 ;
 push bc
 call interrupt_handler ; switch on the right SP
 pop bc
 ; Restore stack pointer to user. This leaves us with an invalid
 ; stack pointer if called from user but interrupts are off anyway
 ld sp,(istack_switched_sp)
 ; On return HL = signal vector E= signal (if any) A = page for
 ; high
 or a
 jr z, kernout
 ; Returning to user space
 ld a, 0x01 ; User bank
 out (0x03),a
 ld a, 0x58
 out (0x03),a
 ; User stack is now valid
 ; back on user stack
 xor a
 cp e
 call nz, sigpath
pops:
 ex af,af'
 exx
 pop iy
 pop ix
 pop hl
 pop de
 pop bc
 exx
 pop bc
 pop af
 ex af,af'
 pop hl
 pop de
 pop af
 ei
 ret
kernout:
 ; restore bank - if we interrupt mid user copy or similar we
 ; have to put the right bank back
 ld a, 1
 out (0x03),a
 ld a,c
 out (0x03),a
 jr pops

sigpath:
 push de ; signal number
 ld de, irqsigret
 push de ; clean up
 ex de,hl
 ld hl,(PROGLOAD+16) ; helper vector
 jp (hl)
irqsigret:
 inc sp ; drop signal number
 inc sp
 ret

syscall_platform:
 push ix
 ld ix, 0
 add ix,sp
 push bc
 ld c,(ix + 6)
 ld b,(ix + 7)
 ld e,(ix + 8)
 ld d,(ix + 9)
 ld l,(ix + 10)
 ld h,(ix + 11)
 push hl
 ld l,(ix + 12)
 ld h,(ix + 13)
 pop ix
 di
 ; BUG: syscall corrupts AF' - should we just define some
 ; alt register corruptors for new API - would be sanest fix
 ex af, af'		; Ick - find a better way to do this bit !
 ld a, 1
 out (0x03),a
 ld a, 0x18
 out (0x03),a
 ex af,af'
 ; Stack now invalid
 ld (_udata + 8),sp
 ld sp, kstack_top
 call unix_syscall_entry
 ; FIXME check di rules
 ; stack now invalid. Grab the new sp before we unbank the
 ; memory holding it
 ld sp,(_udata + 8)
 ld a, 0x01 ; back to the user page
 out (0x03),a
 ld a, 0x58
 out (0x03),a
 xor a
 cp h
 call nz, syscall_sigret
 ; FIXME for now do the grungy C flag HL DE stuff from
 ; lowlevel-z80 until we fix the ABI
 pop bc
 ld a,h
 or l
 jr nz, error
 ex de,hl
 pop ix
 ei
 ret
error: scf
 pop ix
 ei
 ret
syscall_sigret:
 ld a,l ; DEBUG
 push hl ; save errno
 push de ; save retval
 ld l,h
 ld h, 0
 push hl ; signal
 ld hl, syscall_sighelp
 push hl ; vector
 push bc
 ret
syscall_sighelp:
 pop de ; discard signal
 pop de ; recover error info
 pop hl
 ld h, 0 ; clear signal bit
 ret
