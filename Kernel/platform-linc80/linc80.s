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
	ld a,#90	; counter base (320 per second)
	out (CTC_CH2),a
	ld a,#0xF5	; counter, irq on
	out (CTC_CH3),a
	ld a,#8
	out (CTC_CH3),a	; 320 per sec down to 40 per sec

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

sioa_rx:
	jp	init		; for boot only
	.ds	128-3		; we wrote a JP init in the first 3
sioa_tx:			; that will be recycled
	.ds	128
siob_rx:
	.ds	128
siob_tx:
	.ds	128

	.area _COMMONMEM
sioa_error:
	.db	0
sioa_rxover:
	.db	0
sioa_stat:
	.db	0
sioa_txp:
	.dw	sioa_tx
sioa_txe:
	.dw	sioa_tx
sioa_rxp:
	.dw	sioa_rx
sioa_rxe:
	.dw	sioa_rx

siob_error:
	.db	0
siob_rxover:
	.db	0
siob_stat:
	.db	0
siob_txp:
	.dw	sioa_tx
siob_txe:
	.dw	sioa_tx
siob_rxp:
	.dw	sioa_rx
siob_rxe:
	.dw	sioa_rx

;
;	Interrupt vector handler for port A transmit empty
;
sioa_txd:
	push af
	ld a,(_sioa_txl)
	or a
	jr z, tx_a_none
	push hl
	dec a
	ld (_sioa_txl),a
	ld hl,(sioa_txp)
	ld a,(hl)
	out (SIOA_D),a
	inc hl
	set 7,l
	ld (sioa_txp),hl
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
sioa_rx_ring:
	push af
	push hl
sioa_rx_next:
	in a,(SIOA_D)		; read ASAP
	ld l,a
	ld a,(_sioa_rxl)
	inc a
	jp m, a_rx_over
	ld (_sioa_rxl),a
	; should we check bit 5/6 and if appropriate flow control on bit 5/6
	; high ?
	cp #0x60		; flow control threshold
	call z, _sioa_flow_control_on
	ld a,l
	ld hl,(sioa_rxp)
	ld (hl),a
	inc l
	res 7,l
	ld (sioa_rxp),hl
	;
	;	The chip has a small FIFO and bytes can also arrive as we
	;	read. To maximise performance try and empty it each time.
	;
	;	This is bounded as worst case at high data rate and low
	;	CPU speed we will overrun and bail out.
	;
	in a,(SIOA_C)		; RR 0
	rra
	jr c, sioa_rx_next
	pop hl
	pop af
	ei
	reti
a_rx_over:
	ld a,(sioa_error)
	or #0x20		; Fake an RX overflow bit
	ld (sioa_rxover),a
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
sioa_status:
	; CTS or DCD change
	push af
	push hl
	; RR0
	in a,(SIOA_C)
	ld (_sioa_state),a
	and #8
	jr z, no_dcd_drop_a
	; \DCD went high
	ld (_sioa_dropdcd),a		; Set the dcdflag
no_dcd_drop_a:
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
sioa_special:
	; Parity, RX Overrun, Framing
	; Probably want to record them, but we at least must clean up
	push af
	ld a,#1
	out (SIOA_C),a		; RR1 please
	in a,(SIOA_C)		; clear events
	ld (sioa_error),a	; Save error bits
	; Clear the latched values
	ld a,#0x30
	out (SIOA_C),a
	pop af
	ei
	reti

;
;	Interrupt vector handler for port B transmit empty
;
siob_txd:
	push af
	ld a,(_siob_txl)
	or a
	jr z, tx_b_none
	push hl
	dec a
	ld (_siob_txl),a
	ld hl,(siob_txp)
	ld a,(hl)
	out (SIOB_D),a
	inc hl
	set 7,l
	ld (siob_txp),hl
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
siob_rx_ring:
	push af
	push hl
siob_rx_next:
	in a,(SIOB_D)		; read ASAP
	ld l,a
	ld a,(_siob_rxl)
	inc a
	jp m, b_rx_over
	ld (_siob_rxl),a
	; should we check bit 5/6 and if appropriate flow control on bit 5/6
	; high ?
	cp #0x60		; flow control threshold
	call z, _siob_flow_control_on
	ld a,l
	ld hl,(siob_rxp)
	ld (hl),a
	inc l
	res 7,l
	ld (siob_rxp),hl
	;
	;	The chip has a small FIFO and bytes can also arrive as we
	;	read. To maximise performance try and empty it each time.
	;
	;	This is bounded as worst case at high data rate and low
	;	CPU speed we will overrun and bail out.
	;
	in a,(SIOB_C)		; RR 0
	rra
	jr c, siob_rx_next
	pop hl
	pop af
	ei
	reti
b_rx_over:
	ld a,(siob_error)
	or #0x20		; Fake an RX overflow bit
	ld (siob_rxover),a
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
siob_status:
	; CTS or DCD change
	push af
	push hl
	; RR0
	in a,(SIOB_C)
	ld (_siob_state),a
	and #8
	jr z, no_dcd_drop_b
	; \DCD went high
	ld (_siob_dropdcd),a		; Set the dcdflag
no_dcd_drop_b:
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
siob_special:
	; Parity, RX Overrun, Framing
	; Probably want to record them, but we at least must clean up
	push af
	ld a,#1
	out (SIOB_C),a		; RR1 please
	in a,(SIOB_C)		; clear events
	ld (siob_error),a	; Save error bits
	; Clear the latched values
	ld a,#0x30
	out (SIOB_C),a
	pop af
	ei
	reti

;
;	C interface methods
;
	.globl _sioa_txqueue
	.globl _sioa_flow_control_off
	.globl _sioa_flow_control_on
	.globl _sioa_rx_get
	.globl _sioa_error_get

	.globl _siob_txqueue
	.globl _siob_flow_control_off
	.globl _siob_flow_control_on
	.globl _siob_rx_get
	.globl _siob_error_get

	.globl _sio_dropdcd
	.globl _sio_flow
	.globl _sio_rxl
	.globl _sio_state
	.globl _sio_txl
	.globl _sio_wr5

; These are paid and exposed as arrays to C
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

;
;	Queue a byte to be sent (DI required)
;
;	l = byte
;
;	Need a way to halt processing somewhere here or a_tx ?
;	(or can we use hardware ?)
;	128 byte ring buffer aligned to upper half (rx is in lower)
;
_sioa_txqueue:
	ld a,(_sioa_txl)
	or a
	jr z, sioa_direct_maybe	; if can tx now then do
	inc a
	jp m, txa_overflow
sioa_queue:
	ld (_sioa_txl),a
	ld a,l
	ld hl,(sioa_txe)
	ld (hl),a
	inc l
	set 7,l
	ld (sioa_txe),hl
	ld l,#0
	ret
txa_overflow:
	; some kind of flag for error
	ld l,#1
	ret
sioa_direct_maybe:
	; check RR
	in a,(SIOA_C)
	and #0x04		; RX space ?
	; if space
	ld a,#1
	jr nz, sioa_queue
	; bypass the queue and kickstart the interrupt machine
	ld a,l
	out (SIOA_D),a
	ld l,#0
	ret
	; Call with DI

_sioa_flow_control_off:
	ld a,#5
	out(SIOA_C),a		; WR 5
	ld a,(_sioa_wr5)
	out (SIOA_C),a		; Turn off RTS
	ret

_sioa_flow_control_on:
	ld a,#5
	out(SIOA_C),a		; WR 5
	ld a,(_sioa_wr5)
	and #0xFD
	out (SIOA_C),a		; Turn off RTS
	ret

	; DI required
	; Returns char in L
	;
	; Caller responsible for making post buffer fetch decisions about
	; RTS
_sioa_rx_get:
	ld a,(_sioa_rxl)
	or a
	ret z
	dec a
	ld (_sioa_rxl),a
	ld hl,(sioa_rxe)
	ld a,(hl)
	inc l
	res 7,l
	ld (sioa_rxe),hl
	scf
	ld l,a
	ret

	; DI required
_sioa_error_get:
	ld hl,#sioa_error
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
_siob_txqueue:
	ld a,(_siob_txl)
	or a
	jr z, siob_direct_maybe	; if can tx now then do
	inc a
	jp m, txb_overflow
siob_queue:
	ld (_siob_txl),a
	ld a,l
	ld hl,(siob_txe)
	ld (hl),a
	inc l
	set 7,l
	ld (siob_txe),hl
	ld l,#0
	ret
txb_overflow:
	; some kind of flag for error
	ld l,#1
	ret
siob_direct_maybe:
	; check RR
	in a,(SIOB_C)
	and #0x04		; RX space ?
	; if space
	ld a,#1
	jr z, siob_queue
	; bypass the queue and kickstart the interrupt machine
	ld a,l
	out (SIOB_D),a
	ld l,#0
	ret
	; Call with DI

_siob_flow_control_off:
	ld a,#5
	out(SIOB_C),a		; WR 5
	ld a,(_siob_wr5)
	out (SIOB_C),a		; Turn off RTS
	ret

_siob_flow_control_on:
	ld a, (_siob_flow)
	or a
	ret z
	ld a,#5
	out(SIOB_C),a		; WR 5
	ld a,(_siob_wr5)
	and #0xFD
	out (SIOB_C),a		; Turn off RTS
	ret

	; DI required
	; Returns char in L
	;
	; Caller responsible for making post buffer fetch decisions about
	; RTS
_siob_rx_get:
	ld a,(_siob_rxl)
	or a
	ret z
	dec a
	ld (_siob_rxl),a
	ld hl,(siob_rxe)
	ld a,(hl)
	inc l
	res 7,l
	ld (siob_rxe),hl
	scf
	ld l,a
	ret

	; DI required
_siob_error_get:
	ld hl,#siob_error
	ld a,(hl)
	ld (hl),#0
	ld l,a
	ret

