; 2014-02-19 Sergey Kiselev
; Zeta SBC V2 hardware specific code

        ; exported symbols
        .export init_hardware
	.export _program_vectors
	.export map_kernel
	.export map_kernel_restore
	.export map_proc
	.export map_proc_always
	.export map_buffers
	.export map_kernel_di
	.export map_proc_di
	.export map_proc_always_di
	.export map_save_kernel
	.export map_restore
	.export map_for_swap
	.export _irqvector
	.export plt_interrupt_all
	.export _copy_common
	.export mpgsel_cache
	.export top_bank
	.export _kernel_pages
	.export _plt_reboot
	.export _bufpool
	.export _int_disabled

	.export inchar
	.export outchar

#include "kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"

;=========================================================================
; Constants
;=========================================================================
CONSOLE_X16		.equ	16 * CONSOLE_RATE
CONSOLE_DIVISOR		.equ	1843200 / CONSOLE_X16

;=========================================================================
; Buffers
;=========================================================================
        .buffers

_bufpool:
        .ds BUFSIZE * 4 ; adjust NBUFS in config.h in line with this

;=========================================================================
; Initialization code
;=========================================================================

	.discard
init_hardware:
        ; program vectors for the kernel
        ld hl, 0
        push hl
        call _program_vectors
        pop hl

	; Compiler helper vectors - in kernel bank only

	ld	hl, rstblock
	ld	de, 8
	ld	bc, 32
	ldir

        ; Stop floppy drive motors
        ld a, 0x0C
        out (FDC_DOR), a

	; initialize UART0
        ld a, (_boot_from_rom)          ; do not set the baud rate and other
        or a                            ; serial line parameters if the BIOS
        jr z, init_partial_uart         ; already set them for us.
	ld a, 0x80			; LCR = DLAB ON
	out (UART0_LCR),a		; set LCR
	ld a, <CONSOLE_DIVISOR		; baud rate divisor - low byte
	out (UART0_DLL),a		; set low byte of divisor
	ld a, >CONSOLE_DIVISOR		; baud rate divisor - high byte
	out (UART0_DLH),a		; set high byte of divisor
	ld a, 0x03			; value for LCR
	out (UART0_LCR),a		; 8 bit data, 1 stop, no parity
init_partial_uart:
	ld a, 0x03			; value for MCR
	out (UART0_MCR),a		; DTR ON, RTS ON
	ld a, 0x06			; disable and clear FIFOs
	out (UART0_FCR),a
	ld a, 0x01			; enable receive data available
	out (UART0_IER),a		; interrupt
	; initialize CTC
	ld a, 0x47			; counter mode, disable interrupts
	out (CTC_CH0),a			; set CH0 mode
	ld a, 0				; time constant = 256
	out (CTC_CH0),a			; set CH0 time constant
	ld a, 0xC7			; counter mode, enable interrupts
	out (CTC_CH1),a			; set CH1 mode
	ld a, 180			; time constant = 180
	out (CTC_CH1),a			; set CH1 time constant
	ld a, 0xD7			; counter mode, rising edge
					; enable interrupts
	out (CTC_CH2),a			; set CH2 mode
	ld a, 1				; time constant = 1
	out (CTC_CH2),a			; set CH2 time constant
	; FIXME: should use interrupts when PPP firmware allows it
	ld a, 0x37			; timer mode for now, disable interrupts
	out (CTC_CH3),a
	ld a, 0				; time constant = 256
	out (CTC_CH3),a			; set CH3 time constant
	ld hl, intvectors
	ld a,l
	and 0xF8			; get bits 7-3 of int. vectors table
	out (CTC_CH0),a			; send it to CTC
	ld a,h				; get bits 15-8 of int. vectors table
	ld i,a				; load to I register
	im 2				; set Z80 CPU interrupt mode 2
        jp _init_hardware_c             ; pass control to C, which returns for us

;=========================================================================
; Kernel code
;=========================================================================
        .code

_plt_reboot:
        ; We need to map the ROM back in -- ideally into every page.
        ; This little trick based on a clever suggestion from John Coffman.
        di
        ld hl, 0xD3 + MPGENA * 256	; OUT (MPGENA), A
        ld (0xFFFE), hl                 ; put it at the very top of RAM
        xor a                           ; A=0
        jp 0xFFFE                       ; execute it; PC then wraps to 0


;=========================================================================
; Common Memory (0xF000 upwards)
;=========================================================================
        .common

;=========================================================================
; Interrupt stuff
;=========================================================================
; IM2 interrupt verctors table
; Note: this is linked after the udata block, so it is aligned on 256 byte
; boundary
intvectors:
	.word	ctc0_int		; CTC CH0 used as prescaler for CH1
	.word	ctc1_int		; timer interrupt handler
	.word	serial_int		; UART interrupt handler
	.word	ppi_int			; PPI interrupt handler

_irqvector:
	.byte	0			; used to identify interrupt vector

; CTC CH0 shouldn't be used to generate interrupts
; but we'll implement it just in case
ctc0_int:
	push af
	xor a				; IRQ vector = 0
	ld (_irqvector),a		; store it
	pop af
	jp interrupt_handler

; periodic timer interrupt
ctc1_int:
	push af
	ld a, 1				; IRQ vector = 1
	ld (_irqvector),a		; store it
	pop af
	jp interrupt_handler

; UART interrupt
serial_int:
	push af
	ld a, 2				; IRQ vector = 2
	ld (_irqvector),a		; store it
	pop af
	jp interrupt_handler

; PPI interrupt - not used for now
ppi_int:
	push af
	ld a, 3				; IRQ vector = 3
	ld (_irqvector),a		; store it
	pop af
	jp interrupt_handler

plt_interrupt_all:
	ret

_int_disabled:
	.byte 1

; install interrupt vectors
_program_vectors:
	di
	pop de				; temporarily store return address
	pop hl				; function argument -- base page number
	push hl				; put stack back as it was
	push de

	; At this point the common block has already been copied
	call map_proc

	; write zeroes across all vectors
	ld hl, 0
	ld de, 1
	ld bc, 0x007f			; program first 0x80 bytes only
	ld (hl), 0x00
	ldir

	; now install the interrupt vector at 0x0038
	; Zeta SBC V2 uses IM 2 interrupts, so nothing should hit this vector
	; use null_handler just in case something causes it anyway
	ld a, 0xC3			; JP instruction
	ld (0x0038),a
	ld hl, null_handler
	ld (0x0039),hl

	; set restart vector for UZI system calls
	ld (0x0030),a			; rst 30h is unix function call vector
	ld hl, unix_syscall_entry
	ld (0x0031),hl

	ld (0x0000),a
	ld hl, null_handler		; to Our Trap Handler
	ld (0x0001),hl

	ld (0x0066),a			; Set vector for NMI
	ld hl, nmi_handler
	ld (0x0067),hl

	jr map_kernel

;=========================================================================
; map_proc_always - map process pages
; Inputs: page table address in U_DATA__U_PAGE
; Outputs: none; all registers preserved
;=========================================================================
map_proc_always:
map_proc_always_di:
	push hl
	ld hl, _udata + U_DATA__U_PAGE
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
; map_buffers - map disk buffers
; Inputs: none
; Outputs: none; all registers preserved
;=========================================================================
map_buffers:
map_kernel:
map_kernel_di:
map_kernel_restore:
	push hl
	ld hl, _kernel_pages
        jr map_proc_2_pophl_ret

;=========================================================================
; map_proc_2 - map process or kernel pages, update mpgsel_cache
; Inputs: page table address in HL
; Outputs: none, HL destroyed
;=========================================================================
map_proc_2:
        push af
        ld a, (hl)
        ld (mpgsel_cache+0), a
        out (MPGSEL_0), a
        inc hl
        ld a, (hl)
        ld (mpgsel_cache+1), a
        out (MPGSEL_1), a
        inc hl
        ld a, (hl)
        ld (mpgsel_cache+2), a
        out (MPGSEL_2), a
        pop af
        ret

;=========================================================================
; map_restore - restore a saved page mapping
; Inputs: none
; Outputs: none, all registers preserved
;=========================================================================
map_restore:
	push hl
	ld hl, map_savearea
map_proc_2_pophl_ret:
	call map_proc_2
	pop hl
	ret

;=========================================================================
; map_save_kernel - save the current page mapping to map_savearea
;		    and switch to the kernel map.
; Inputs: none
; Outputs: none
;=========================================================================
map_save_kernel:
	push hl
	ld hl,(mpgsel_cache)
	ld (map_savearea),hl
	ld hl,(mpgsel_cache+2)
	ld (map_savearea+2),hl
	ld hl, _kernel_pages
	jr map_proc_2_pophl_ret

;=========================================================================
; map_for_swap - map a page into a bank for swap I/O
; Inputs: A = page
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

;
; When we do swapping we need to grab the common page from somewhere and fix
; it up before we can get back to normality. Thus this copy has to be in asm
;
_copy_common:
	pop bc
	pop hl
	pop de
	push de
	push hl
	push bc
	ld a,e
	call map_for_swap
	ld hl, 0xF200
	ld de, 0x7200
	ld bc, 0x0E00
	ldir
	jp map_kernel

; MPGSEL registers are read only, so their content is cached here
mpgsel_cache:
	.byte	0,0,0
top_bank:		; common library needs this name for the right byte
	.byte	0

; kernel page mapping
_kernel_pages:
	.byte	32,33,34,35

; memory page mapping save area for map_save/map_restore
map_savearea:
	.byte	0,0,0,0

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
ocloop:
	in a,(UART0_LSR)		; read Line Status Register
	and 0x20			; get THRE bit
	jr z,ocloop
	; now output the char to serial port
	pop af
	out (UART0_THR),a
	ret

;=========================================================================
; inchar - Wait for character on UART, return in A
; Inputs: none
; Outputs: A - received character, F destroyed
;=========================================================================
inchar:
	in a,(UART0_LSR)		; read Line Status Register
	and 0x01			; test if data is in receive buffer
	jp z,inchar			; no data, wait
	in a,(UART0_RBR)		; read the character from the UART
	ret

;
;	Stub helpers for code compactness. Note that
;	sdcc_enter_ix is in the standard compiler support already
;
	.discard

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
