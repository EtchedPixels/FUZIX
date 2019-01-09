;
;	Linc80 Initial Support
;

        .module linc80

        ; exported symbols
        .globl init_hardware
        .globl interrupt_handler
        .globl _program_vectors
	.globl _kernel_flag
        .globl map_kernel
        .globl map_buffers
        .globl map_process_always
        .globl map_process
        .globl map_kernel_di
        .globl map_process_always_di
        .globl map_save_kernel
        .globl map_restore
	.globl map_for_swap
	.globl _platform_reboot
	.globl _int_disabled
	.globl platform_interrupt_all
	.globl _need_resched

        ; exported debugging tools
        .globl _platform_monitor
        .globl outchar

        ; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl istack_top
        .globl istack_switched_sp
	.globl kstack_top
        .globl unix_syscall_entry
	.globl null_handler
	.globl nmi_handler
        .globl outcharhex
	.globl init

	.globl s__COMMONMEM
	.globl l__COMMONMEM

        .include "kernel.def"
        .include "../kernel.def"


;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

	.globl _bufpool
	.area _BUFFERS

_bufpool:
	.ds BUFSIZE * NBUFS

;
;	We need this above 16K so the ROM doesn't map over it
;
        .area _CODE2

_platform_monitor:
	    ; Reboot ends up back in the monitor
_platform_reboot:
	xor a
	out (0x38), a		; ROM appears low
	rst 0			; bang

_int_disabled:
	.db 1

	.area _COMMONMEM
map_buffers:
map_kernel:
map_kernel_di:
	push af
	ld a,#1
map_a:
	ld (mapreg),a
	out (0x38),a
	pop af
	ret
map_process:
	ld a,h
	or l
	jr z, map_kernel
map_process_always:
map_process_always_di:
map_for_swap:
	push af
	ld a,#3
	jr map_a
map_save_kernel:
	push af
	ld a,(mapreg)
	ld (mapsave),a
	ld a,#1
	jr map_a
map_restore:
	push af
	ld a,(mapsave)
	jr map_a

_program_early_vectors:
        ; write zeroes across all vectors
        ld hl, #0
        ld de, #1
        ld bc, #0x007f ; program first 0x80 bytes only
        ld (hl), #0x00
        ldir

        ; now install the interrupt vector at 0x0038
        ld a, #0xC3 ; JP instruction
        ld (0x0038), a
        ld hl, #interrupt_handler
        ld (0x0039), hl

        ; set restart vector for FUZIX system calls
        ld (0x0030), a   ;  (rst 30h is unix function call vector)
        ld hl, #unix_syscall_entry
        ld (0x0031), hl

        ld (0x0000), a   
        ld hl, #null_handler   ;   to Our Trap Handler
        ld (0x0001), hl

        ld (0x0066), a  ; Set vector for NMI
        ld hl, #nmi_handler
        ld (0x0067), hl

_program_vectors:
platform_interrupt_all:
	ret

	.globl spurious		; so we can debug trap on it

spurious:
	ei
	reti

mapreg:
	.db 0
mapsave:
	.db 0

_need_resched:
	.db 0



; -----------------------------------------------------------------------------
;	All of discard gets reclaimed when init is run
;
;	Discard must be above 0x8000 as we need some of it when the ROM
;	is paged in during init_hardware
; -----------------------------------------------------------------------------
	.area _DISCARD

init_hardware:
	call _program_early_vectors
	; IM2 vector for the CTC
	ld hl, #spurious
	ld (0x80),hl			; CTC vectors
	ld (0x82),hl
	ld (0x84),hl
	ld hl, #interrupt_handler	; Standard tick handler
	ld (0x86),hl

	ld hl,#spurious			; For now
	ld (0x88),hl			; PIO A
	ld (0x8A),hl			; PIO B

	ld hl,#sio2b_txd
	ld (0x90),hl			; SIO B TX empty
	ld hl,#sio2b_status
	ld (0x92),hl			; SIO B External status
	ld hl,#sio2b_rx_ring
	ld (0x94),hl			; SIO B Receive
	ld hl,#sio2b_special
	ld (0x96),hl			; SIO B Special
	ld hl,#sio2a_txd
	ld (0x98),hl			; SIO A TX empty
	ld hl,#sio2a_status
	ld (0x9A),hl			; SIO A External status
	ld hl,#sio2a_rx_ring
	ld (0x9C),hl			; SIO A Received
	ld hl,#sio2a_special
	ld (0x9E),hl			; SIO A Special
	ld hl, #80
        ld (_ramsize), hl
	ld hl,#32
        ld (_procmem), hl

	ld hl,#sio_setup
	ld bc,#0xA00 + SIOA_C		; 10 bytes to SIOA_C
	otir
	ld hl,#sio_setup
	ld bc,#0x0C00 + SIOB_C		; and to SIOB_C with vector 
	otir

	;
	;	Now program up the CTC
	;	CTC 0 and CTC 1 are set up for the serial and we should
	;	not touch them. We configure up 2 and 3
	;
	ld a,#0x80
	out (CTC_CH0),a	; set the CTC vector

	ld a,#0x25	; timer, 256 prescale, irq off
	out (CTC_CH2),a
	ld a,#180	; counter base (160 per second)
	out (CTC_CH2),a
	ld a,#0xF5	; counter, irq on
	out (CTC_CH3),a
	ld a,#16
	out (CTC_CH3),a	; 160 per sec down to 10 per sec

	xor a
	ld i,a	; Use upper half of rst vector page
        im 2	; set CPU interrupt

        ret

RTS_LOW	.EQU	0xEA

sio_setup:
	.byte 0x00
	.byte 0x18		; Reset
	.byte 0x04
	.byte 0xC4		; x64 async 1 stop no parity
	.byte 0x01
	.byte 0x1F		; status affects vector, ti, ei, int all
	.byte 0x03
	.byte 0xE1		; 8bit, autoen, rx en
	.byte 0x05
	.byte RTS_LOW		; dtr, 8bit, tx en, rts
	.byte 0x02
	.byte 0x90		; IRQ vector (B only)

	    .area _CODE

_kernel_flag:
	    .db 1	; We start in kernel mode


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
;
; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
;
; We use the A port for debug as the console is usually on B
;
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

;
;	SIO2 IM2 console driver
;
;	The buffers are 256 bytes per channel and page aligned. The lower
;	half is used for receive the upper for transmit. Both are rings.

;
;	Transmit data from the queue. We need a stop system for this but
;	actually the logical one might be to just turn that IRQ off, so long
;	as we remember to kickstart it again properly. Check if it's enough
;	to just unmask the IRQ bit ?
;
;	All of this lives in common space so we don't bank switch so much.

	.area _CONSOLE

sio2a_rx:
	jp	init		; for boot only
	.ds	128-3		; we wrote a JP init in the first 3
sio2a_tx:			; that will be recycled
	.ds	128
sio2b_rx:
	.ds	128
sio2b_tx:
	.ds	128

	.area _COMMONMEM
_sio2a_txl:
	.db	0
sio2a_error:
	.db	0
sio2a_rxover:
	.db	0
sio2a_stat:
	.db	0
sio2a_txp:
	.dw	sio2a_tx
sio2a_txe:
	.dw	sio2a_tx
_sio2a_rxl:
	.db	0
sio2a_rxp:
	.dw	sio2a_rx
sio2a_rxe:
	.dw	sio2a_rx

_sio2b_txl:
	.db	0
sio2b_error:
	.db	0
sio2b_rxover:
	.db	0
sio2b_stat:
	.db	0
sio2b_txp:
	.dw	sio2a_tx
sio2b_txe:
	.dw	sio2a_tx
_sio2b_rxl:
	.db	0
sio2b_rxp:
	.dw	sio2a_rx
sio2b_rxe:
	.dw	sio2a_rx

;
;	Interrupt vector handler for port A transmit empty
;
sio2a_txd:
	push af
	ld a,(_sio2a_txl)
	or a
	jr z, tx_a_none
	push hl
	dec a
	ld (_sio2a_txl),a
	ld hl,(sio2a_txp)
	ld a,(hl)
	out (SIOA_D),a
	inc hl
	set 7,l
	ld (sio2a_txp),hl
	pop hl
tx_a_none:
	ld a,#0x28
	out (SIOA_C),a		; silence tx interrupt
	pop af
	ei
	reti
;
;	Interrupt vector handler for port A receive ready
;
sio2a_rx_ring:
	push af
	push hl
sio2a_rx_next:
	in a,(SIOA_D)		; read ASAP
	ld l,a
	ld a,(_sio2a_rxl)
	inc a
	jp m, a_rx_over
	ld (_sio2a_rxl),a
	; should we check bit 5/6 and if appropriate flow control on bit 5/6
	; high ?
;sio2a_flow_patch:		; patch with fastest 5 byte nop
;	cp #0x60		; flow control threshold
;	call z,  _sio2a_flow_control_on
	ld a,l
	ld hl,(sio2a_rxp)
	ld (hl),a
	inc l
	res 7,l
	ld (sio2a_rxp),hl
	;
	;	The chip has a small FIFO and bytes can also arrive as we
	;	read. To maximise performance try and empty it each time.
	;
	;	This is bounded as worst case at high data rate and low
	;	CPU speed we will overrun and bail out.
	;
	in a,(SIOA_C)		; RR 0
	rra
	jr c, sio2a_rx_next
	pop hl
	pop af
	ei
	reti
a_rx_over:
	ld a,(sio2a_error)
	or #0x20		; Fake an RX overflow bit
	ld (sio2a_rxover),a
	pop af
	ei
	reti
;
;	Interrupt vector for a port A status change
;
;	FIXME:
;		log the or of changes seem (dcd down, up) and last state
;	of both CTS and DCD. The CTS last state is sufficient for flow
;	and we can use thw two edges of DCD to work out if we had a hangup
;	and if we are now open.
;
sio2a_status:
	; CTS or DCD change
	push af
	push hl
	; RR0
	in a,(SIOA_C)
	; Clear the latched values
	ld a,#0x10
	out (SIOA_C),a
	pop hl
	pop af
	ei
	reti

;
;	Interrupt vector for a port A error
;
sio2a_special:
	; Parity, RX Overrun, Framing
	; Probably want to record them, but we at least must clean up
	push af
	ld a,#1
	out (SIOA_C),a		; RR1 please
	in a,(SIOA_C)		; clear events
	ld (sio2a_error),a	; Save error bits
	; Clear the latched values
	ld a,#0x30
	out (SIOA_C),a
	pop af
	ei
	reti

;
;	Interrupt vector handler for port B transmit empty
;
sio2b_txd:
	push af
	ld a,(_sio2b_txl)
	or a
	jr z, tx_b_none
	push hl
	dec a
	ld (_sio2b_txl),a
	ld hl,(sio2b_txp)
	ld a,(hl)
	out (SIOB_D),a
	inc hl
	set 7,l
	ld (sio2b_txp),hl
	pop hl
tx_b_none:
	ld a,#0x28
	out (SIOB_C),a		; silence tx interrupt
	pop af
	ei
	reti
;
;	Interrupt vector handler for port B receive ready
;
sio2b_rx_ring:
	push af
	push hl
sio2b_rx_next:
	in a,(SIOB_D)		; read ASAP
	ld l,a
	ld a,(_sio2b_rxl)
	inc a
	jp m, b_rx_over
	ld (_sio2b_rxl),a
	; should we check bit 5/6 and if appropriate flow control on bit 5/6
	; high ?
;sio2b_flow_patch:		; patch with fastest 5 byte nop
;	cp #0x60		; flow control threshold
;	call z,  _sio2b_flow_control_on
	ld a,l
	ld hl,(sio2b_rxp)
	ld (hl),a
	inc l
	res 7,l
	ld (sio2b_rxp),hl
	;
	;	The chip has a small FIFO and bytes can also arrive as we
	;	read. To maximise performance try and empty it each time.
	;
	;	This is bounded as worst case at high data rate and low
	;	CPU speed we will overrun and bail out.
	;
	in a,(SIOB_C)		; RR 0
	rra
	jr c, sio2b_rx_next
	pop hl
	pop af
	ei
	reti
b_rx_over:
	ld a,(sio2b_error)
	or #0x20		; Fake an RX overflow bit
	ld (sio2b_rxover),a
	pop af
	ei
	reti
;
;	Interrupt vector for a port B status change
;
;	FIXME:
;		log the or of changes seem (dcd down, up) and last state
;	of both CTS and DCD. The CTS last state is sufficient for flow
;	and we can use thw two edges of DCD to work out if we had a hangup
;	and if we are now open.
;
sio2b_status:
	; CTS or DCD change
	push af
	push hl
	; RR0
	in a,(SIOB_C)
	; Clear the latched values
	ld a,#0x10
	out (SIOB_C),a
	pop hl
	pop af
	ei
	reti

;
;	Interrupt vector for a port B error
;
sio2b_special:
	; Parity, RX Overrun, Framing
	; Probably want to record them, but we at least must clean up
	push af
	ld a,#1
	out (SIOB_C),a		; RR1 please
	in a,(SIOB_C)		; clear events
	ld (sio2b_error),a	; Save error bits
	; Clear the latched values
	ld a,#0x30
	out (SIOB_C),a
	pop af
	ei
	reti

;
;	C interface methods
;
	.globl _sio2a_txqueue
	.globl _sio2a_flow_control_on
	.globl _sio2a_rx_get
	.globl _sio2a_error_get

	.globl _sio2a_rxl
	.globl _sio2a_txl
	.globl _sio2a_wr5

	.globl _sio2b_txqueue
	.globl _sio2b_flow_control_on
	.globl _sio2b_rx_get
	.globl _sio2b_error_get

	.globl _sio2b_rxl
	.globl _sio2b_txl
	.globl _sio2b_wr5

_sio2a_wr5:
	.db 0xEA		; DTR, 8bit, tx enabled
_sio2b_wr5:
	.db 0xEA		; DTR, 8bit, tx enabled

;
;	Queue a byte to be sent (DI required)
;
;	l = byte
;
;	Need a way to halt processing somewhere here or a_tx ?
;	(or can we use hardware ?)
;	128 byte ring buffer aligned to upper half (rx is in lower)
;
_sio2a_txqueue:
	ld a,(_sio2a_txl)
	or a
	jr z, sio2a_direct_maybe	; if can tx now then do
	inc a
	jp m, txa_overflow
sio2a_queue:
	ld (_sio2a_txl),a
	ld a,l
	ld hl,(sio2a_txe)
	ld (hl),a
	inc l
	set 7,l
	ld (sio2a_txe),hl
	ld l,#0
	ret
txa_overflow:
	; some kind of flag for error
	ld l,#1
	ret
sio2a_direct_maybe:
	; check RR
	in a,(SIOA_C)
	and #0x04		; RX space ?
	; if space
	ld a,#1
	jr nz, sio2a_queue
	; bypass the queue and kickstart the interrupt machine
	ld a,l
	out (SIOA_D),a
	ld l,#0
	ret
	; Call with DI

_sio2a_flow_control_on:
	ld a,#5
	out(SIOA_C),a		; WR 5
	ld a,(_sio2a_wr5)
	or #0x02
	out (SIOA_C),a		; Turn on RTS
	ret

	; DI required
	; Returns char in L
	;
	; Caller responsible for making post buffer fetch decisions about
	; RTS
_sio2a_rx_get:
	ld a,(_sio2a_rxl)
	or a
	ret z
	dec a
	ld (_sio2a_rxl),a
	ld hl,(sio2a_rxe)
	ld a,(hl)
	inc l
	res 7,l
	ld (sio2a_rxe),hl
	scf
	ld l,a
	ret

	; DI required
_sio2a_error_get:
	ld hl,#sio2a_error
	ld a,(hl)
	ld (hl),#0
	ld l,a
	ret

;
;	Queue a byte to be sent (DI required)
;
;	l = byte
;
;	Need a way to halt processing somewhere here or a_tx ?
;	(or can we use hardware ?)
;	128 byte ring buffer aligned to upper half (rx is in lower)
;
_sio2b_txqueue:
	ld a,(_sio2b_txl)
	or a
	jr z, sio2b_direct_maybe	; if can tx now then do
	inc a
	jp m, txb_overflow
sio2b_queue:
	ld (_sio2b_txl),a
	ld a,l
	ld hl,(sio2b_txe)
	ld (hl),a
	inc l
	set 7,l
	ld (sio2b_txe),hl
	ld l,#0
	ret
txb_overflow:
	; some kind of flag for error
	ld l,#1
	ret
sio2b_direct_maybe:
	; check RR
	in a,(SIOB_C)
	and #0x04		; RX space ?
	; if space
	ld a,#1
	jr z, sio2b_queue
	; bypass the queue and kickstart the interrupt machine
	ld a,l
	out (SIOB_D),a
	ld l,#0
	ret
	; Call with DI

_sio2b_flow_control_on:
	ld a,#5
	out(SIOB_C),a		; WR 5
	ld a,(_sio2b_wr5)
	or #0x02
	out (SIOB_C),a		; Turn on RTS
	ret

	; DI required
	; Returns char in L
	;
	; Caller responsible for making post buffer fetch decisions about
	; RTS
_sio2b_rx_get:
	ld a,(_sio2b_rxl)
	or a
	ret z
	dec a
	ld (_sio2b_rxl),a
	ld hl,(sio2b_rxe)
	ld a,(hl)
	inc l
	res 7,l
	ld (sio2b_rxe),hl
	scf
	ld l,a
	ret

	; DI required
_sio2b_error_get:
	ld hl,#sio2b_error
	ld a,(hl)
	ld (hl),#0
	ld l,a
	ret

