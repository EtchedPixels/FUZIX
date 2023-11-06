;
;	    Z80 Membership Card support
;
;	This first chunk is mostly boilerplate to adjust for each
;	system.
;

	; exported symbols
	.export init_early
	.export init_hardware
	.export _program_vectors
	.export map_buffers
	.export map_kernel
	.export map_kernel_di
	.export map_proc
	.export map_proc_a
	.export map_proc_always
	.export map_proc_always_di
	.export map_save_kernel
	.export map_restore
	.export plt_interrupt_all
	.export _kernel_flag
	.export _int_disabled
	.export _irq_cause

	; exported debugging tools
	.export _plt_monitor
	.export _plt_reboot
	.export outchar

#include "kernelu.def"
#include "../../cpu-z80u/kernel-z80.def"

;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

	.export _bufpool
	.buffers
_bufpool:
	.ds BUFSIZE * NBUFS

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (kept even when we task switch)
; -----------------------------------------------------------------------------
	.common
;
;	Interrupt flag. This needs to be in common memory for most memory
;	models. It starts as 1 as interrupts start off.
;
_int_disabled:
	.byte 1
;
;	This method is invoked early in interrupt handling before any
;	complex handling is done. It's useful on a few platforms but
;	generally a ret is all that is needed
;
plt_interrupt_all:
	ret

;
;	If you have a ROM monitor you can get back to then do so, if not
;	fall into reboot.
;
;	Wait for a key as the monitor does a screen clear
;
_plt_monitor:
;
;	Reboot the system if possible, halt if not. On a system where the
;	ROM promptly wipes the display you may want to delay or wait for
;	a keypress here (just remember you may be interrupts off, no kernel
;	mapped so hit the hardware).
;
_plt_reboot:
	di
	xor a
	out (0xC1),a
	rst 0

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (may be below 0x8000, only accessible when the kernel is
; mapped)
; -----------------------------------------------------------------------------
	.code

;
;	This routine is called very early, before the boot code shuffles
;	things into place. We do the ttymap here mostly as an example but
;	even that really ought to be in init_hardware.
;
init_early:
	ret

; -----------------------------------------------------------------------------
; DISCARD is memory that will be recycled when we exec init
; -----------------------------------------------------------------------------
	.discard
;
;	After the kernel has shuffled things into place this code is run.
;	It's the best place to breakpoint or trace if you are not sure your
;	kernel is loading and putting itself into place properly.
;
;	It's required jobs are to set up the vectors, ramsize (total RAM),
;	and procmem (total memory free to processs), as well as setting the
;	interrupt mode but *not* enabling interrupts. Many platforms also
;	program up support hardware like PIO and CTC devices here.
;
init_hardware:
	ld hl, 544			; 512 + 32
	ld (_ramsize), hl
	ld hl, 480
	ld (_procmem), hl

	; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
	ld hl, #0
	push hl
	call _program_vectors
	pop hl

	im 1 ; set CPU interrupt mode

	ret

;
;	Bank switching unsurprisingly must be in common memory space so it's
;	always available.
;
	.common

mapsave:   .byte 0	; Saved copy of the previous map (see map_save)

_kernel_flag:
	.byte 1	; We start in kernel mode

intcount:
	.byte 100
_irq_cause:
	.byte 0

;
;	1000 interrupts/second (ouch)
;
fast_interrupt_handler:
	push af			; 11
	in a,(0x40)		; 11
	and #0x40		; 7
	jr z,nottimer		; 7/12
	out (0x40),a		; 11
	ld a,(intcount)		; 13
	dec a			; 4
	jr z,timerevent		; 7 / 12
	ld (intcount),a		; 13
	pop af			; 10
	ei			; 4
	ret			; 10	 - No NMI or M1 users so
				; 	   use ret not reti for
				; 	   speed
;
;	Our normal overhead per event is 108 clocks, with a 4MHz CPU that's
;	still 1/40th of the CPU time. Supporting the bit bang port would
;	raise the cost way higher still.
;
;	We need to do softints really as otherwise our clock slides because
;	many events are multiple ticks long at this rate.
;
timerevent:
	inc a
	ld (_irq_cause),a
	ld a, 100
	ld (intcount),a
	pop af
	jp interrupt_handler
nottimer:
	ld a, 2
	ld (_irq_cause),a
	pop af
	jp interrupt_handler
;
;	This is invoked with a NULL argument at boot to set the kernel
;	vectors and then elsewhere in the kernel when the kernel knows
;	a bank may need vectors writing to it.
;
_program_vectors:
	; we are called, with interrupts disabled, by both newproc() and crt0
	; will exit with interrupts off
	di ; just to be sure
	pop de ; temporarily store return address
	pop hl ; function argument -- base page number
	push hl ; put stack back as it was
	push de

	call map_proc

	; now install the interrupt vector at 0x0038
	ld a, #0xC3 ; JP instruction
	ld (0x0038), a
	ld hl, #fast_interrupt_handler
	ld (0x0039), hl

	; set restart vector for FUZIX system calls
	ld (0x0030), a   ;  (rst 30h is unix function call vector)
	ld hl, #unix_syscall_entry
	ld (0x0031), hl

	ld (0x0000), a   
	ld hl, #null_handler   ;   to Our Trap Handler
	ld (0x0001), hl

	; and fall into map_kernel

;
;	Mapping set up for the Z80 Membership Card
;
;	The low 32K is banked and high 32K fixed.
;
;	We know the ROM mapping is already off
;
;	The _di versions of the functions are called when we know interrupts
;	are definitely off. In our case it's not useful information so both
;	symbols end up at the same code.
;
map_buffers:
	; for us no difference. We could potentially use a low 32K bank
	; for buffers but it's not clear it would gain us much value
map_kernel_di:
map_kernel:
	push af
	xor a
	out (0xCC),a	; Modem control lines wired to banking
	pop af
	ret
	; map_proc is called with HL either NULL or pointing to the
	; page mapping. Unlike the other calls it's allowed to trash AF
map_proc:
	ld a, h
	or l
	jr z, map_kernel
map_proc_hl:
	ld a, (hl)			; and fall through
	;
	; With a simple bank switching system you need to provide a
	; method to switch to the bank in A without corrupting any
	; other registers. The stack is safe in common memory.
	; For swap you need to provide what for simple banking is an
	; identical routine.
map_proc_a:			; used by bankfork
	out (0xCC),a
	ret

	;
	; Map the current process into memory. We do this by extracting
	; the bank value from u_page.
	;
map_proc_always_di:
map_proc_always:
	push af
	push hl
	ld hl, #_udata + U_DATA__U_PAGE
	call map_proc_hl
	pop hl
	pop af
	ret
	;
	; Save the existing mapping and switch to the kernel.
	; The place you save it to needs to be in common memory as you
	; have no idea what bank is live. Alternatively defer the save
	; until you switch to the kernel mapping
	;
map_save_kernel:
	push af
	in a,(0xCC)		; FIXME: check can read back
	and #0x0F
	ld (mapsave), a
	xor a
	out (0xCC),a
	pop af
	ret
	;
	; Restore the saved bank. Note that you don't need to deal with
	; stacking of banks (we never recursively use save/restore), and
	; that we may well call save and decide not to call restore.
	;
map_restore:
	push af
	ld a,(mapsave)
	out (0xCC),a
	pop af
	ret
	
	;
	; Used for low level debug. Output the character in A without
	; corrupting other registers. May block. Interrupts and memory
	; state are undefined
	;
outchar:
	push af
twait:	    in a,(0xCD)
	bit 5,a
	jr z, twait
	pop af
	out (0xC8),a
	ret

;
;	SPI interface. Bitbanged but with a little bit of help. Slow though
;	and more like a floppy than a hard disk
;

	.export _sd_spi_tx_byte
	.export _sd_spi_rx_byte
	.export _sd_spi_tx_sector
	.export _sd_spi_rx_sector

_sd_spi_tx_byte:
	push bc
	ld a,l
	rlca
	ld c, 0xC0
	out (0xC5),a
	rlca
	in b,(c)
	out (0xC5),a
	rlca
	in b,(c)
	out (0xC5),a
	rlca
	in b,(c)
	out (0xC5),a
	rlca
	in b,(c)
	out (0xC5),a
	rlca
	in b,(c)
	out (0xC5),a
	rlca
	in b,(c)
	out (0xC5),a
	rlca
	in b,(c)
	out (0xC5),a
	in b,(c)
	pop bc
	ret
_sd_spi_rx_byte:
	push bc
	ld a, 1
	out (0xC5),a		; send FF
	in a,(0xCE)
	rlca
	rl l
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl l
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl l
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl l
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl l
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl l
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl l
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl l
	in a,(0xC0)
	ld a,l
	cpl
	ld l,a
	ld h,0
	pop bc
	ret
sd_spi_rx_sec:
	ld b, 0
	ld a, 1
	out (0xC5),a		; send FF
spi_rx_loop:
	in a,(0xCE)
	rlca
	rl e
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl e
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl e
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl e
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl e
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl e
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl e
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl e
	ld a,e
	cpl

	ld (hl),a
	inc hl

	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl e
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl e
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl e
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl e
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl e
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl e
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl e
	in a,(0xC0)
	in a,(0xCE)
	rlca
	rl e
	ld a,e
	cpl
	ld (hl),a
	inc hl
	in a,(0xC0)
	djnz spi_rx_loop
	ret
sd_spi_tx_sec:
	ld bc, 0xC0
spi_tx_loop:
	ld a,(hl)
	inc hl

	rlca			; need bit 7 in 0
	out (0xC5),a
	rlca
	in e,(c)
	out (0xC5),a
	rlca
	in e,(c)
	out (0xC5),a
	rlca
	in e,(c)
	out (0xC5),a
	rlca
	in e,(c)
	out (0xC5),a
	rlca
	in e,(c)
	out (0xC5),a
	rlca
	in e,(c)
	out (0xC5),a
	rlca
	in e,(c)
	out (0xC5),a

	ld a,(hl)
	inc hl

	rlca
	in e,(c)
	out (0xC5),a
	rlca
	in e,(c)
	out (0xC5),a
	rlca
	in e,(c)
	out (0xC5),a
	rlca
	in e,(c)
	out (0xC5),a
	rlca
	in e,(c)
	out (0xC5),a
	rlca
	in e,(c)
	out (0xC5),a
	rlca
	in e,(c)
	out (0xC5),a
	rlca
	in e,(c)
	out (0xC5),a
	in e,(c)

	djnz spi_tx_loop
	ret

	.export _sd_spi_rx_sector
	.export _sd_spi_tx_sector

; No swap devices
sd_map:
	ld a,(_td_raw)
	or a
	jp nz, map_proc_always
	ret

_sd_spi_rx_sector:
	pop de
	pop hl
	push hl
	push de
	push bc
	call sd_map
	call sd_spi_rx_sec
	pop bc
	ld hl,0
	jp map_kernel
_sd_spi_tx_sector:
	pop de
	pop hl
	push hl
	push de
	push bc
	call sd_map
	call sd_spi_tx_sec
	pop bc
	ld hl,0
	jp map_kernel
