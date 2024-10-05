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
        .globl map_proc_always
        .globl map_proc
        .globl map_kernel_di
        .globl map_kernel_restore
        .globl map_proc_always_di
        .globl map_save_kernel
        .globl map_restore
	.globl map_for_swap
	.globl _plt_reboot
	.globl _int_disabled
	.globl plt_interrupt_all
	.globl _need_resched

        ; exported debugging tools
        .globl _plt_monitor
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
	.globl ___sdcc_enter_ix

	.globl s__COMMONMEM
	.globl l__COMMONMEM

        .include "kernel.def"
        .include "../../cpu-z80/kernel-z80.def"


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

_plt_monitor:
	    ; Reboot ends up back in the monitor
_plt_reboot:
	xor a
	out (0x38), a		; ROM appears low
	rst 0			; bang

_int_disabled:
	.db 1

	.area _COMMONMEM
map_buffers:
map_kernel:
map_kernel_di:
map_kernel_restore:
	push af
	ld a,#1
map_a:
	ld (mapreg),a
	out (0x38),a
	pop af
	ret
map_proc:
	ld a,h
	or l
	jr z, map_kernel
map_proc_always:
map_proc_always_di:
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

        ld (0x0000), a   
        ld hl, #null_handler   ;   to Our Trap Handler
        ld (0x0001), hl

        ld (0x0066), a  ; Set vector for NMI
        ld hl, #nmi_handler
        ld (0x0067), hl

	;' Install the RST size helpers
	ld hl,#rstblock
	ld de,#0x8
	ld bc,#32
	ldir

_program_vectors:
plt_interrupt_all:
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


	.area _COMMONMEM

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

	.globl _sio_dropdcd
	.globl _sio_flow
	.globl _sio_rxl
	.globl _sio_state
	.globl _sio_txl
	.globl _sio_wr5

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
;	Our data is fixed in common so nothing is needed
;
.macro switch
.endm

.macro switchback
.endm

sio_handler_im2	a, SIOA_C, SIOA_D, reti
sio_handler_im2 b, SIOB_C, SIOB_D, reti

	.area _BOOT
	jp init

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
