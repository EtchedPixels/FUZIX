;
;	Cromemco hardware support
;

	.module cromemco

	; exported symbols
	.globl init_early
	.globl init_hardware
	.globl _program_vectors
	.globl plt_interrupt_all

	.globl map_kernel_low
	.globl map_save_low
	.globl map_restore_low
	.globl map_user_low
	.globl map_page_low

	.globl _plt_reboot

	.globl _plt_doexec

	.globl _int_disabled

	; exported debugging tools
	.globl _plt_monitor
	.globl outchar

	; imported symbols
	.globl _ramsize
	.globl _procmem

	.globl s__COMMONMEM
	.globl l__COMMONMEM

	.globl syscall_platform
	.globl unix_syscall_entry
	.globl my_nmi_handler
	.globl interrupt_handler
	.globl _doexit
	.globl kstack_top
	.globl istack_top
	.globl istack_switched_sp
	.globl _panic
	.globl _need_resched
	.globl _ssig
	.globl _udata

	.globl _set_irq
	.globl _spurious
	.globl _irqvec
	.globl interrupt_high

	.globl outcharhex
	.globl outhl, outde, outbc
	.globl outnewline
	.globl outstring
	.globl outstringhex

	.include "kernel.def"
	.include "../../cpu-z80/kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF200 upwards)
; -----------------------------------------------------------------------------
        .area _COMMONMEM

;
;	Must be page aligned
;
_irqvec:
	; 0
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 16
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 32
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 48
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 64
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 80
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 96
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 112
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 128
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 144
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 160
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 176
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 192
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 208
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 224
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 240
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	.word _spurious
	; 256
	.byte 0


_plt_reboot:
_plt_monitor:
	jr _plt_monitor
plt_interrupt_all:
	ret

_int_disabled:
	.db 1

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
	.area _CODE

init_early:
        ret

init_hardware:
        ; set system RAM size
        ld hl, #448		; Assuming fully loaded for now
        ld (_ramsize), hl
        ld hl, #(448-64)	; 64K for kernel
        ld (_procmem), hl

	; Write the stubs for our bank
	ld hl,#stubs_low
	ld de,#0x0000
	ld bc,#0x67
	ldir

	ld a,#0x40		; enable interrupt mode
	out (2),a
	ld a,#0x70
	out (3),a		; serial 0 timer 4 tbe rda
	ld a, #156		; ticks for 10Hz (9984uS per tick)
	out (8), a		; 10Hz timer on

	ld hl,#_irqvec
	ld a,h			; deal with linker limits
	ld i,a
	im 2
        ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

	.area _COMMONMEM

;
;	We switch in one go so we don't have these helpers. This means
;	we need custom I/O wrappers and custom usercopy functions.
;
map_save_low:
map_kernel_low:
map_restore_low:
map_user_low:
map_page_low:
	ret

_program_vectors:
	pop de
	pop hl
	push hl
	push de
	push ix
	; From kernel to user bank
	ld d,#1
	ld e,(hl)
	; Write the stubs for our bank
	ld hl,#stubs_low
	ld ix,#0x0000
	ld bc,#0x67
	call ldir_far
	pop ix
	ret

;
;	Called when we get an unexpected vector - just ack and return
;
_spurious:			; unexpected IRQ vector - handy breakpoint
	ei
	reti

;
;	Set an interrupt table entry. Done in asm as we want to write it
;	through to all the banks.
;
_set_irq:
	pop hl
	pop de
	pop bc
	push bc
	push de
	push hl
	ld hl,#_irqvec
	di
	add hl,de
	ld a,#0x81
	out (0x40),a		; write to all banks, read kernel
	ld (hl),c
	inc hl
	ld (hl),b
	ld a,#0x01
	out (0x40),a		; kernel mapping back
	ld a,(_int_disabled)
	or a
	ret nz
	ei
	ret

; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
	push af
outcharl:
	in a, (0x00)
	bit 7,a
	jr z, outcharl
	pop af
	out (0x01), a
        ret

;
;	Low level pieces for floppy driver
;
	.globl _fd_reset
	.globl _fd_seek
	.globl _fd_operation
	.globl _fd_map
	.globl _fd_cmd

;
;	On entry c holds the bits from 0x34, bde must be preserved
;	a may be trashed
;
fd_setup:
	bit	2,c			; motor off
	jr	nz, motor_stopped
	ld	hl,#400			; delay
	; If the side has changed we need to allow for a head load delay
	; FIXME
	; We don't switch config mid flow (we close/reopen) so we probably
	; don't need to consider a config change delay here
	bit	5,c			; head load
	ret	nz
	jp	delayhl1		; wait 40ms for head load
motor_stopped:
	ld	hl,#0x4E20		; two seconds for spin up
	jp	delayhl1

;
;	Passed a buffer with the controller states in
;	Set up the state, do a restore and then clean up
;
;	Does not work on all 8" types attachable to the FDC16 ?
;
_fd_reset:
	in	a,(0x34)
	ld	c,a
	ld	a,(_fd_cmd + 1)
	out 	(0x34),a	; set control bits
	call	fd_setup	; get the drive spinning
	ld	a,(_fd_cmd + 5)	; restore with delay bits
	out	(0x30),a	; issue the correct command
fd_restore_wait:
	in	a,(0x34)	; wait for error or done
	bit	2,a		; motor stopped
	jr	nz, nodisk	; that's an error
	rra			; end of job ?
	jr	nc, fd_restore_wait	; nope - try again
	call	delayhl		; short sleep for the controller
	in	a,(0x34)	; read the status
	and	#0x98		; bits that mean an error
reta:
	ld	l,a		; report them back in HL
	ret
nodisk:
	ld	l,#0xFF
	ret


;
;	Passed a control buffer
;	0: cmd
;	1: bit 0 set if writing
;	2: 0x34 bits
;	3/4: data
;	5: delay information (used by seek not read/write)
;
;
_fd_operation:
	;
	;	Set up the registers in order. The aux register is already
	;	done by our caller
	;
	ld	hl,#_fd_cmd
	ld	b,(hl)			; command
	inc	hl
	inc	hl
	;
	;	Make sure we patch across all instances
	;
	;
	;	We have to disable interrupts before we write the
	;	command register when doing a data transfer otherwise an
	;	interrupt can lose us bytes and break the transfer. Do it
	;	a spot earlier so we can also do the patching under it
	;
	di
	bit	0,(hl)			; read or write ?
	ld	a, #0x81		; write all common, read kernel
	out	(0x40),a
	; patch patch1/2 according to the transfer direction
	ld	a,#0xA2			; ini
	jr	z, patchit
	ld	a,#0xA3			; outi
patchit:
	ld	(patch1+1),a
	ld	(patch2+1),a
	ld	a,#0x01
	out	(0x40),a		; kernel map
	in	a,(0x34)		; check head status
	bit	5,a
	jr 	nz, nomod		; loaded
	set	2,b			; set head-load bit in command
nomod:
	;
	;	Now figure out what set up time is needed. There are
	;	three cases
	;
	;	1. Motor is stopped
	;	2. Motor is running head is unloaded
	;	3. Motor is running head is loaded
	ld	c,a			; Save motor bits
	;
	;	Get the set up values
	;
	dec	hl
	ld	a,(hl)			; 0x34 bits
	ld	d,a			; save bits for later
	;
	;	Set up for the transfer using autowait.
	;
	out	(0x34),a		; output 0x34 bits
	;
	;	Get the drive spinning
	;
	call	fd_setup
	;
	;	When we hit this point the drive is supposed to be selected
	;	and running. The head has had time to settle if needed and
	;	we can try and do an I/O at last.
	;
issue_command:
	ld	hl,(_fd_cmd+3)		; buffer
	ld	c,#0x33			; data port
	ld	e,b			; save command code
	di

	;	If need be flip bank. Remember this also switches stack copy
	ld 	a,(_fd_map)
	or 	a
	jr	z, cmdout

	out	(0x40),a		; switch bank and stack copy
cmdout:
	ld	a,e
	out	(0x30),a		; issue the command


	;	FIXME: mappings
	;
	;	For now we only do double density (256 words per sector)
	;
	ld	b,#0
	;
	;	Check for EOJ (DRQ is handled by autowait)
	;
fd_waitop:
	in	a,(0x34)
	rra
	jr	c,fd_done
	;
	;	ASAP transfer a byte
	;
patch1:	outi
	inc	b			; count in words
	in	a,(0x34)		; check EOJ again
	rra
	jr	c,fd_done
patch2:	outi				; second byte out
	jp	nz,fd_waitop		; faster than jr
	;
	;	The transfer is done - wait for EOJ
	;
fd_waiteoj:
	;
	;	Memory and stack back
	;
	ld	a,#1
	out	(0x40),a		; kernel map back
fd_waiteoj_l:
	ei
	in	a,(0x34)
	rra
	jr	nc, fd_waiteoj_l
	;
	;	Job complete. Turn off autowait and recover
	;	the status byte
	;
fd_done:
	ld	a,#1
	out	(0x40),a		; kernel map back
	ei
	ld	a,d			; 0x34 bits
	and	#0x7F
	out	(0x34),a		; autowait off
	;
	;	Let the controller catch up for 10ms
	;
	ld	hl,#100
	call	delayhl1
	;
	;	Read the status bits
	;
	in	a,(0x30)
	and	#0xFC			; Mask off bad bits
jrreta:	jr	reta			; HL return

;
;	Seek from track to track. The calling C code has set up the
;	control and configuration bits for us including managing the
;	seek bit. This probably doesn't work for some of the slow 8" drives
;
;	HL points at our config...
;
_fd_seek:
	in	a,(0x34)
	ld	c,a
	ld	a,(_fd_cmd + 1)		; control bits
	out	(0x34),a		; control
	ld	d,a
	call	fd_setup		; get the drive spinning
	ld	a,(_fd_cmd + 5)
	out	(0x30),a		; seek (0x10)+delay
seek_wait:
	in	a,(0x34)
	bit	2,a			; motor stopped
	jp	nz, nodisk
	rra
	jr	nc, seek_wait
	ld	a,d
	and	#0x7f
	out	(0x34),a		; auto off
	ld	hl,#100			; 10ms
	call	delayhl1
	in	a,(0x30)		; recover status
	and	#0x98
	jr	jrreta

_fd_map:
	.byte 0
_fd_cmd:
	.byte 0, 0, 0, 0, 0, 0


;
;	0.9ms for 8" drive 1.2ms for 5.25"
;
delayhl:
	ld	hl,#0x09
	ld	a,(_fd_cmd+1)
	bit	2,a			; 8 " ?
	jr	nz, delayhl1
	ld	hl,#0x0C
delayhl1:
	push	bc			; 11
delayhl2:
	dec	hl			; 6

	;
	;	This inner loop costs us 13 cycles per iteration plus 2
	;	(setup costs us 7 more, exit costs us 5 less)
	;
	;	Giving us 366 cycles per loop
	;

	ld	b,#0x1c			; 7
delayhl3:
	djnz	delayhl3		; 13 / 8
	;
	;	Each cycle of the outer loop costs us another 6 to dec hl
	;	and 28 to do the end part of the loop
	;
	;	Giving us a total of 400 cycles per loop, at 4MHz that
	;	means each loop is 0.1ms
	;
	nop				; 4
	nop				; 4
	ld	a,l			; 4
	or	h			; 4
	jr	nz,delayhl2		; 12 / 7
	pop	bc			; 10
	ret				; 10

;
; Don't be tempted to put the symbol in the code below ..it's relocated
; to zero. Instead define where it ends up.
;

_plt_doexec	.equ	0x18

        .area _COMMONMEM

	.globl rst38
	.globl stubs_low
;
;	This exists at the bottom of each bank. We move these into place
;	from discard.
;
stubs_low:
	.byte 0xC3
	.word 0		; cp/m emu changes this
	.byte 0		; cp/m emu I/O byte
	.byte 0		; cp/m emu drive and user
	jp 0		; cp/m emu bdos entry point
rst8:
	ret
	nop
	nop
	nop
	nop
	nop
	nop
	nop
rst10:
	ret
	nop
	nop
	nop
	nop
	nop
	nop
	nop
rst18:
	; Activate the user bank (which also holds these bytes)
	ld a,(_udata + U_DATA__U_PAGE)
	out (0x40),a
	ei
	; and leap into user space
	jp (hl)
	nop
rst20:	ret
	nop
	nop
	nop
	nop
	nop
	nop
	nop
rst28:	ret
	nop
	nop
	nop
	nop
	nop
	nop
	nop
rst30:	jp syscall_high
	nop
	nop
	nop
	nop
	nop
;
;	We only have 38-4F available for this in low space
;
rst38:	jp interrupt_high		; Interrupt handling stub
	nop
	nop
	nop
	nop
	nop
	.ds 0x26
my_nmi_handler:		; Should be at 0x66
	retn

;
;	This stuff needs to live somewhere, anywhere out of the way (so we
;	use common). We need to copy it to the same address on both banks
;	so place it in common as we will put common into both banks
;

	.area _COMMONMEM

	.globl ldir_to_user
	.globl ldir_from_user
	.globl ldir_far
;
;	This needs some properly optimized versions!
;
ldir_to_user:
	ld de,(_udata + U_DATA__U_PAGE)	; will load with 0 e with page
	inc d			; Kernel is in #1
ldir_far:
	push bc
	ld c,#0x40
	exx
	pop bc			; get BC into alt bank
far_ldir_1:
	exx
	out (c),d			; Select source
	ld a,(hl)
	inc hl
	out (c),e			; Select target
	ld (ix),a
	inc ix
	exx
	dec bc
	ld a,b
	or c
	jr nz, far_ldir_1
	ld a,#1
	out (0x40),a		; Select kernel
	ret
ldir_from_user:
	ld a,(_udata + U_DATA__U_PAGE)
	ld e,#1
	ld d,a
	jr ldir_far
;
;	High stubs. Present in each bank in the top 256 bytes
;	of the available space (remembering F000-FFFF is not available
;	for general usage but is ok for reading)
;
interrupt_high:
	push af
	push de
	push hl
	ex af,af'
	push af
	push bc
	exx
	push bc
	push de
	push hl
	push ix
	push iy
	in a,(0x40)		; bank register is thankfully R/W
	ld c,a
	ld a,#0x01
	out (0x40),a		; Kernel map
	ld (istack_switched_sp),sp	; istack is positioned to be valid
	ld sp,#istack_top		; in both banks. We just have to
	;
	;	interrupt_handler may come back on a different stack in
	;	which case bc is junk. Fortuntely we never pre-empt in
	;	kernel so the case we care about bc is always safe. This is
	;	not a good way to write code and should be fixed! FIXME
	;
	push bc
	call interrupt_handler	; switch on the right SP
	pop bc
	; Restore stack pointer to user. This leaves us with an invalid
	; stack pointer if called from user but interrupts are off anyway
	ld sp,(istack_switched_sp)
	; On return HL = signal vector E= signal (if any) A = page for
	; high
	or a
	jr z, kernout
	; Returning to user space
	out (0x40),a		; page passed back
	; User stack is now valid
	; back on user stack
	xor a
	cp e
	call nz, sigpath
pops:
	ex af,af'
	exx
	pop iy
	pop ix
	pop hl
	pop de
	pop bc
	exx
	pop bc
	pop af
	ex af,af'
	pop hl
	pop de
	pop af
	ei
	ret
kernout:
	; restore bank - if we interrupt mid user copy or similar we
	; have to put the right bank back
	ld a,c
	out (0x40),a
	jr pops

sigpath:
	push de		; signal number
	ld de,#irqsigret
	push de		; clean up
	ex de,hl
	ld hl,(PROGLOAD+16)	; helper vector
	jp (hl)
irqsigret:
	inc sp		; drop signal number
	inc sp
	ret

syscall_platform:
syscall_high:
	push ix
	ld ix,#0
	add ix,sp
	push bc
	ld c,6(ix)
	ld b,7(ix)
	ld e,8(ix)
	ld d,9(ix)
	ld l,10(ix)
	ld h,11(ix)
	push hl
	ld l,12(ix)
	ld h,13(ix)
	pop ix
	di
	; AF' can be changed in the ABI
	ex af, af'	; Ick - find a better way to do this bit !
	ld a,#1		; Kernel
	out (0x40),a
	ex af,af'
	; Stack now invalid
	ld (_udata + U_DATA__U_SYSCALL_SP),sp
	ld sp,#kstack_top
	call unix_syscall_entry
	; FIXME check di rules
	; stack now invalid. Grab the new sp before we unbank the
	; memory holding it
	ld sp,(_udata + U_DATA__U_SYSCALL_SP)
	ld a, (_udata + U_DATA__U_PAGE)	; back to the user page
	out (0x40),a
	xor a
	cp h
	call nz, syscall_sigret
	; FIXME for now do the grungy C flag HL DE stuff from
	; lowlevel-z80 until we fix the ABI
	pop bc
	ld a,h
	or l
	jr nz, error
	ex de,hl
	pop ix
	ei
	ret
error:	scf
	pop ix
	ei
	ret
syscall_sigret:
	ld a,l		; DEBUG
	push hl		; save errno
	push de		; save retval
	ld l,h
	ld h,#0
	push hl		; signal
	ld hl,#syscall_sighelp
	push hl		; vector
	push bc
	ret
syscall_sighelp:
	pop de		; discard signal
	pop de		; recover error info
	pop hl
	ld h,#0		; clear signal bit
	ret
