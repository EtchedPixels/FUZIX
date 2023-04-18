        .module z80retro

        ; exported symbols
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_kernel_restore
	.globl map_process
	.globl map_buffers
	.globl map_kernel_di
	.globl map_process_di
	.globl map_process_always
	.globl map_process_always_di
	.globl map_save_kernel
	.globl map_restore
	.globl map_for_swap
	.globl plt_interrupt_all
	.globl _copy_common
	.globl mpgsel_cache
	.globl top_bank
	.globl _kernel_pages
	.globl _plt_reboot
	.globl _plt_monitor
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

	; exported debugging tools
	.globl outchar
	.globl inchar

	; interrupt vectors
	.globl spurious
	.globl siob_txd
	.globl siob_status
	.globl siob_rx_ring
	.globl siob_special
	.globl sioa_txd
	.globl sioa_status
	.globl sioa_rx_ring
	.globl sioa_special

        .include "kernel.def"
        .include "../kernel-z80.def"

;=========================================================================
; Constants
;=========================================================================
RTS_HIGH	.EQU	0xE8
RTS_LOW		.EQU	0xEA

;=========================================================================
; Vector init table - only needed at boot
;=========================================================================

	.area _DISCARD
vectortab:
; 0xXXE0: Our IM2 table
	.word	spurious
	.word	spurious
	.word	spurious
	.word	interrupt_handler	; CTC 3 - timer tick
	.word	0
	.word	0
	.word	0
	.word	0
; 0xxxF0 SIO
	.word	siob_txd
	.word	siob_status
	.word	siob_rx_ring
	.word	siob_special
	.word	sioa_txd
	.word	sioa_status
	.word	sioa_rx_ring
	.word	sioa_special

	.area	_VECTORS
vectors:

;=========================================================================
; Initialization code
;=========================================================================
        .area _DISCARD
init_hardware:
        ;	Install IM2 vector table
	ld	hl,#vectortab
	ld	de,#vectors
	ld	bc,#32
	ldir

        ld hl, #0
        push hl
        call _program_vectors
        pop hl

	; Get the internal DI state right
	call ___hard_di

	ld hl,#sio_setup
	ld bc,#0xA00 + SIOA_C		; 10 bytes to SIOA_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0C00 + SIOB_C		; and to SIOB_C
	otir

        ; ---------------------------------------------------------------------
	; Initialize CTC
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
	;	The clock/trg lines are jumper configured
	;	between SQWOUT and each other.CTC is optional
	;	but not for Fuzix. SQWOUT isn't terribly useful
	;	and the CPU clock is annoying. Serial is clocked
	;	at either clock or clock / 2 and not linked to the
	;	CTC ports.CPU clock is 14.74MHz.
	;
	ld a,#0xE0
	out (CTC_CH0),a		; set the CTC vector

	ld a,#0xA7		; CPU clock / 256, interrupts on
	out (CTC_CH3),a
	ld a,#240		; 240 x 240 x 256
	out (CTC_CH3),a		; Divide by 240 .. 240Hz tick - faster than
				; we want by far

	; We would like to use CTC 3 as a chained counter but we can't
	; assume the user jumpered it to suit our desires

        ; Done CTC Stuff
        ; ---------------------------------------------------------------------

	ld	hl,#vectors		; Keep linker mappy
	ld	a,h
	ld	i,a
	im	2			; set Z80 CPU interrupt mode 2
        jp	_init_hardware_c	; pass control to C, which returns for us

sio_setup:
	.byte 0x00
	.byte 0x18		; Reset
	.byte 0x04
	.byte 0xC4		; x64 off the CPU/2 clock - 115200 baud
	.byte 0x01
	.byte 0x1F
	.byte 0x03
	.byte 0xC1
	.byte 0x05
	.byte RTS_LOW
	.byte 0x02
sio_irqv:
	.byte 0xF0		; IRQ vector (write to port B only)

;=========================================================================
; Kernel code
;=========================================================================
        .area _CODE

_plt_monitor:
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
	call map_process

	; write zeroes across all vectors
	ld hl,#0
	ld de,#1
	ld bc,#0x007f			; program first 0x80 bytes only
	ld (hl),#0x00
	ldir

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
	ld hl,#_udata + U_DATA__U_PAGE
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
map_kernel_restore:
map_buffers:
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
	ld hl,#0xEA00
	ld de,#0x2A00
	ld bc,#0x1600
	ldir
	jr map_kernel


; MPGSEL registers are write only, so their content is cached here
mpgsel_cache:
	.db	0,0,0
top_bank:	; the shared tricks code needs this name for cache+3
	.db	0

; kernel page mapping
_kernel_pages:
	.db	0x3C,0x3D,0x3E,0x3F

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

	.include "../dev/z80sio.s"

sio_ports a
sio_ports b

	.area _COMMONMEM

;
;	As the serial is so fast we keep it in common so we can avoid any
;	switching overheads.
;

.macro switch
.endm

.macro switchback
.endm

sio_handler_im2	a, SIOA_C, SIOA_D, reti
sio_handler_im2 b, SIOB_C, SIOB_D, reti


;
;	SPI banger
;
;	Split across ports which limits optimizations. Could really
;	do with the clock being strobed by the output port write
;	instead.
;

	.globl _sd_spi_transmit_byte
	.globl _sd_spi_receive_byte
	.globl _sd_spi_transmit_sector
	.globl _sd_spi_receive_sector

	.globl _td_raw
	.globl _td_page

_sd_spi_transmit_byte:
	ld	e,l
sd_tx:
	; bit 0
	rl	e
	rla
	out	(0x68),a
	out	(0x69),a
	; bit 1
	rl	e
	rla
	out	(0x68),a
	out	(0x69),a
	; bit 2
	rl	e
	rla
	out	(0x68),a
	out	(0x69),a
	; bit 3
	rl	e
	rla
	out	(0x68),a
	out	(0x69),a
	; bit 4
	rl	e
	rla
	out	(0x68),a
	out	(0x69),a
	; bit 5
	rl	e
	rla
	out	(0x68),a
	out	(0x69),a
	; bit 6
	rl	e
	rla
	out	(0x68),a
	out	(0x69),a
	; bit 7
	rl	e
	rla
	out	(0x68),a
	out	(0x69),a
	ret
;
;	Receive is by far the most important path
;
_sd_spi_receive_byte:
	ld	c,#0xFF
	call	sd_rx
	ld	l,e
	ret

; FIXME - rotate C
sd_rx:
	ld	a,c
	; bit 0
	out	(0x68),a
	out	(0x69),a
	in	a,(0x64)
	rra
	rl	e
	; bit 1
	ld	a,c
	out	(0x68),a
	out	(0x69),a
	in	a,(0x64)
	rra
	rl	e
	; bit 2
	ld	a,c
	out	(0x68),a
	out	(0x69),a
	in	a,(0x64)
	rra
	rl	e
	; bit 3
	ld	a,c
	out	(0x68),a
	out	(0x69),a
	in	a,(0x64)
	rra
	rl	e
	; bit 4
	ld	a,c
	out	(0x68),a
	out	(0x69),a
	in	a,(0x64)
	rra
	rl	e
	; bit 5
	ld	a,c
	out	(0x68),a
	out	(0x69),a
	in	a,(0x64)
	rra
	rl	e
	; bit 6
	ld	a,c
	out	(0x68),a
	out	(0x69),a
	in	a,(0x64)
	rra
	rl	e
	; bit 7
	ld	a,c
	out	(0x68),a
	out	(0x69),a
	in	a,(0x64)
	rra
	rl	e
	ret

_sd_spi_receive_sector:
	ld	bc,#0xFF		; 0 for count 255 for reload of A
	ld	a,(_td_raw)
	or	a
	jr	z, rx_byte
	dec	a
	jr	z, rx_user
	ld	a,(_td_page)
	call	map_for_swap
	jr	rx_byte
rx_user:
	call	map_process_always
rx_byte:
	call	sd_rx
	ld	(hl),e
	inc	hl
	call	sd_rx
	ld	(hl),e
	inc	hl
	djnz	rx_byte
	jp	map_kernel

_sd_spi_transmit_sector:
	ld	b,#0
	ld	a,(_td_raw)
	or	a
	jr	z, tx_byte
	dec	a
	jr	z, tx_user
	ld	a,(_td_page)
	call	map_for_swap
	jr	tx_byte
tx_user:
	call	map_process_always
tx_byte:
	ld	e,(hl)
	inc	hl
	call	sd_tx
	ld	e,(hl)
	inc	hl
	call	sd_tx
	djnz	tx_byte
	jp	map_kernel
