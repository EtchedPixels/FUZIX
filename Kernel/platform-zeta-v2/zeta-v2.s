; 2014-02-19 Sergey Kiselev
; Zeta SBC V2 hardware specific code

        .module zeta-v2

        ; exported symbols
        .globl init_early
        .globl init_hardware
	.globl _program_vectors
	.globl map_kernel
	.globl map_process
	.globl map_process_always
	.globl map_save
	.globl map_restore
	.globl _irqvector
	.globl platform_interrupt_all
	.globl mpgsel_cache
	.globl _kernel_pages

        ; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl outhl
        .globl outnewline
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl nmi_handler
	.globl null_handler
        .globl _boot_from_rom

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

;=========================================================================
; Initialization code
;=========================================================================
        .area _DISCARD

init_early:
        ; We want to be at the bottom of the RAM in the physical address space.
        ; This may or may not alredy be the case. We can't read out the current
        ; values of the memory banking registers.

        ; We are going to assume that we are loaded in either one 64K chunk
        ; or two 32K chunks, and that this code, plus the stack, live in the
        ; top 16K bank. First we find ourselves in physical memory.
        ld hl, #0x0000      ; go looking for bank 0
        call searchpage     ; find it
        ld d, a             ; store mapping for low 32K
        ld hl, #0x8000      ; now go looking for bank 3
        call searchpage     ; find it
        ld e, a             ; store mapping for top 64K
        push de

        ; Now we copy ourselves to the right place in physical memory.
        ld a, #32           ; target = first page of RAM
        call copy32k        ; copy first 32K
        pop de              ; recover memory mapping
        ld d, e             ; now for the next 32K
        ld a, #34           ; target = third page of RAM
        call copy32k        ; copy second 32K

        ; Finally remap all four 16K banks to the bottom of physical memory.
        ld a, #32           ; first page of RAM
        ld c, #MPGSEL_0     ; first bank register
mappage:
        out (c), a          ; map
        inc c               ; next bank
        inc a               ; next page
        cp #36              ; done last page?
        ret z
        jr mappage

copy32k:
        out (MPGSEL_1), a
        inc a
        push af             ; store next RAM page
        ld a, d             ; setup source
        out (MPGSEL_0), a
        call copy16k
        inc a
        out (MPGSEL_0), a   ; source is now second half
        pop af              ; recover next RAM page
        out (MPGSEL_1), a
        ; fall through to copy16k
copy16k:                    ; copy bank 0 to bank 1
        ld hl, #0x0000
        ld de, #0x4000
        ld bc, #0x4000
        ldir
        ret

searchpage:                 ; search for a page of virtual memory in physical memory
        ld a, (hl)          ; read a byte
        ld c, a             ; save it
        ld b, #32           ; first 512K is ROM on Zeta2
checkpage:
        ld a, #0x55         ; known value
        ld (hl), a          ; write it at a fixed address
        ld a, b             ; map the page
        out (MPGSEL_1), a   ; at 0x4000
        ld a, (0x4000)      ; read test byte from remapped memory
        cp #0x55            ; found it?
        jr nz, nextpage     ; no
        ld a, #0x42         ; test with another value (possibly it was 0x55 by chance)
        ld (hl), a          ; write it
        ld a, (0x4000)      ; read it
        cp #0x42            ; found it?
        jr z, foundpage     ; we got one!
nextpage:
        inc b               ; next page
        jr nz, checkpage    ; check next page -- note we test pages above 1MB too
        halt                ; wrapped to page 0, not found it: shouldn't happen
foundpage:
        ld a, c             ; get back the byte
        ld (hl), a          ; put it back before anyone notices
        ld a, b             ; return with page number in A
	ret

init_hardware:
        ld hl, #RAM_KB			; set system RAM size
        ld (_ramsize), hl
        ld hl,#(RAM_KB-64)		; 64K for kernel
        ld (_procmem), hl

	; initialize UART0
        ld a, (_boot_from_rom)          ; do not set the baud rate and other
        or a                            ; serial line parameters if the BIOS
        jr z, init_partial_uart         ; already set them for us.
	ld a,#0x80			; LCR = DLAB ON
	out (UART0_LCR),a		; set LCR
	ld a,#CONSOLE_DIVISOR_LOW	; baud rate divisor - low byte
	out (UART0_DLL),a		; set low byte of divisor
	ld a,#CONSOLE_DIVISOR_HIGH	; baud rate divisor - high byte
	out (UART0_DLH),a		; set high byte of divisor
	ld a,#0x03			; value for LCR
	out (UART0_LCR),a		; 8 bit data, 1 stop, no parity
init_partial_uart:
	ld a,#0x03			; value for MCR
	out (UART0_MCR),a		; DTR ON, RTS ON
	ld a,#0x06			; disable and clear FIFOs
	out (UART0_FCR),a
	ld a,#0x01			; enable receive data available
	out (UART0_IER),a		; interrupt
	; initialize CTC
	ld a,#0x47			; counter mode, disable interrupts
	out (CTC_CH0),a			; set CH0 mode
	ld a,#0				; time constant = 256
	out (CTC_CH0),a			; set CH0 time constant
	ld a,#0xC7			; counter mode, enable interrupts
	out (CTC_CH1),a			; set CH1 mode
	ld a,#180			; time constant = 180
	out (CTC_CH1),a			; set CH1 time constant
	ld a,#0xD7			; counter mode, rising edge
					; enable interrupts
	out (CTC_CH2),a			; set CH2 mode
	ld a,#1				; time constant = 1
	out (CTC_CH2),a			; set CH2 time constant
	; FIXME: should use interrupts when PPP firmware allows it
	ld a,#0x37			; timer mode for now, disable interrupts
	out (CTC_CH3),a
	ld a,#0				; time constant = 256
	out (CTC_CH3),a			; set CH3 time constant
	ld hl,#intvectors
	ld a,l
	and #0xF8			; get bits 7-3 of int. vectors table
	out (CTC_CH0),a			; send it to CTC
	ld a,h				; get bits 15-8 of int. vectors table
	ld i,a				; load to I register
	im 2				; set Z80 CPU interrupt mode 2

	ret

;=========================================================================
; Common Memory (0xF000 upwards)
;=========================================================================
        .area _COMMONMEM

;=========================================================================
; Interrupt stuff
;=========================================================================
; IM2 interrupt verctors table
; Note: this is linked after the udata block, so it is aligned on 256 byte
; boundary
intvectors:
	.dw	ctc0_int		; CTC CH0 used as prescaler for CH1
	.dw	ctc1_int		; timer interrupt handler
	.dw	serial_int		; UART interrupt handler
	.dw	ppi_int			; PPI interrupt handler

_irqvector:
	.db	0			; used to identify interrupt vector

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
	ld a,#1				; IRQ vector = 1
	ld (_irqvector),a		; store it
	pop af
	jp interrupt_handler

; UART interrupt
serial_int:
	push af
	ld a,#2				; IRQ vector = 2
	ld (_irqvector),a		; store it
	pop af
	jp interrupt_handler

; PPI interrupt - not used for now
ppi_int:
	push af
	ld a,#3				; IRQ vector = 3
	ld (_irqvector),a		; store it
	pop af
	jp interrupt_handler

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
	; Zeta SBC V2 uses IM 2 interrupts, so nothing should hit this vector
	; use null_handler just in case something causes it anyway
	ld a,#0xC3			; JP instruction
	ld (0x0038),a
	ld hl,#null_handler
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
	push hl
	ld hl,#U_DATA__U_PAGE
	call map_process_2
	pop hl
	ret

;=========================================================================
; map_process - map process or kernel pages
; Inputs: page table address in HL, map kernel if HL == 0
; Outputs: none; A and HL destroyed
;=========================================================================
map_process:
	ld a,h
	or l				; HL == 0?
	jr nz,map_process_2		; HL == 0 - map the kernel

;=========================================================================
; map_kernel - map kernel pages
; Inputs: none
; Outputs: none; all registers preserved
;=========================================================================
map_kernel:
	push hl
	ld hl,#_kernel_pages
	call map_process_2
	pop hl
	ret

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
	call map_process_2
	pop hl
	ret

;=========================================================================
; map_save - save the current page mapping to map_savearea
; Inputs: none
; Outputs: none
;=========================================================================
map_save:
	push hl
	ld hl,(mpgsel_cache)
	ld (map_savearea),hl
	ld hl,(mpgsel_cache+2)
	ld (map_savearea+2),hl
	pop hl
	ret

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
	; wait for transmitter to be idle
ocloop:
	in a,(UART0_LSR)		; read Line Status Register
	and #0x20			; get THRE bit
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
	and #0x01			; test if data is in receive buffer
	jp z,inchar			; no data, wait
	in a,(UART0_RBR)		; read the character from the UART
	ret
