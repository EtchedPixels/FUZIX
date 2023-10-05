; 2014-02-19 Sergey Kiselev
; RC2014 hardware specific code

        .module rc2014

        ; exported symbols
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_kernel_restore
	.globl map_proc
	.globl map_buffers
	.globl map_buffers_user
	.globl map_buffers_user_h
	.globl map_kernel_di
	.globl map_proc_di
	.globl map_proc_always
	.globl map_proc_always_di
	.globl map_save_kernel
	.globl map_restore
	.globl map_for_swap
	.globl plt_interrupt_all
	.globl _copy_common
	.globl mpgsel_cache
	.globl top_bank
	.globl _kernel_pages
	.globl _plt_reboot
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
	.globl _udata
	.globl istack_top
	.globl ___hard_di
	.globl ___sdcc_enter_ix

	; exported debugging tools
	.globl outchar
	.globl inchar

        .include "kernel.def"
        .include "../../cpu-z80/kernel-z80.def"

;=========================================================================
; Constants
;=========================================================================
CONSOLE_DIVISOR		.equ	(1843200 / (16 * CONSOLE_RATE))
CONSOLE_DIVISOR_HIGH	.equ	(CONSOLE_DIVISOR >> 8)
CONSOLE_DIVISOR_LOW	.equ	(CONSOLE_DIVISOR & 0xFF)

RTS_HIGH	.EQU	0xE8
RTS_LOW		.EQU	0xEA

; Base address of SIO/2 chip 0x80. On board SIO has D/C backwards to RC2014!

SIOA_D		.EQU	0x80
SIOA_C		.EQU	SIOA_D+1
SIOB_D		.EQU	SIOA_D+2
SIOB_C		.EQU	SIOA_D+3

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

	; RST shorteners
	ld hl,#rstblock
	ld de,#8
	ld bc,#32
	ldir

	; Get the internal DI state right
	call ___hard_di

        ; Stop floppy drive motors
        ld a, #0x0C
        out (FDC_DOR), a

	; Play guess the serial port

	;
	; We are booted under ROMWBW, therefore use the same algorithm as
	; ROMWBW so if the probe fails we at least expect it to have failed
	; before we run.
	;
	; FIXME: see if we can cleanly ask ROMWBW for the device type
	;

	ld hl,#sio_setup
	ld bc,#0xA00 + SIOA_C		; 10 bytes to SIOA_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0C00 + SIOB_C		; and to SIOB_C
	otir

        ; ---------------------------------------------------------------------
	; Initialize CTC
	;
	; Need to do autodetect on this
	;
	; We must initialize all channels of the CTC. The documentation
	; states that the initial CTC state is undefined and we don't want
	; random interrupt surprises
	;
	; ---------------------------------------------------------------------

	;
	; Defence in depth - shut everything up first
	;

	ld a,#0x43
	out (CTC_CH0),a			; set CH0 mode
	out (CTC_CH1),a			; set CH1 mode
	out (CTC_CH2),a			; set CH2 mode
	out (CTC_CH3),a			; set CH3 mode

	;
	; Our input clock is 921.6Khz. It would be tempting to use the CPU
	; clock and chain but the CPU clock depends on the users crystal
	; choice so we have to live with this.
	;

	ld a,#0x80
	out (CTC_CH0),a	; set the CTC vector

	ld a,#0x57
	out (CTC_CH2),a
	xor a
	out (CTC_CH2),a		; Divide by 256

	;
	; Set up counter CH3 as a missed event counter
	;

	ld a,#0xD7
	out (CTC_CH3),a
	ld a,#72		; Keep to 50Hz as used by the firmware
	out (CTC_CH3),a

        ; Done CTC Stuff
        ; ---------------------------------------------------------------------

	xor a
	ld i,a
	im 2				; set Z80 CPU interrupt mode 2
        jp _init_hardware_c             ; pass control to C, which returns for us

sio_setup:
	.byte 0x00
	.byte 0x18		; Reset
	.byte 0x04
	.byte 0x44		; x16 off the 1.8MHz crystal
	.byte 0x01
	.byte 0x1F
	.byte 0x03
	.byte 0xC1
	.byte 0x05
	.byte RTS_LOW
	.byte 0x02
sio_irqv:
	.byte 0x90		; IRQ vector (write to port B only)

;=========================================================================
; Kernel code
;=========================================================================
        .area _CODE

_plt_reboot:
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

spurious:
	reti

plt_interrupt_all:
	ret

; install interrupt vectors
_program_vectors:
	di
	pop de				; temporarily store return address
	pop hl				; function argument -- base page number
	push hl				; put stack back as it was
	push de

	; At this point the common block has already been copied
	call map_proc

	; now install the interrupt vector at 0x0038 (shouldn't be used)

	ld a,#0xC3			; JP instruction
	ld (0x0038),a
	ld hl,#interrupt_handler
	ld (0x0039),hl

	ld (0x0000),a
	ld hl,#null_handler		; to Our Trap Handler
	ld (0x0001),hl

	ld (0x0066),a			; Set vector for NMI
	ld hl,#nmi_handler
	ld (0x0067),hl

	; IM2 vector for the CTC
	ld hl, #spurious
	ld (0x80),hl			; CTC vectors
	ld (0x82),hl
	ld (0x84),hl
	ld hl, #interrupt_handler	; Standard tick handler
	ld (0x86),hl

	ld hl,#siob_txd
	ld (0x90),hl			; SIO B TX empty
	ld hl,#siob_status
	ld (0x92),hl			; SIO B External status
	ld hl,#siob_rx_ring
	ld (0x94),hl			; SIO B Receive
	ld hl,#siob_special
	ld (0x96),hl			; SIO B Special
	ld hl,#sioa_txd
	ld (0x98),hl			; SIO A TX empty
	ld hl,#sioa_status
	ld (0x9A),hl			; SIO A External status
	ld hl,#sioa_rx_ring
	ld (0x9C),hl			; SIO A Received
	ld hl,#sioa_special
	ld (0x9E),hl			; SIO A Special

	jr map_kernel

;=========================================================================
; Memory management
; - kernel pages:     32 - 34
; - common page:      35
; - user space pages: 36 - 63
;=========================================================================

;=========================================================================
; map_proc_always - map process pages
; Inputs: page table address in #U_DATA__U_PAGE
; Outputs: none; all registers preserved
;=========================================================================
map_proc_always:
map_proc_always_di:
	push hl
	ld hl,#_udata + U_DATA__U_PAGE
        jr map_proc_2_pophl_ret

map_buffers_user:
	push hl
	ld hl,(_udata + U_DATA__U_PAGE)
	ld h,#36
	ld (_ubuffer_pages),hl
	ld hl,(_udata + U_DATA__U_PAGE + 2)
	ld (_ubuffer_pages + 2),hl
	ld hl,#_ubuffer_pages
        jr map_proc_2_pophl_ret

map_buffers_user_h:
	push hl
	ld hl,(_udata + U_DATA__U_PAGE)
	ld (_ubuffer_pages),hl
	ld hl,(_udata + U_DATA__U_PAGE + 2)
	ld l,#36
	ld (_ubuffer_pages + 2),hl
	ld hl,#_ubuffer_pages
        jr map_proc_2_pophl_ret

;=========================================================================
; map_proc - map process or kernel pages
; Inputs: page table address in HL, map kernel if HL == 0
; Outputs: none; A and HL destroyed
;=========================================================================
map_proc:
map_proc_di:
	ld a,h
	or l				; HL == 0?
	jr nz,map_proc_2		; HL == 0 - map the kernel

;=========================================================================
; map_kernel - map kernel pages
; Inputs: none
; Outputs: none; all registers preserved
;=========================================================================
map_kernel:
map_kernel_di:
map_kernel_restore:
	push hl
	ld hl,#_kernel_pages
        jr map_proc_2_pophl_ret

;=========================================================================
; map_buffers - map kernel with disk buffers at 0x4000-0x7FFF
; Inputs: none
; Outputs: none; all registers preserved
;=========================================================================
map_buffers:
	push hl
	ld hl,#_kernelbuf_pages
        jr map_proc_2_pophl_ret

;=========================================================================
; map_proc_2 - map process or kernel pages
; Inputs: page table address in HL
; Outputs: none, HL destroyed
;=========================================================================
map_proc_2:
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
map_proc_2_pophl_ret:
	call map_proc_2
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
	jr map_proc_2_pophl_ret

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
	ld hl,#0xEA00
	ld de,#0x2A00
	ld bc,#0x1600
	ldir
	jr map_kernel


; MPGSEL registers are read only, so their content is cached here
mpgsel_cache:
	.db	0,0,0
top_bank:	; the shared tricks code needs this name for cache+3
	.db	0

; kernel page mapping
_kernel_pages:
	.db	32,33,34,35

; kernel page mapping with buffer window
_kernelbuf_pages:
	.db	32,36,34,35

_ubuffer_pages:
	.db	0,0,0,0
; memory page mapping save area for map_save/map_restore
map_savearea:
	.db	0,0,0,0

;
;	A little SIO helper
;
	.globl _sio_r
	.globl _sio2_otir

_sio2_otir:
	ld b,#0x06
	ld c,l
	ld hl,#_sio_r
	otir
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

	push af
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
	ret

;=========================================================================
; inchar - Wait for character on UART, return in A
; Inputs: none
; Outputs: A - received character, F destroyed
;=========================================================================
inchar:
inchar_s:
        xor a                           ; read register 0
        out (SIOA_C), a
	in a,(SIOA_C)   		; read Line Status Register
	and #0x01			; test if data is in receive buffer
	jr z,inchar_s			; no data, wait
	in a,(SIOA_D)   		; read the character from the UART
	ret
;=========================================================================
;
;	Logic for IM2 interrupt driven SIO
;
;=========================================================================

	.area _SERIALDATA

	.globl _sio_state
	.globl _sio_dropdcd
	.globl _sio_rxl
	.globl _sio_txl

; These are laid out and exposed as arrays to C
_sio_wr5:
_sioa_wr5:
	.db 0xEA		; DTR, 8bit, tx enabled
_siob_wr5:
	.db 0xEA		; DTR, 8bit, tx enabled
_sio_flow:
_sioa_flow:
	.db 0			; Flow starts off
_siob_flow:
	.db 0			; Flow starts off
_sio_state:
_sioa_state:
	.db 0			; Last status report
_siob_state:
	.db 0			; Last status report
_sio_dropdcd:
_sioa_dropdcd:
	.db 0			; DCD dropped since last checked
_siob_dropdcd:
	.db 0			; DCD dropped since last checked
_sio_rxl:
_sioa_rxl:
	.db 0
_siob_rxl:
	.db 0
_sio_txl:
_sioa_txl:
	.db 0
_siob_txl:
	.db 0

	.include "../../dev/z80sio.s"

sio_ports a
sio_ports b

	.area _COMMONMEM

;
;	We don't do any clever IRQ re-entry tricks so we can use the istack.
;	If we ever do soft interrupts and take the sio during a timer int
;	we will need a small (16 byte or so) private sio work stack and to
;	be very careful with our stack handling on switches.
;
sio_sp:	.dw 0

.macro switch
	ld (sio_sp),sp
	ld sp,#istack_top
	call map_save_kernel
.endm

.macro switchback
	call map_restore
	ld sp,(sio_sp)
.endm

sio_handler_im2	a, SIOA_C, SIOA_D, reti
sio_handler_im2 b, SIOB_C, SIOB_D, reti

;
;	Stub helpers for code compactness. Note that
;	sdcc_enter_ix is in the standard compiler support already
;
	.area _DISCARD

;
;	The first two use an rst as a jump. In the reload sp case we don't
;	have to care. In the pop ix case for the function end we need to
;	drop the spare frame first, but we know that af contents don't
;	matter
;

rstblock:
	jp	___sdcc_enter_ix
	.ds	5
___spixret:
	ld	sp,ix
	pop	ix
	ret
	.ds	3
___ixret:
	pop	af
	pop	ix
	ret
	.ds	4
___ldhlhl:
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	ret
