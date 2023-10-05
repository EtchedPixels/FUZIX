;
;	    RCBUS 68HC11 platform functions
;
;	The 68HC11 is a very clean processor so there isn't a lot required here
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
	    .globl copy_common
	    .globl cur_map

            ; exported debugging tools
            .globl plt_monitor
	    .globl plt_reboot
            .globl outchar
	    .globl __hard_di
	    .globl __hard_ei
	    .globl __hard_irqrestore


#include "kernel.def"
#include "../kernel-hc11.def"

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
	    ldd 10,x
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

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .section .text

rebootit:
	    clr 0xFE7B		; top 16K to ROM 0
	    ldx 0xFFFE		; restart vector
	    jmp ,x		; and go

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
	    ldaa #0x3A		; SPI outputs (in case ROM didn't set it up)
	    staa 0x08,x		;
            ; set system RAM size
	    ldd #512
	    std ramsize
	    ldd #448
	    std procmem
            jmp program_vectors_k

	     ;
	     ;	The timers require we know 16bit load/stores are generated
	     ;  so we do this bit in assembler
	     ;
plt_interrupt:
	    ldx #IOBASE
	    ldaa #0x55
	    staa 0x3a,x		; reset watchdog. We shouldn't have it on
	    coma		; but we might as well reset it in case
	    staa 0x3a,x
	    bset 0x23,x 0x40	; clear OC2F
timer_tick_again:
	    inc timer_ticks
	    ldd 0x0E,x		; get the current timer
	    std timer_tmp
	    ldd 0x18,x		; current match
	    addd timer_step	; move on 1/50th of a second
	    std 0x18,x
	    subd timer_tmp	; if x > timer_tmp then we are good
				; if not then we missed ticks
	    bmi timer_tick_again
timer_good:
	    jmp plt_event

timer_tmp:
	    .word 0

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .section .common

program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
	    ;
	    ; our C caller will invoke us with the pointer in D
	    ; just pass it on

	    xgdx
	    jsr map_proc
	    ;
	    ; We need to split these up
	    ;

program_vectors_k:
	    ldd #system_fail
	    std 0xFFFC		; COP fail
	    std 0xFFFA		; watchdog timeout
	    ldd #trap_handler
	    std 0xFFF8		; illegal instruction trap
	    ldd #unix_syscall_entry
	    std 0xFFF6		; SWI vector
	    ldd #nmi_handler
	    std 0xFFF4		; NMI vector
	    ldd #interrupt_handler
	    std 0xFFF2		; Interrupt from rcbus
	    std 0xFFE6		; TOC2
	    ldd #sci_intr
	    std 0xFFD6		; Interrupt from SCI serial
	    ldd #nullirq
	    std 0xFFF0		; Interrupt from RTI (RTI overflow)
	    std 0xFFEE		; TC1
	    std 0xFFEA		; TC2
	    std 0xFFE8		; TOC1
	    std 0xFFE4		; TOC3
	    std 0xFFE2		; TOC4
	    std 0xFFE0		; TIC4/TOC5
	    std 0xFFDE		; Timer overflow
	    std 0xFFDC		; Pulse accumulator overflow
	    std 0xFFDA		; Pulse accumulator input edge
	    std 0xFFD8		; SPI transfer complete
	    jmp map_kernel

system_fail:			; we won't see this as the reset will
				; page us back to ROM. Our ROM ought to
				; report it but right now just dies TODO
nullirq:
	    rti

;
;	Copy the common memory into the page given (in D)
;
copy_common:
	   bsr map_for_swap	; map the page in B at 0x4000
	   ldx #__common_start
	   ldy #__common_start-0x8000	; offset into new page
commoncp:
	   ldd ,x
           std ,y
           inx
           inx
           iny
           iny
	   cpx #__common_end
	   bne commoncp
	   bra map_kernel	; restore the map

;
;	Map the current process, do not change register values
;
map_proc_always:
map_proc_always_di:
	    pshx
	    ldx #udata + U_DATA__U_PAGE
	    jsr map_bank
	    pulx
	    rts
;
;	X points to the map table of this process, preserve D
;
map_proc:
	    cpx #0
	    bne map_proc_2
;
;	Map in the kernel below the current common, all registers preserved
;	the kernel lives in 32/33/34/35
;	Later we'll be clever and stuff _DISCARD and the copy blocks there or
;	something (that would also let us put RODATA in
;	common area just to balance out memory usages).
;
map_kernel_di:
map_kernel:
	    pshx
				; Common is left untouched
	    ldx #kmap		; Kernel RAM
	    jsr map_bank
	    pulx
	    rts
kmap:
	    .byte 0x20
	    .byte 0x21
	    .byte 0x22

;
;	Switch the low 48K of the memory map. We switch the top bank in
;	the task switcher as it contains stacks and other interesting items.
;
;	This all works because the only time we access memory belonging to
;	another process is when we swap and swap is handled by a different
;	mapping technique
;
map_proc_x:
	    pshb
	    ldab #P_TAB__P_PAGE_OFFSET
	    abx
	    bra map_in
map_proc_2:
map_bank:
	    pshb
map_in:
	    ; We must load these both before the map changes because we
	    ; might be unmapping the table X points to
	    ldab 2,x
	    ldx ,x
	    stab cur_map+2
	    stab 0xFE7A
	    stx cur_map
	    stx 0xFE78
	    pulb
	    rts

;
;	Map the page in B into the 16K window for swap
;
map_for_swap:
	    stab cur_map+1
	    stab 0xFE79
	    rts

;
;	Restore mapping. This may not be sufficient. We may need to do a
;	careful 4 byte save/restore if we do clever stuff in future. We only
;	ever use this from the current kernel map when returning to kernel
;	so this is fine.
;
map_restore:
	    pshx
	    ldx #saved_map
	    jsr map_bank
	    pulx
	    rts

;
;	Save the current mapping.
;	May not be sufficient if we want IRQs on while doing page tricks
;
map_save_kernel:
	    pshb
	    psha
	    ldab cur_map+2
	    stab saved_map+2	; always save the map in the right commonmem
	    ldd cur_map
	    std saved_map
	    jsr map_kernel
	    pula
	    pulb
	    rts

cur_map:    .byte 0
	    .byte 0
	    .byte 0
saved_map:  .byte 0
	    .byte 0
	    .byte 0

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

;
;	SCI variables (their location is very platform and mapping
;	specific). For us commonmem is sufficient. We put the buffer in
;	the iram at F100 for now
;

	.globl sci_txl, sci_rxl, sci_rxp, sci_rxe, sci_txp, sci_txe, sci_err

sci_txl:
	.byte 0
sci_rxl:
	.byte 0
sci_err:
	.byte 0
sci_rxp:
	.word 0xF100
sci_rxe:
	.word 0xF100
sci_txp:
	.word 0xF180
sci_txe:
	.word 0xF180

;
;	Disk copier (needs to be in common), call with ints off
;	for now
;
;	TOS = ptr, length always 512, src and page in globals
;

	.globl hd_read_data
	.globl hd_write_data
	.globl hd_map
	.globl hd_swap

hd_read_data:
	xgdx

	;
	;	We must flip banks before we play mmu pokery, or it will
	; undo all our work. This means our variables must be commondata
	; and we must reference them before we map_proc
	ldab hd_map
	beq hd_kmap
	deca
	beq hd_umap
	ldab hd_swap
	bsr map_for_swap
	bra hd_kmap
hd_umap:
	jsr map_proc_always
hd_kmap:
	clrb
	jsr hd_read256
	jsr hd_read256
	jsr map_kernel
	rts

hd_read256:
	ldaa 0xFE10
	staa ,x
	inx
	incb
	bne hd_read256
	rts

hd_write_data:
	xgdx
	;
	;	We must flip banks before we play mmu pokery, or it will
	; undo all our work. This means our variables must be commondata
	; and we must reference them before we map_proc
	;
	ldab hd_map
	beq hd_kmapw
	deca
	beq hd_umapw
	ldab hd_swap
	bsr map_for_swap
	bra hd_kmapw
hd_umapw:
	jsr map_proc_always
hd_kmapw:
	clrb
	jsr hd_write256
	jsr hd_write256
	jsr map_kernel
	rts

hd_write256:
	ldaa ,x
	staa 0xFE10
	inx
	incb
	bne hd_write256
	rts

hd_map:
	.byte 0
hd_swap:
	.byte 0

;
;	SD card
;

	.globl sd_spi_transmit_byte
	.globl sd_spi_receive_byte
	.globl sd_spi_raise_cs
	.globl sd_spi_lower_cs
	.globl sd_spi_clock
	.globl sd_spi_transmit_sector
	.globl sd_spi_receive_sector

;
;	Select the SD card
;
sd_spi_lower_cs:
	ldx #0xF000
	bclr 0x08,x 0x20	; port D bit 5
	rts
;
;	Deselect SD and send clocks
;
sd_spi_raise_cs:
	ldx #0xF000
	bset 0x08,x 0x20
;
;	Send 0xFF and receive a byte
;
sd_spi_receive_byte:
	ldab #0xFF
;
;	Send a byte and return the resulting reply
;
sd_spi_transmit_byte:
	ldx #0xF000
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
sd_spi_clock:
	tstb
	bne clkfast
	ldaa #0x52		; Master, mode 0, 125KHz
	staa 0xF028
	rts
clkfast:
	ldaa #0x50		; Master, mode 0, 1MHz
	staa 0xF028
	rts
;
;	Sector transfers (for speed)
;
sd_spi_transmit_sector:
	ldx #blk_op
	ldy ,x
	ldaa 2,x
	beq from_k
	deca
	beq from_u
	ldab 3,x
	bsr map_for_swap
	bra from_k
from_u:
	bsr map_proc_always
from_k:
	clra
	ldx #0xF000
txloop:
	ldab ,y
	stab 0x2A,x
	iny
txw1:
	brclr 0x29,x 0x80 txw1
	tst 0x2A,x		; Read data

	ldab ,y
	stab 0x2A,x
	iny
txw2:
	brclr 0x29,x 0x80 txw2
	tst 0x2A,x		; Read data
	deca
	bne txloop
	bra map_kernel

sd_spi_receive_sector:
	ldx #blk_op
	ldy ,x
	ldaa 2,x
	beq to_k
	deca
	beq to_u
	ldab 3,x
	bsr map_for_swap
	bra to_k
to_u:
	bsr map_proc_always
to_k:
	clra
	ldx #0xF000
rxloop:
	ldab #0xFF
	stab 0x2A,x
rxw1:
	brclr 0x29,x 0x80 rxw1
	ldab 0x2A,x		; Read data
	stab ,y
	iny

	ldab #0xFF
	stab 0x2A,x
rxw2:
	brclr 0x29,x 0x80 rxw2
	ldab 0x2A,x		; Read data
	stab ,y
	iny

	deca
	bne rxloop
	bra map_kernel
