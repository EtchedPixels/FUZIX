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

	.org 8
_int_disabled:
	.byte 0
_mem_bank:
	.byte 0
_saved_bank:
	.byte 0

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
	ld _mem_bank,#0x20
map_bank:
	push r3
	push r14
	push r15
map_bankp:
	ld r14,#0xFF
	ld r15,#0x78
	;  set I/O on
	push 2
	and 2,#0xBF
	push r3
	ld r3,_mem_bank
	lde @rr14,r3
	inc r3
	incw rr14
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
	push r3
	lde r3,@rr14
	ld _mem_bank,r3
	pop r3
	jr map_bank

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
	.export _z8_spi_tx
_z8_spi_tx:
	ret

	.export _z8_spi_rx
_z8_spi_rx:
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
	call __garg2
	ld 0xF0,r3
	ret

	.export _z8tty_status
_z8tty_status:
	ld r3,0xFB
	clr r2
	ret
