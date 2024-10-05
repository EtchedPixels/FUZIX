;
;	The 68HC11 is a very clean processor so there isn't a lot required here
;
;	We are running with separated banks and no conventional writable
;	common so some care is needed.
;

        .globl init_early
        .globl init_hardware
        .globl program_vectors
	.globl map_kernel
	.globl map_kernel_di
	.globl map_proc
	.globl map_proc_always
	.globl map_proc_always_di
	.globl map_save_kernel
	.globl map_restore
	.globl map_proc_x
	.globl plt_interrupt

        ; exported debugging tools
        .globl plt_monitor
	.globl plt_reboot
        .globl outchar
	.globl __hard_di
	.globl __hard_ei
	.globl __hard_irqrestore


#include "kernel.def"
#include "../../cpu-68hc11/kernel-hc11.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK
; -----------------------------------------------------------------------------

	    .section .common

plt_monitor:
	    ldx #ccreg
	    jsr outstring
	    tsx
	    ldaa 2,x
	    jsr outcharhex
	    ldx #dreg
	    jsr outstring
	    tsx
            ldaa 4,x
	    jsr outcharhex
	    tsx
            ldaa 3,x
	    jsr outcharhex
	    ldx #xreg
            jsr outstring
	    tsx
            ldaa 5,x
            jsr outcharhex
	     tsx
            ldaa 6,x
            jsr outcharhex
	    ldx #yreg
            jsr outstring
	    tsx
            ldaa 7,x
            jsr outcharhex
	    tsx
            ldaa 8,x
            jsr outcharhex
            ldx #pcreg
            jsr outstring
	    tsx
            ldaa 9,x
            jsr outcharhex
	    tsx
            ldaa 10,x
            jsr outcharhex
            ldx #endtrap
            jsr outstring
	    tsx
	    xgdx
	    addd #10		; Stack frame below monitor trap
	    xgdx
	    ldaa #8
stkrow:
	    ldab #13
	    jsr outchar
	    ldab #10
	    jsr outchar
	    jsr outx
            ldab #':'
	    jsr outchar
	    ldab #' '
            jsr outchar
            clrb
stkword:
	    pshb
	    psha
	    ldd ,x
            jsr outd
	    pula
	    pulb
	    inx
	    inx
	    incb
	    cmpb #8
	    beq endrow
	    pshb
            ldab #32
	    jsr outchar
	    pulb
	    bra stkword
endrow:	    deca
	    bne stkrow
	    ldab #13
	    jsr outchar
	    ldab #10
	    jsr outchar

plt_reboot:
	    jsr map_kernel
	    jmp rebootit


ccreg:	    .byte 13,10
	    .ascii "Trap: CC = $"
	    .byte 0
dreg:       .byte 13,10
	    .ascii "      AB = $"
	    .byte 0
xreg:       .byte 13,10
	    .ascii "       X = $"
	    .byte 0
yreg:       .byte 13,10
	    .ascii "       Y = $"
	    .byte 0
pcreg:      .byte 13,10
	    .ascii "      PC = $"
	    .byte 0
endtrap:    .byte 13,10,0

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .section .text

;
;	For now do hard interrupt masking. We will want to change this to
;	only do timer masking once we have separate serial I/O buffering
;
;
;	Disable interrupts and return the old interrupt flag.
;
__hard_di:
	tpa
	tab
	sei			; Save old state in return to C
	rts
;
;	Enable interrupts
;
__hard_ei:
	cli			; on 680x cli enables IRQs!!!
	rts

;
;	Old flag is passed to use in B
;
__hard_irqrestore:
	andb #16		; IRQ flag
	bne irq_on
	cli
	rts
irq_on:
	sei
	rts


rebootit:
	ldaa	#0x10
	staa	MLATCH		; Switch to bank 0
	sei
	ldx	0xFFFE
	jmp	,x

;
;	TODO: .discard ?

;
init_early:
	rts			; stack was copied so this is ok

init_hardware:
	ldx #IOBASE
	; set up the timer
        ldd 0x0E,x		; system timer clock
	addd timer_step
	std 0x18,x		; set compare
	clr 0x20,x		; disable output pin logic
	bset 0x22,x 0x40	; turn on timer 2 interrupt
        ; set system RAM size
	ldd #512
	std ramsize
	ldd #448
	std procmem
	; set DDRA7 to A7 is an output (second CS line)
	bset 0x26,x 0x80
	ldx #vectors
	ldy #0xF000-60	; 20 vectors
nvec:
	ldd ,x
	std ,y
	inx
	inx
	iny
	iny
	cpy #0xF000
	bne nvec

	; copy bank 0 common to all other banks
	; interrupts are off - we have no valid stack during the copy

	; Put the helper into the iram (we aren't yet using the
	; iram so this is fine. The helper must be relocatable

	ldy	#copy_helper
	ldx	#0xF040
copy_copier:
	ldd	,y
	std	,x
	inx
	inx
	iny
	iny
	cpx	#0xF080
	bne	copy_copier
	jsr	0xF040

	; All common banks now have the same data in E200-EFFF

	; Set up the cached memory bank info
	jmp	map_kernel_di

;
;	TODO rewrite me for the M8 and use the 32K split for the copy 
;

copy_helper:
	ldx	#MLATCH
	ldy	#0xE200
copyall:
	ldab	#0x10
	stab	,x
	ldaa	,y
	ldab	#0x32
	; Copy ,y into each other bank
nbank:
	stab	,x
	staa	,y
	addb	#0x22
	bcc	nbank		; when we wrap we are done
	iny
	;	Next byte
	cpy	#0xF000
	bne	copyall
	ldaa	#0x10
	staa	,x		; kernel map
	rts

	;
	;  The timers require we know 16bit load/stores are generated
	;  so we do this bit in assembler
	;
	; TODO: review here and RCBUS version
	;
plt_interrupt:
	ldx #IOBASE
	ldaa #0x55
	staa 0x3a,x		; reset watchdog. We shouldn't have it on
	coma			; but we might as well reset it in case
	staa 0x3a,x
	bset 0x23,x 0x40	; clear OC2F
timer_tick_again:
	inc timer_ticks
	ldd 0x0E,x		; get the current timer
	std timer_tmp
	ldd 0x18,x		; current match
	addd timer_step		; move on 1/50th of a second
	std 0x18,x
	subd timer_tmp		; if x > timer_tmp then we are good
				; if not then we missed ticks
	bmi timer_tick_again
timer_good:
	jmp plt_event

timer_tmp:
	.word 0

program_vectors:
	;	We copy the vectors once during setup for
	;	each bank.
	rts

	;
	;	We set these up in the kernel map once
	;	the copy the common blocks to each bank
	;
	;	The monitor forwards all interrupts (except reset)
	;	to jmps in the top of memory (current bank).
	;
vectors:
	jmp	sci_intr	; SCI serial interrupt		FFD6
	jmp	nullirq		; SPI transfer complete		FFD8
	jmp	nullirq		; Pulse accumulator input edge	FFDA
	jmp	nullirq		; Pulse accumulator overflow	FFDC
	jmp	nullirq		; Timer overflow		FFDE
	jmp	nullirq		; TIC4/TOC5			FFE0
	jmp	nullirq		; TOC4				FFE2
	jmp	nullirq		; TOC3				FFE4
	jmp	common_intr	; TOC2				FFE6
	jmp	nullirq		; TOC1				FFE8
	jmp	nullirq		; TIC3				FFEA
	jmp	nullirq		; TIC2				FFEC
	jmp	nullirq		; TIC1				FFEE
	jmp	nullirq		; RTI overflow			FFF0
	jmp	common_intr	; External interrupt		FFF2
	jmp	nmi_handler	; NMI vector (XIRQ)		FFF4
	jmp	common_syscall	; SWI vector			FFF6
	jmp	trap_handler	; Illegal instruction		FFF8
	jmp	system_fail	; Watchdog timeout		FFFA
	jmp	system_fail	; COP fail			FFFC
	; Reset is not forwarded				FFFE

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .section .common

system_fail:			; we won't see this as the reset will
				; page us back to ROM. Our ROM ought to
				; report it but right now just dies TODO
nullirq:
	rti

;
;	Map the current process, do not change register values.
;	This is running on the mini shared stack so only use
;	a few bytes and always run with interrupts off
;
map_proc_always:
map_proc_always_di:
	psha
	ldaa	udata + U_DATA__U_PAGE+1	; big endian
	staa	shared_mlatch
	staa	MLATCH		; switch bank
	pula
	rts
;
;	X points to the map table of this process, preserve D
;
map_proc:
	cpx #0
	bne map_proc_2
;
;	Map in the kernel
;
map_kernel_di:
map_kernel:
	psha
	ldaa	#0x10
	staa	shared_mlatch
	staa	MLATCH		; switch to kernel
	pula
	rts

map_proc_2:
	ldaa	1,x
	staa	shared_mlatch
	staa	MLATCH
	rts
;
;	Restore mapping.
;
map_restore:
	psha
	ldaa	shared_map
	staa	shared_mlatch
	staa	MLATCH
	pula
	rts

;
;	Save the current mapping.
;
map_save_kernel:
	psha
	ldaa	shared_mlatch
	staa	shared_map
	ldaa	#0x10
	staa	shared_mlatch
	staa	MLATCH
	pula
	rts

; outchar: Wait for UART TX idle, then print the char in a without
; corrupting other registers

outchar:
	psha
outcharw:
	ldaa 0xF02E
	anda #0x80
	beq outcharw
	pula
	stab 0xF02F
	rts

	.section	.commondata

;
;	SCI variables (their location is very platform and mapping
;	specific). We bank them as our non switched space is precious
;
	.section	.data

	.globl sci_txl, sci_rxl, sci_rxp, sci_rxe, sci_txp, sci_txe, sci_err

sci_txl:
	.byte 0
sci_rxl:
	.byte 0
sci_err:
	.byte 0
sci_rxp:
	.word 0xEE00
sci_rxe:
	.word 0xEE00
sci_txp:
	.word 0xEE80
sci_txe:
	.word 0xEE80

;
;	SD card
;

	.section	.common

	.globl spi_select_port
	.globl spi_select_none
	.globl sd_spi_tx_byte
	.globl sd_spi_rx_byte
	.globl sd_spi_raise_cs
	.globl sd_spi_lower_cs
	.globl sd_spi_fast
	.globl sd_spi_slow
	.globl sd_spi_tx_sector
	.globl sd_spi_rx_sector

;
;	Select the Wiznet or other device
;	For now just hardcode for Wiznet TODO if we add an SPI clock
;	Must happen with interrupts off for now

spi_select_port:
	ldx #IOBASE
	bclr 0x00,x 0x80	; Wiznet on
	rts

spi_select_none:
	ldx #IOBASE
	bset 0x08,x 0x20
	bset 0x00,x 0x80
	rts
;
;	Select the SD card
;
sd_spi_lower_cs:
	ldx #IOBASE
	bclr 0x08,x 0x20	; port D bit 5
	rts
;
;	Deselect SD and send clocks
;
sd_spi_raise_cs:
	ldx #IOBASE
	bset 0x08,x 0x20

;
;	Send 0xFF and rx a byte
;
sd_spi_rx_byte:
	ldab #0xFF
;
;	Send a byte and return the resulting reply
;
sd_spi_tx_byte:
	ldx #IOBASE
	stab 0x2A,x		; Write to data starts the transfer
sendwt:
	brclr 0x29,x 0x80 sendwt
	ldab 0x2A,x		; Read data
	rts
;
;	Switch clock speeds
;	0 = init (100-400Khz)
;	!0 depends on card but is always faster than we can manage
;
sd_spi_slow:
	ldaa #0x52		; Master, mode 0, 125KHz
	staa 0xF028
	rts
sd_spi_fast:
	ldaa #0x50		; Master, mode 0, 1MHz
	staa 0xF028
	rts
;
;	Sector transfers (for speed)
;
;	We have to deal with thunked mappings here. That means
;	whilst we are doing the block we will have no valid stack
;
sd_spi_tx_sector:
	xgdy			; pointer into Y
	tpa
	psha			; Save int flag
	sei
	ldx	#IOBASE
	ldaa	td_raw
	beq	from_k
	deca
	beq	from_u
	ldaa	td_page
	staa	MLATCH		; Stack now invalid be careful
	bra from_k
from_u:
	ldaa	udata+U_DATA__U_PAGE+1
	staa	MLATCH		; User map
from_k:
	clra
txloop:
	ldab	,y
	stab	0x2A,x
	iny
txw1:
	brclr	0x29,x 0x80 txw1
	tst	0x2A,x		; Read data

	ldab	,y
	stab	0x2A,x
	iny
txw2:
	brclr	0x29,x 0x80 txw2
	tst	0x2A,x		; Read data
	deca
	bne	txloop
	ldaa	shared_mlatch
	staa	MLATCH		; Kernel back in , stack now valid
	pula
	tap			; restore int flag
	rts

; Same stack care needed
sd_spi_rx_sector:
	xgdy			; pointer into Y
	tpa
	psha
	sei
	ldx	#IOBASE
	ldaa	td_raw
	beq	to_k
	deca
	beq	to_u
	ldaa	td_page
	staa	MLATCH
	bra	to_k
to_u:
	ldaa	udata+U_DATA__U_PAGE+1
	staa	MLATCH
to_k:
	clra
rxloop:
	ldab	#0xFF
	stab	0x2A,x
rxw1:
	brclr	0x29,x 0x80 rxw1
	ldab	0x2A,x		; Read data
	stab	,y
	iny

	ldab	#0xFF
	stab	0x2A,x
rxw2:
	brclr	0x29,x 0x80 rxw2
	ldab	0x2A,x		; Read data
	stab	,y
	iny

	deca
	bne	rxloop
	ldaa	shared_mlatch
	staa	MLATCH
	pula	
	tap
	rts

;
;	User access routines. We need these inlined for speed to avoid
;	messing with all the stacks. For now just disable interrupts. We
;	should probably switch to the syscall_stack temporarily for the
;	longer ones ?

	.globl _uputc
	.globl _uputw
	.globl _ugetc
	.globl _ugetw
	.globl _uzero
	.globl _uget
	.globl _uput

	.section .common

_uputc:
	tsx
	ldx 2,x
	tpa
	psha
	sei
	ldaa udata+U_DATA__U_PAGE+1
	staa MLATCH
	stab 0,x
	ldaa shared_mlatch
	staa MLATCH
	pula
	tap
	rts

_uputw:
	xgdy		; data into Y
	tsx
	ldx 2,x
	tpa
	psha
	sei
	ldaa udata+U_DATA__U_PAGE+1
	staa MLATCH
	xgdy		; data back into D
	std 0,x
	ldaa shared_mlatch
	staa MLATCH
	pula
	tap
	rts

_ugetc:
	xgdx
	tpa
	psha
	ldaa udata+U_DATA__U_PAGE+1
	sei
	staa MLATCH
	ldab 0,x
	ldaa shared_mlatch
	staa MLATCH
	pula
	tap
	clra
	rts

_ugetw:
	xgdx
	tpa
	psha
	ldaa udata+U_DATA__U_PAGE+1
	sei
	staa MLATCH
	ldd 0,x
	xgdx
	ldaa shared_mlatch
	staa MLATCH
	pula
	tap
	xgdx
	rts

_uzero:
	tsx
	ldy 2,x			; size
	xgdx			; swap over as we want pointer in X
	tpa
	psha
	sei
	ldaa udata+U_DATA__U_PAGE+1
	staa MLATCH

	; Optimize, unroll etc ?
uzerolp:
	clr 0,x
	inx
	dey
	bne uzerolp
	ldaa shared_mlatch
	staa MLATCH
	pula
	tap
	clra
	rts

_uget:	
	; source D dest 3 len 5
	std __end
	xgdy		; Y is now source
	tsx
	ldd 4,x		; len
	beq nowork
	addd __end
	std __end	; stop marker
	ldx 2,x		; to
	tpa
	psha
	sei
ugetl:
	ldab	udata+U_DATA__U_PAGE+1
	stab	MLATCH
	ldab	,y
	iny
	ldaa	#0x10		; FIXME move out of loops to optimize
	staa	MLATCH
	stab	,x
	inx
	cpy	__end
	bne	ugetl
	pula
	tap
nowork:
	clrb
	clra
	rts

_uput:	
	; source D dest 3 len 5
	std __end
	xgdy		; Y is now source
	tsx
	ldd	4,x	; len
	beq	nowork
	addd	__end
	std	__end	; stop marker
	ldx	2,x	; to
	tpa
	psha
	sei
uputl:
	ldab	,y
	iny
	ldaa	udata+U_DATA__U_PAGE+1
	staa	MLATCH
	stab ,x
	inx
	ldaa	#0x10
	staa	MLATCH
	cpy	__end
	bne	uputl
	pula
	tap
	clrb
	clra
	rts


	.section .data

;
;	We should be ok not saving these on IRQ as we don't allow IRQs to
;	poke user space. TODO: move some of these to ZP
;
__end:
	.word 0
