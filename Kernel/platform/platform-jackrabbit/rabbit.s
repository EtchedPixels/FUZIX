;
;	Skeleton for Rabbit 2000
;
		.module rabbit

		.area _COMMONMEM

		.globl _plt_reboot
		.globl _plt_monitor

		.include 'kernel.def'
		.include '../../cpu-r2k/kernel-rabbit.def'

_plt_reboot:
_plt_monitor:
		; Force a hardware level reset using the watchdog
		ld a,#0x53
		ioi
		ld (WDTCR),a
		jr _plt_monitor

		.globl plt_interrupt_all

plt_interrupt_all:
		ret

		.area _COMMONDATA

		.globl _int_disabled

_int_disabled:
		.db 1


		.area _CODE

		.globl init_early
		.globl init_hardware

		.globl _ramsize
		.globl _procmem
		.globl _iir

init_early:
		ret

init_hardware:
		; FIXME: should get this from firmware
		ld hl,#128
		ld (_ramsize),hl
		ld de,#64
		or a,a
		sbc hl,de
		ld (_procmem),hl

		;
		; Set up the interrupt table
		;

		ld hl,#_iir
		ld de,#0
		ld bc,#256
doldir:
		ldi
		jp lo,doldir

		xor a,a
		ld iir,a
		ld eir,a

		ret

		.area _COMMONMEM

		.globl _program_vectors

_program_vectors:
		ret


		.globl map_proc
		.globl map_proc_always
		.globl map_proc_always_di
		.globl map_proc_a
		.globl map_kernel
		.globl map_kernel_di
		.globl map_save_kernel
		.globl map_restore
		.globl map_for_swap

		.globl _kdataseg


		.globl _udata

map_kernel:
map_kernel_di:
		push af
		ld a,(_kdataseg)
		ioi
		ld (DATASEG),a
		pop af
		ret

map_proc:
map_proc_di:
		ld a,h
		or a,l
		jr z, map_kernel
		ld a,(hl)
map_for_swap:
map_proc_a:
		ioi
		ld (DATASEG),a
		ret
map_proc_always:
map_proc_always_di:
		push af
		ld a,(_udata + U_DATA__U_PAGE)
		ioi
		ld (DATASEG),a
		pop af
		ret
map_save_kernel:
		push af
		ioi
		ld a, (DATASEG)
		ld (map_store),a
		ld a,(_kdataseg)
		ioi
		ld (DATASEG),a	; Must match map_kernel
		pop af
		ret
map_restore:
		push af
		ld a,(map_store)
		ioi
		ld (DATASEG),a
		pop af
		ret
map_store:
		.db 0
_kdataseg:
		.db 0

		.globl outchar

outchar:
		push af
outwait:
		ioi
		ld a, (SASR)
		bit 2,a
		jr nz,outwait
		pop af
		ioi
		ld (SADR),a
		ret

;
;	Low level Rabbit 2000 primitives to use Port B for SPI.
;
;	Assumptions: The port is configured and the clock is set up
;	properly. That is you have the serial pins enabled and you have
;	timer A5 programmed and running.
;
;	We don't support simultaneous rx/tx although the R2000 can do it
;	at lower speeds with some mucking about. SD doesn't need the full
;	duplex behaviour.
;
;	Remember the bit ordering !


		.area _CODE

		.globl _rabbit_spi_tx
		.globl _rabbit_spi_rx
		.globl _rabbit_spi_slow
		.globl _rabbit_spi_fast

		.globl rabbit_spi_rxblock
		.globl rabbit_spi_txblock
;
_rabbit_spi_tx:
		ld e,l
rabbit_spi_tx:
		; Finish any previous transaction activity
		ioi
		ld a,(SBSR)
		bit 7,a
		jr nz,_rabbit_spi_tx
		ld a,e
		ioi
		ld (SBDR),a
		ld a, #0x8C		; send, our clock (ie master)
		ioi
		ld (SBCR),a
txwait:		ioi
		ld a,(SBSR)
		bit 3,a
		jr nz,txwait
txwait2:	ioi
		ld a,(SBSR)
		bit 2,a
		jr nz,txwait2
		ret

_rabbit_spi_rx:
		; Fire up the receiver and clock
		ld a,#0x4C		; receive (using PC4, 0x5C is PD4)
		ioi
		ld (SBCR),a
		; Wait for the data byte to appear
		; This is bounded by the synchronous clock
rxwait:
		ioi
		ld a,(SBSR)
		and a,#0x80
		jr z,rxwait
		ioi
		ld a,(SBDR)
		ld l,a
		ret

		;
		; SD card block transfer helpers
		; HL = address, 512 bytes map is correct
		;
rabbit_spi_txblock:
		ld b,#0
txloop:
		ld e,(hl)
		inc hl
		call rabbit_spi_tx
		ld e,(hl)
		inc hl
		call rabbit_spi_tx
		djnz txloop
		ret

rabbit_spi_rxblock:
		ld b,#0
		ex de,hl
rxloop:
		call _rabbit_spi_rx
		ld (de),a
		inc de
		call _rabbit_spi_rx
		ld (de),a
		inc de
		djnz rxloop
		ret

;
;	Slow speed for probing
;
_rabbit_spi_slow:
		ld a,#30		; about 25KHz with a 29.5MHz clock
		ioi
		ld (TAT5R),a
		ret

;
;	Set the timer for fast SPI.
;
;	We are limited to about 7.5MHz
;
_rabbit_spi_fast:
		xor a,a
		ioi
		ld(TAT5R),a
		ret

;
;	Real time clock interface. This is a 48bit counter running at 32KHz
;
;	Convention is that clock 0 is Jan 1 1980
;
;	The caller needs to ensure that they get two identical copies
;

		.globl _rabbit_read_rtc

_rabbit_read_rtc:
		ex de,hl		; C argument is the buffer
		ld hl,#RTC0R
		ioi
		ld (hl),#0
		nop			; R2000 erratum
		ipset3
		ioi
		ldi
		ioi
		ldi
		ioi
		ldi
		ioi
		ldi
		ioi
		ldi
		ioi
		ldi
		ipres
		ret

;
;	Watchdog timer
;
;	0x5A = 2 second, 0x57 = 1 second, 0x59 = 500m, 0x53 = 250ms
;
		.globl _rabbit_bop_watchdog

_rabbit_bop_watchdog:
		ld a,l
		ioi
		ld (WDTCR),a
		ret
;
;	Periodic interrupt. Annoyingly fixed at 2KHz it seems
;	This is the same for all rabbit 2k so maybe it belongs in the
;	lowlevel-rabbit code ?
;
		.globl ticktock

		.globl _ticker
		.globl interrupt_handler
ticktock:
		push af
		ioi
		ld a,(GCSR)
		push hl
		ld hl,#_ticker
		inc (hl)
		jr z,tickwrap
		pop hl
		pop af
		ipres
		ret
tickwrap:	ld (hl),#11	; slightly off for 200/second but the rtc
				; will compensate just fine. We need the
				; speed for the serial queues
		jp interrupt_handler		

;
;	Serial queues. The basic logic for them is given in the manual
;	We do a simple optimization - our queues never cross a page. With
;	the free d we then read the char early
;
;	Our platform interrupt will pick up the queue each timer tick
;

.macro serial	X Y

		.area _COMMONMEM

		.globl _ser'X'_q
		.globl _ser'X'_rxbuf

		.globl ser'X'_rx
		.globl ser'X'_tx

ser'X'_rx:
		push hl
		push de
		ioi
		ld a,(S'Y'DR)
		ld d,a
		ld hl,#_ser'X'_q
		ld a,(hl)
		ld e,a
		inc hl
		cp a,(hl)
		jr z,ser'X'_rx_over
		inc a
		and a, #63
		dec hl
		ld (hl),a
		ipres
		; work around stupid linker bug
		ld hl, #_ser'X'_rxbuf
		ld l,e
		ld (hl),d
ser'X'_rx_over:		; for now do nothing clever
		pop de
		pop hl
		pop af
		ret

ser'X'_tx:		; really 'transmit and other...'
			; we don't do transmit interrupts yet but we
			; can't turn them off alone 8(
		ioi
		ld (S'Y'SR),a
		pop af
		ipres
		ret

		.globl _ser'X'_setup
		.globl _ser'X'_get

;
;	00PPMMII
;
;	PP 00 port C 01 port D 1X off
;	MM 00 8bit 01 7bit 10 clocked ext, 11 clocked int
;	II 00 no int 01 pri 1 10 pri 2 11 pri 3
;
_ser'X'_setup:
		ld a,l
		ioi
		ld (S'Y'CR),a
		ret

_ser'X'_get:
		ld hl,#_ser'X'_q
		ld a,(hl)		; out ptr
		ld e,a
		inc hl
		inc a
		cp a,(hl)		; in ptr
		jr z, ser'X'_empty
		dec hl
		and #63
		ipset3
		ld (hl),a
		ld hl,#_ser'X'_rxbuf	; work around dumb linker limit
		ld l,e
		ld l,(hl)
		ld h,#0
		ipres
		ret
ser'X'_empty:
		ld hl,#0xffff
		ret
_ser'X'_q:
		.db 0
		.db 1

		.area _SERIALBUF

_ser'X'_rxbuf:	.ds 64

		.area _COMMONMEM
.endm

serial a A
serial c C
serial d D
