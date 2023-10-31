# 0 "../../dev/ds1302_rbc.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "../../dev/ds1302_rbc.S"
 .code

PIN_CE .equ 0x10
PIN_DATA_HIZ .equ 0x20
PIN_CLK .equ 0x40
PIN_DATA_OUT .equ 0x80
PIN_DATA_IN .equ 0x01
PIN_OTHER .equ 0x00


# 1 "../../dev/ds1302_commonu.s" 1
; 2015-02-19 Sergey Kiselev
; 2014-12-31 William R Sowerbutts
; N8VEM SBC / Zeta SBC / RC2014 DS1302 real time clock interface code
;
;
        ; exported symbols
        .export _ds1302_set_ce
        .export _ds1302_set_clk
        .export _ds1302_set_data
        .export _ds1302_set_driven
        .export _ds1302_get_data

# 1 "../../dev/../build/kernel.def" 1
; FUZIX mnemonics for memory addresses etc
;
;
; The U_DATA address. If we are doing a normal build this is the start
; of common memory. We do actually have a symbol for udata so
; eventually this needs to go away
;
U_DATA__TOTALSIZE .equ 0x200 ; 256+256 bytes @ F000
;
; Space for the udata of a switched out process within the bank of
; memory that it uses. Normally placed at the very top
;
U_DATA_STASH .equ 0x7E00 ; 7E00-7FFF
;
; Z80 systems start program space at 0, and load at 0x100 so that the
; low 256 bytes are free for syscall vectors and the like, with some
; also used as a special case by the CP/M emulator.
;
PROGBASE .equ 0x0000
PROGLOAD .equ 0x0100
;
; CPU type
; 0 = CMOS Z80
; 1 = NMOS Z80 (also works with CMOS)
; 2 = Z180
;
; If either NMOS or CMOS may be present pick NMOS as the NMOS build
; contains extra code to work around an erratum n the NUMS Z80
;
Z80_TYPE .equ 1 ; NMOS (IRQ bugs) Z80
;
; For special platforms that have external memory protection hardware
; Just say 0.
;
Z80_MMU_HOOKS .equ 0
;
; Set this if the platform has swap enabled in config.h
;

;
; The number of disk buffers. Must match config.h
;
NBUFS .equ 5
;
; Configuration for the Zeta floppy controller. Specific to that
; driver rather than generic.
;
; FDC9266 floppy controller ports
FDC_CCR .equ 0 ; No CCR
FDC_MSR .equ 0x36 ; 8272 Main Status Register (R/O)
FDC_DATA .equ 0x37 ; 8272 Data Port (R/W)
FDC_DOR .equ 0x38 ; Digital Output Register (W/O)

CPU_CLOCK_KHZ .equ 8000 ; 8MHz is usual top

;
; Values for the PPI port
;
ppi_port_a .equ 0x60
ppi_port_b .equ 0x61
ppi_port_c .equ 0x62
ppi_control .equ 0x63

PPIDE_CS0_LINE .equ 0x08
PPIDE_CS1_LINE .equ 0x10
PPIDE_WR_LINE .equ 0x20
PPIDE_RD_LINE .equ 0x40
PPIDE_RST_LINE .equ 0x80

PPIDE_PPI_BUS_READ .equ 0x92
PPIDE_PPI_BUS_WRITE .equ 0x80

ppide_data .equ PPIDE_CS0_LINE
# 14 "../../dev/ds1302_commonu.s" 2
# 1 "../../dev/../cpu-z80u/kernel-z80.def" 1
# 15 "../../dev/ds1302_commonu.s" 2

; -----------------------------------------------------------------------------
; DS1302 interface
; -----------------------------------------------------------------------------

_ds1302_get_data:
 ld bc,(_rtc_port)
        in a, (c) ; read input register
        and #PIN_DATA_IN ; mask off data pin
        ld l, a ; return result in L
        ret

_ds1302_set_driven:
 pop de
 pop hl
 push hl
 push de
 push bc
        ld a, (_rtc_shadow)
        and >PIN_DATA_HIZ ; 0 - output pin
        bit 0, l ; test bit
        jr nz, writereg
        or <PIN_DATA_HIZ
        jr writereg

_ds1302_set_data:
 pop de
 pop hl
 push hl
 push de
 push bc
        ld bc, PIN_DATA_OUT
        jr setpin

_ds1302_set_ce:
 pop de
 pop hl
 push hl
 push de
 push bc
        ld bc, PIN_CE
        jr setpin

_ds1302_set_clk:
 pop de
 pop hl
 push hl
 push de
 push bc
        ld bc, PIN_CLK
        jr setpin

setpin:
        ld a, (_rtc_shadow) ; load current register contents
        and b ; unset the pin
        bit 0, l ; test bit
        jr z, writereg ; arg is false
        or c ; arg is true
writereg:
 ld bc, (_rtc_port)
        out (c), a ; write out new register contents
        ld (_rtc_shadow), a ; update our shadow copy
 pop bc
        ret
# 12 "../../dev/ds1302_rbc.S" 2
