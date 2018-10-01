;
;	Cromemco hardware support
;

            .module cromemco

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl _program_vectors
	    .globl platform_interrupt_all

	    .globl map_kernel
	    .globl map_process
	    .globl map_process_always
	    .globl map_kernel_di
	    .globl map_process_di
	    .globl map_process_always_di
	    .globl map_process_a
	    .globl map_save_kernel
	    .globl map_restore

	    .globl _platform_reboot

	    .globl _int_disabled

            ; exported debugging tools
            .globl _platform_monitor
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem

	    .globl unix_syscall_entry
            .globl null_handler
	    .globl nmi_handler
            .globl interrupt_handler
	    .globl _doexit
	    .globl _inint
	    .globl kstack_top
	    .globl _panic
	    .globl _need_resched
	    .globl _ssig

            .globl outcharhex
            .globl outhl, outde, outbc
            .globl outnewline
            .globl outstring
            .globl outstringhex

            .include "kernel.def"
            .include "../kernel.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_platform_reboot:
_platform_monitor:
	    jr _platform_monitor
platform_interrupt_all:
	    ret

_int_disabled:
	    .db 1

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xF000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
	    ld a,#0x81			; Every memory is in bank 7
	    out (0x40),a		; MMU set
	    ld hl,#0xF000		; Copy 4K to itself loading
	    ld d,h			; into into the other banks
	    ld e,l
	    ld b,#0x10
	    ld c,l
	    ldir
	    ld a,#0x01			; bank to the kernel bank
	    out (0x40),a
            ret

init_hardware:
            ; set system RAM size
            ld hl, #448
            ld (_ramsize), hl
            ld hl, #(448-64)		; 64K for kernel
            ld (_procmem), hl

	    ld a, #156			; ticks for 10Hz (9984uS per tick)
	    out (8), a			; 10Hz timer on

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

;	    ld a, #0xfe			; Use FEFF (currently free)
;	    ld i, a
;            im 2 ; set CPU interrupt mode
	    im 1			; really should use a page and im2?
            ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM


_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
            di ; just to be sure
            pop de ; temporarily store return address
            pop hl ; function argument -- base page number
            push hl ; put stack back as it was
            push de

	    call map_process

            ; write zeroes across all vectors
            ld hl, #0
            ld de, #1
            ld bc, #0x007f ; program first 0x80 bytes only
            ld (hl), #0x00
            ldir

            ; now install the interrupt vector at 0x38
            ld hl, #interrupt_handler
            ld (0x39), hl

	    ld a,#0xC3		; JP
            ; set restart vector for UZI system calls
            ld (0x0030), a   ;  (rst 30h is unix function call vector)
	    ld (0x0038), a   ;  (rst 38h)
            ld hl, #unix_syscall_entry
            ld (0x0031), hl

            ; now install the interrupt vector at 0x38
            ld hl, #interrupt_handler
            ld (0x39), hl

            ; Set vector for jump to NULL
            ld (0x0000), a   
            ld hl, #null_handler  ;   to Our Trap Handler
            ld (0x0001), hl

            ld (0x0066), a  ; Set vector for NMI
            ld hl, #nmi_handler
            ld (0x0067), hl

	    ; falls through

            ; put the paging back as it was -- we're in kernel mode so this is predictable
map_kernel:
map_kernel_di:
	    push af
	    ld a,#1
	    out (0x40), a
	    ld (map_page), a	; map_page lives in kernel so be careful
	    pop af		; our common is r/o common so writes won't
            ret			; cross a bank
map_process:
map_process_di:
	    ld a, h
	    or l
	    jr z, map_kernel
	    ld a, (hl)
map_process_a:
	    ld (map_page),a	; save before we map out kernel
	    out (0x40), a
            ret
map_process_always:
map_process_always_di:
	    push af
	    ld a, (U_DATA__U_PAGE)
	    ld (map_page),a	; save before we map out kernel
	    out (0x40), a
	    pop af
	    ret
map_save_kernel:
	    push af		; map_save will always do a map_kernel
	    ld a, #1		; map_kernel so we do the map_kernel
	    out (0x40), a	; first so we can get the variables
	    ld a, (map_page)	;
	    ld (map_store), a
	    pop af
	    ret	    
map_restore:			; called in kernel map
	    push af
	    ld a, (map_store)
	    ld (map_page),a
	    out (0x40), a
	    pop af
	    ret	    
map_store:
	    .db 0
map_page:
	    .db 0

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
	;	We run in common. Map the kernel or user according to the
	;	destination of the transfer
	;
	ld	a, (_fd_map)
	or	a
	call	nz, map_process_always
	;
	;	Set up the registers in order. The aux register is already
	;	done by our caller
	;
	ld	hl,#_fd_cmd
	ld	b,(hl)			; command
	inc	hl
	bit	0,(hl)			; read or write ?
	; patch patch1/2 according to the transfer direction
	ld	a,#0xA2			; ini
	jr	z, patchit
	ld	a,#0xA3			; outi
patchit:
	ld	(patch1+1),a
	ld	(patch2+1),a
	;
	;	We have to disable interrupts before we write the
	;	command register when doing a data transfer otherwise an
	;	interrupt can lose us bytes and break the transfer
	;
	di
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
	ld	a,b
	out	(0x30),a		; issue the command

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
	ei
	in	a,(0x34)
	rra
	jr	nc, fd_waiteoj
	;
	;	Job complete. Turn off autowait and recover
	;	the status byte
	;
fd_done:
	ei
	call	map_kernel
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

