; 2014-02-19 Sergey Kiselev
; RC2014 hardware specific code

        .module rc2014

        ; exported symbols
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_process
	.globl map_kernel_di
	.globl map_process_di
	.globl map_process_always
	.globl map_process_always_di
	.globl map_save_kernel
	.globl map_restore
	.globl map_for_swap
	.globl platform_interrupt_all
	.globl _copy_common
	.globl mpgsel_cache
	.globl _kernel_pages
	.globl _platform_reboot
	.globl _bufpool
	.globl _int_disabled

        ; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl _init_hardware_c
        .globl outhl
        .globl outnewline
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl nmi_handler
	.globl null_handler
        .globl _boot_from_rom
	.globl _ser_type

	; exported debugging tools
	.globl inchar
	.globl outchar

        .include "kernel.def"
        .include "../kernel.def"

;=========================================================================
; Constants
;=========================================================================
CONSOLE_DIVISOR		.equ	(1843200 / (16 * CONSOLE_RATE))
CONSOLE_DIVISOR_HIGH	.equ	(CONSOLE_DIVISOR >> 8)
CONSOLE_DIVISOR_LOW	.equ	(CONSOLE_DIVISOR & 0xFF)

RTS_HIGH	.EQU	0xE8
RTS_LOW		.EQU	0xEA

; Base address of SIO/2 chip 0x80
; For the Scott Baker SIO card adjust the order to match rc2014.h
SIOA_C		.EQU	0x80
SIOA_D		.EQU	SIOA_D+1
SIOB_C		.EQU	SIOA_D+2
SIOB_D		.EQU	SIOA_D+3

SIO_IV          .EQU    8               ; Interrupt vector table entry to use

ACIA_C          .EQU     0x80
ACIA_D          .EQU     0x81
ACIA_RESET      .EQU     0x03
ACIA_RTS_HIGH_A      .EQU     0xD6   ; rts high, xmit interrupt disabled
ACIA_RTS_LOW_A       .EQU     0x96   ; rts low, xmit interrupt disabled
;ACIA_RTS_LOW_A       .EQU     0xB6   ; rts low, xmit interrupt enabled

;=========================================================================
; Buffers
;=========================================================================
        .area _BUFFERS
_bufpool:
        .ds (BUFSIZE * 4) ; adjust NBUFS in config.h in line with this

;=========================================================================
; Initialization code
;=========================================================================
        .area _DISCARD
init_hardware:
        ; program vectors for the kernel
        ld hl, #0
        push hl
        call _program_vectors
        pop hl

        ; Stop floppy drive motors
        ld a, #0x0C
        out (FDC_DOR), a

	; Play guess the serial port
	; This needs doing better. We might be fooled by floating flow
	; control lines as the SC104 does expose flow control. FIXME
	in a,(SIOA_C)
	and #0x2C
	cp #0x2C
	; CTS and DCD should be high as they are not wired
	jr nz, try_acia

	; Repeat the check on SIO B
	in a,(SIOB_C)
	and #0x2C
	cp #0x2C
	jr z, is_sio
try_acia:
	;
	;	Look for an ACIA
	;
	ld a,#ACIA_RESET
	out (ACIA_C),a
	; TX should now have gone
	in a,(ACIA_C)
	bit 1,a
	jr z, not_acia_either
	;	Set up the ACIA

        ld a, #ACIA_RTS_LOW_A
        out (ACIA_C),a         		; Initialise ACIA
	ld a,#2
	ld (_ser_type),a
	jp serial_up

	;
	; Doomed I say .... doomed, we're all doomed
	;
	; At least until RC2014 grows a nice keyboard/display card!
	;
not_acia_either:
	xor a
	ld (_ser_type),a
	jp serial_up
;
;	We have an SIO so do the required SIO hdance
;
is_sio:	ld a,b
	ld (_ser_type),a

	ld hl,#sio_setup
	ld bc,#0xA00 + SIOA_C		; 10 bytes to SIOA_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0A00 + SIOB_C		; and to SIOB_C
	otir

serial_up:
        ; ---------------------------------------------------------------------
	; Initialize CTC
	;
	; We must initialize all channels of the CTC. The documentation
	; states that the initial CTC state is undefined and we don't want
	; random interrupt surprises
	; ---------------------------------------------------------------------

	ld a,#0x57			; counter mode, disable interrupts
	out (CTC_CH0),a			; set CH0 mode
	ld a,#0				; time constant = 256
	out (CTC_CH0),a			; set CH0 time constant
	ld a,#0x57			; counter mode, FIXME C7 enable interrupts
	out (CTC_CH1),a			; set CH1 mode
	ld a,#180			; time constant = 180
	out (CTC_CH1),a			; set CH1 time constant
	ld a,#0x57			; counter mode, disable interrupts
	out (CTC_CH2),a			; set CH2 mode
	ld a,#0x57			; counter mode, disable interrupts
	out (CTC_CH3),a			; set CH3 mode

        ; Done CTC Stuff
        ; ---------------------------------------------------------------------

	im 1				; set Z80 CPU interrupt mode 1
        jp _init_hardware_c             ; pass control to C, which returns for us

sio_setup:
	.byte 0x00
	.byte 0x18		; Reset
	.byte 0x04
	.byte 0xC4
	.byte 0x01
	.byte 0x18
	.byte 0x03
	.byte 0xE1
	.byte 0x05
	.byte RTS_LOW

;=========================================================================
; Kernel code
;=========================================================================
        .area _CODE

_platform_reboot:
        ; We need to map the ROM back in -- ideally into every page.
        ; This little trick based on a clever suggestion from John Coffman.
        di
        ld hl, #(MPGENA << 8) | 0xD3    ; OUT (MPGENA), A
        ld (0xFFFE), hl                 ; put it at the very top of RAM
        xor a                           ; A=0
        jp 0xFFFE                       ; execute it; PC then wraps to 0


;=========================================================================
; Common Memory (0xF000 upwards)
;=========================================================================
        .area _COMMONMEM

;=========================================================================

_int_disabled:
	.db 1

platform_interrupt_all:
	ret

; install interrupt vectors
_program_vectors:
	di
	pop de				; temporarily store return address
	pop hl				; function argument -- base page number
	push hl				; put stack back as it was
	push de

	; At this point the common block has already been copied
	call map_process

	; write zeroes across all vectors
	ld hl,#0
	ld de,#1
	ld bc,#0x007f			; program first 0x80 bytes only
	ld (hl),#0x00
	ldir

	; now install the interrupt vector at 0x0038
	ld a,#0xC3			; JP instruction
	ld (0x0038),a
	ld hl,#interrupt_handler
	ld (0x0039),hl

	; set restart vector for UZI system calls
	ld (0x0030),a			; rst 30h is unix function call vector
	ld hl,#unix_syscall_entry
	ld (0x0031),hl

	ld (0x0000),a
	ld hl,#null_handler		; to Our Trap Handler
	ld (0x0001),hl

	ld (0x0066),a			; Set vector for NMI
	ld hl,#nmi_handler
	ld (0x0067),hl

	jr map_kernel

;=========================================================================
; Memory management
; - kernel pages:     32 - 34
; - common page:      35
; - user space pages: 36 - 63
;=========================================================================

;=========================================================================
; map_process_always - map process pages
; Inputs: page table address in #U_DATA__U_PAGE
; Outputs: none; all registers preserved
;=========================================================================
map_process_always:
map_process_always_di:
	push hl
	ld hl,#U_DATA__U_PAGE
        jr map_process_2_pophl_ret

;=========================================================================
; map_process - map process or kernel pages
; Inputs: page table address in HL, map kernel if HL == 0
; Outputs: none; A and HL destroyed
;=========================================================================
map_process:
map_process_di:
	ld a,h
	or l				; HL == 0?
	jr nz,map_process_2		; HL == 0 - map the kernel

;=========================================================================
; map_kernel - map kernel pages
; Inputs: none
; Outputs: none; all registers preserved
;=========================================================================
map_kernel:
map_kernel_di:
	push hl
	ld hl,#_kernel_pages
        jr map_process_2_pophl_ret

;=========================================================================
; map_process_2 - map process or kernel pages
; Inputs: page table address in HL
; Outputs: none, HL destroyed
;=========================================================================
map_process_2:
	push de
	push af
	ld de,#mpgsel_cache		; paging registers are write only
					; so cache their content in RAM
	ld a,(hl)			; memory page number for bank #0
	ld (de),a
	out (MPGSEL_0),a		; set bank #0
	inc hl
	inc de
	ld a,(hl)			; memory page number for bank #1
	ld (de),a
	out (MPGSEL_1),a		; set bank #1
	inc hl
	inc de
	ld a,(hl)			; memory page number for bank #2
	ld (de),a
	out (MPGSEL_2),a		; set bank #2
	pop af
	pop de
	ret

;=========================================================================
; map_restore - restore a saved page mapping
; Inputs: none
; Outputs: none, all registers preserved
;=========================================================================
map_restore:
	push hl
	ld hl,#map_savearea
map_process_2_pophl_ret:
	call map_process_2
	pop hl
	ret

;=========================================================================
; map_save_kernel - save the current page mapping to map_savearea and
; switch to kernel maps
; Inputs: none
; Outputs: none
;=========================================================================
map_save_kernel:
	push hl
	ld hl,(mpgsel_cache)
	ld (map_savearea),hl
	ld hl,(mpgsel_cache+2)
	ld (map_savearea+2),hl
	ld hl,#_kernel_pages
	jr map_process_2_pophl_ret

;=========================================================================
; map_for_swap - map a page into a bank for swap I/O
; Inputs: none
; Outputs: none
;
; The caller will later map_kernel to restore normality
;
; We use 0x4000-0x7FFF so that all the interrupt stuff is mapped.
;
;=========================================================================
map_for_swap:
	ld (mpgsel_cache + 1),a
	out (MPGSEL_1),a
	ret

_copy_common:
	pop hl
	pop de
	push de
	push hl
	ld a,e
	call map_for_swap
	ld hl,#0xF300
	ld de,#0x7300
	ld bc,#0x0D00
	ldir
	jr map_kernel


; MPGSEL registers are read only, so their content is cached here
mpgsel_cache:
	.db	0,0,0,0

; kernel page mapping
_kernel_pages:
	.db	32,33,34,35

; memory page mapping save area for map_save/map_restore
map_savearea:
	.db	0,0,0,0

;=========================================================================
; Basic console I/O
;=========================================================================

;=========================================================================
; outchar - Wait for UART TX idle, then print the char in A
; Inputs: A - character to print
; Outputs: none
;=========================================================================
outchar:

	push af
	ld a,(_ser_type)
	cp #2
	jr z, ocloop_acia

	; wait for transmitter to be idle
ocloop_sio:
        xor a                   ; read register 0
        out (SIOA_C), a
	in a,(SIOA_C)		; read Line Status Register
	and #0x04			; get THRE bit
	jr z,ocloop_sio
	; now output the char to serial port
	pop af
	out (SIOA_D),a
	jr out_done

	; wait for transmitter to be idle
ocloop_acia:
	in a,(ACIA_C)		; read Line Status Register
	and #0x02			; get THRE bit
	jr z,ocloop_acia
	; now output the char to serial port
	pop af
	out (ACIA_D),a
out_done:
        out (VFD_D),a
	ret

;=========================================================================
; inchar - Wait for character on UART, return in A
; Inputs: none
; Outputs: A - received character, F destroyed
;=========================================================================
inchar:
	ld a,(_ser_type)
	cp #2
	jr z,inchar_acia
inchar_s:
        xor a                           ; read register 0
        out (SIOA_C), a
	in a,(SIOA_C)   		; read Line Status Register
	and #0x01			; test if data is in receive buffer
	jr z,inchar_s			; no data, wait
	in a,(SIOA_D)   		; read the character from the UART
	ret
inchar_acia:
	in a,(ACIA_C)   		; read Line Status Register
	and #0x01			; test if data is in receive buffer
	jr z,inchar_acia		; no data, wait
	in a,(ACIA_D)   		; read the character from the UART
	ret
