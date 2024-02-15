#include "kernel.def"
#include "../../cpu-z8/kernel-z8.def"

	.common

	.export _plt_reboot
	.export _plt_monitor

_plt_reboot:
_plt_monitor:
	jr _plt_monitor

	.export plt_interrupt_all

plt_interrupt_all:
	ret

; Hack until we get ZP working properly as register maps

	.abs

	.export _int_disabled
	.export _mem_bank
	.export istack_switched_sp

	.org 8
_int_disabled:
	.byte 0
_mem_bank:
	.byte 0
_mem_saved_bank:
	.byte 0
istack_switched_sp:
	.word 0

	.code

	.export init_early
	.export init_hardware

init_early:
		ret

init_hardware:
	ld r14,#>_ramsize
	ld r15,#<_ramsize
	ld r2,#2	; 512K
	clr r3
	lde @rr14,r2
	incw rr14
	lde @rr14,r3
	dec r2
	ld r3,#192
	ld r14,#>_procmem
	ld r15,#<_procmem
	lde @rr14,r2
	incw rr14
	lde @rr14,r3
	; TODO timers etc

	ret

	.common

	.export _program_vectors

_program_vectors:
	; TODO need to fill in low 12 bytes when we do ints
	; Do this in crt0.S instead for each code bank
	ret

	.export map_proc
	.export map_proc_always
	.export map_proc_always_di
	.export map_proc_a
	.export map_kernel
	.export map_kernel_di
	.export map_save_kernel
	.export map_restore
	.export map_for_swap

map_restore:
	ld _mem_bank, _mem_saved_bank
	jr map_bank
map_save_kernel:
	ld _mem_saved_bank, _mem_bank
map_kernel:
map_kernel_di:
	push r3
	push r14
	push r15
	ld _mem_bank,#0x20
	ld r14,#0xFF	; MMU E
	ld r15,r14
	;  set I/O on
	push 2
	and 2,#0xBF
	push r3
	ld r3,#0xC3	; Bank 0 is kernel 
	; Write to E bank
	lde @rr14,r3
	decw rr14
	clr r3		; Bank 0 is kernel code
	; Write to C bank
	lde @rr14,r3
	inc r3
	incw rr14
	lde @rr14,r3
	; Restore I/O on/off
	pop 2
	pop r15
	pop r14
	pop r3
	ret

map_proc:
map_proc_di:
	or r14,r14
	jr nz, map_notk
	or r15,r15
	jr z, map_kernel
map_notk:
	push r3
	lde r3,@rr14
	ld _mem_bank,r3
	pop r3
map_bank:
	push r3
	push r14
	push r15
map_bankp:
	ld r14,#0xFF	; MMU E
	ld r15,r14
	;  set I/O on
	push 2
	and 2,#0xBF
	push r3
	ld r3,_mem_bank
	; Write to E bank
	lde @rr14,r3
	decw rr14
	; Write to C bank
	lde @rr14,r3
	inc r3
	incw rr14
	lde @rr14,r3
	; Restore I/O on/off
	pop 2
	pop r15
	pop r14
	pop r3
	ret

map_for_swap:
map_proc_a:
	ld _mem_bank,r3
	jr map_bank

map_proc_always:
map_proc_always_di:
	push r3
	push r14
	push r15
	add r15,#<U_DATA__U_PAGE
	adc r14,#>U_DATA__U_PAGE
	lde r3,@rr14
	ld _mem_bank,3
	jr map_bankp

	.export outchar

	; Should use bit tests but to work properly we want to go IRQ
	; anyway
outchar:
	push r3
outchw:
	ld r3,0xFA		; irq status
	and r3,#16
	jr nz, outchw
	pop r3
	xor 0xF3,#0xF7		; clear status again
	; TODO wait..
	ld 240,r3
	pop r3
	ret

;
;	Bitbang SPI : TODO
;
	.export _sd_spi_tx_byte
	.export _sd_spi_rx_byte
	.export _sd_spi_tx_sector
	.export _sd_spi_rx_sector
	.export _sd_spi_lower_cs
	.export _sd_spi_raise_cs
	.export _sd_spi_fast
	.export _sd_spi_slow

_sd_spi_tx_byte:
	ld r15,#2
	call __gargr1
	; r3 is now the byte
	ld r12,3
	ld r13,r12
	or r12,#0x40	; clock high
	and r13,#0xBF	; clock low
spi_tx_r3:
	; 8 cycles of bit banging
	rlc r0
	add r3,r3
	ld 2,r12	; clock high
	rrc r0		; top bit is now bit to send
	ld 2,r3
	ld 2,r13	; clock low
	rlc r0
	add r3,r3
	ld 2,r12	; clock high
	rrc r0		; top bit is now bit to send
	ld 2,r3
	ld 2,r13	; clock low
	rlc r0
	add r3,r3
	ld 2,r12	; clock high
	rrc r0		; top bit is now bit to send
	ld 2,r3
	ld 2,r13	; clock low
	rlc r0
	add r3,r3
	ld 2,r12	; clock high
	rrc r0		; top bit is now bit to send
	ld 2,r3
	ld 2,r13	; clock low
	rlc r0
	add r3,r3
	ld 2,r12	; clock high
	rrc r0		; top bit is now bit to send
	ld 2,r3
	ld 2,r13	; clock low
	rlc r0
	add r3,r3
	ld 2,r12	; clock high
	rrc r0		; top bit is now bit to send
	ld 2,r3
	ld 2,r13	; clock low
	rlc r0
	add r3,r3
	ld 2,r12	; clock high
	rrc r0		; top bit is now bit to send
	ld 2,r3
	ld 2,r13	; clock low
	rlc r0
	add r3,r3
	ld 2,r12	; clock high
	rrc r0		; top bit is now bit to send
	ld 2,r3
	ld 2,r13	; clock low
	
	ret	
	
_sd_spi_rx_byte:
	ld r12,3
	ld r13,r12
	or r12,#0x40	; clock high
	and r13,#0xBF	; clock low
spi_rx_r3:
	ld 2,r12
	ld 2,r13
	cp 3,#0x80
	adc r3,r3	; shift left amd add in bit
	ld 2,r12
	ld 2,r13
	cp 3,#0x80
	adc r3,r3	; shift left amd add in bit
	ld 2,r12
	ld 2,r13
	cp 3,#0x80
	adc r3,r3	; shift left amd add in bit
	ld 2,r12
	ld 2,r13
	cp 3,#0x80
	adc r3,r3	; shift left amd add in bit
	ld 2,r12
	ld 2,r13
	cp 3,#0x80
	adc r3,r3	; shift left amd add in bit
	ld 2,r12
	ld 2,r13
	cp 3,#0x80
	adc r3,r3	; shift left amd add in bit
	ld 2,r12
	ld 2,r13
	cp 3,#0x80
	adc r3,r3	; shift left amd add in bit
	ld 2,r12
	ld 2,r13
	cp 3,#0x80
	adc r3,r3	; shift left amd add in bit
	com r3
	ret
	
_sd_spi_tx_sector:
	ld r15,#2
	call __gargr2
	ld r15,r3
	ld r14,r2		; pointer
	clr r2
	ld r12,3
	ld r13,r12
	or r12,#0x40	; clock high
	and r13,#0xBF	; clock low
txlp:
	call spi_tx_r3
	lde @rr14,r3
	incw rr14
	call spi_rx_r3
	lde @rr14,r3
	incw rr14
	djnz r2,txlp
	ret

_sd_spi_rx_sector:
	ld r15,#2
	call __gargr2
	ld r15,r3
	ld r14,r2		; pointer
	clr r2
	ld r12,3
	ld r13,r12
	or r12,#0x40	; clock high
	and r13,#0xBF	; clock low
rxlp:
	lde r3,@rr14
	incw rr14
	call spi_rx_r3
	lde r3,@rr14
	incw rr14
	call spi_rx_r3
	djnz r2,rxlp
	ret

_sd_spi_lower_cs:
	or 2,#0x20	; set CS1 high fist
	and 3,#0xDF	; set CS0 low CS1 high
	ret
_sd_spi_raise_cs:
	or 2,#0x20	; set CS1 high
	or 3,#0x40	; set CS0 high
_sd_spi_fast:
_sd_spi_slow:
	ret

;
; SD card block transfer helpers
; HL = address, 512 bytes map is correct
;
z8_spi_txblock:
	ret

z8_spi_rxblock:
	ret

	.export _z8_spi_fast
	.export _z8_spi_slow
;
;	Slow speed for probing - bitbang always slow
;
_z8_spi_fast:
_z8_spi_slow:
	ret

;
;	Console - hack with polling for now
;
	.export _z8tty_get
_z8tty_get:
	ld r3,0xFB
	and r3,#0x08
	jr nz, has_ch
	ld r3,#0xFF
	ld r2,r3
	ret
has_ch:
	and 0xFB,#0xFB	; clear flag
	clr r2
	ld r3,0xF0
	ret

	.export _z8tty_put
_z8tty_put:
	ld r3,#2
	call __gargr2
	ld 0xF0,r3
	ret

	.export _z8tty_status
_z8tty_status:
	ld r3,0xFB
	clr r2
	ret
