;
;	    Z280 RC support.
;
;	    Minimal for now to get us up and running
;

            .module sbcv2

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl map_kernel
	    .globl map_process
	    .globl map_process_a
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore
	    .globl map_for_swap
	    .globl platform_interrupt_all
	    .globl _kernel_flag

            ; exported debugging tools
            .globl _platform_monitor
            .globl _platform_reboot
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl istack_top
            .globl istack_switched_sp
            .globl unix_syscall_entry
            .globl trap_illegal
            .globl outcharhex
	    .globl outstring
	    .globl outhl
	    .globl null_handler
	    .globl nmi_handler
	    .globl _inint
	    .globl kstack_top

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

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (kept even when we task switch)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

;
;	This method is invoked early in interrupt handling before any
;	complex handling is done. It's useful on a few platforms but
;	generally a ret is all that is needed
;
platform_interrupt_all:
	    ret

;
;	If you have a ROM monitor you can get back to then do so, if not
;	fall into reboot.
;
_platform_monitor:
;
;	Reboot the system if possible, halt if not. On a system where the
;	ROM promptly wipes the display you may want to delay or wait for
;	a keypress here (just remember you may be interrupts off, no kernel
;	mapped so hit the hardware).
;
_platform_reboot:
	jr _platform_reboot

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (may be below 0x8000, only accessible when the kernel is
; mapped)
; -----------------------------------------------------------------------------
	    .area _CODE

;
;	This routine is called very early, before the boot code shuffles
;	things into place. We assume the bootstrap already set up
;
;	- bus timing and initialization
;	- bus timing and control
;	- cache
;
init_early:
	    ld c,#0x16
	    ld hl,#0x0000
;	    ldctl (c),hl		; set interrupt status
	    .byte 0xED, 0x6E
	    ld c,#0x06
	    ld hl,#_vectors		; Vectors on 4K boundary
	    ld l,h			; Work around crappy linker
	    ld h,#0
;	    ldctl (c),hl		; into int trap/vector ptr
	    .byte 0xED, 0x6E
	    ld c,#0x08
	    ld l,#0
;	    ldctl (c),hl		; clear I/O page
	    .byte 0xED, 0x6E
	    ld c,#0x10
	    ld l,#0x04			; no user I/O no EPU
;	    ldctl (c),hl		; set trap control (can't set
					; stack trap until have user mode
					; done)
	    .byte 0xED, 0x6E
	    ret

; -----------------------------------------------------------------------------
; DISCARD is memory that will be recycled when we exec init
; -----------------------------------------------------------------------------
	    .area _DISCARD
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
	    call map_kernel
	    ld l,#0xFF
	    call _io_bank_set
	    push hl
	    ld hl,#0xBBE0	; MMU on S and U, no split I/D
				; We may want to consider MMU off in
				; supervisor later on - is there a perf
				; gain versus MMU on ??
	    ld c,#0xF0
;	    outw (c),hl
	    .byte 0xED,0xBF
	    pop hl
	    call _io_bank_set	    

	    ld hl,#2048
            ld (_ramsize), hl
	    ld hl,#1984
            ld (_procmem), hl

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

	    ; Our CTC is fed from the system clock / 4 which gives us
	    ; 7.372800Mhz input. We want a semsible multiple of 10Hz and
	    ; for a fast CPU 50 or 100 isn't a bad value. On a single CTC
	    ; counter our best shot is 120Mhz (61440 divider exactly)

	    ld l,#0xFE
	    call _io_bank_set
	    push hl
	    ld a,#0xA0		; Timer, interrupting
	    out (0xE0),a
	    ld hl,#61440	; 120Hz
	    ld c,#0xE2
;	    outw (c),hl
	    .byte 0xED,0xBF
	    ld a,#0xC0		; Enable, gate
	    out (0xE1),a
	    pop hl	
	    call _io_bank_set    


;            im 3 ; set CPU interrupt mode
	    .byte 0xED, 0x4E

            ret

;
;	Bank switching unsurprisingly must be in common memory space so it's
;	always available. This is a simple hack to get us going. The Z280
;	has a real MMU and virtual memory. We just treat it like a bunch of
;	banks.
;
            .area _COMMONMEM

mapreg:    .db 0	; Our map register is write only so keep a copy
mapsave:   .db 0	; Saved copy of the previous map (see map_save)

_kernel_flag:
	    .db 1	; We start in kernel mode

;
;	This is invoked with a NULL argument at boot to set the kernel
;	vectors and then elsewhere in the kernel when the kernel knows
;	a bank may need vectors writing to it.
;
;	Will need rewriting when we do things properly
;
_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
            di ; just to be sure
            pop de ; temporarily store return address
            pop hl ; function argument -- base page number
            push hl ; put stack back as it was
            push de

	    call map_process

            ; set restart vector for FUZIX system calls
	    ; once we do real Z280 mode we'll need this to syscall
            ld (0x0030), a   ;  (rst 30h is unix function call vector)
            ld hl, #unix_syscall_entry
            ld (0x0031), hl

            ld (0x0000), a   
            ld hl, #null_handler   ;   to our trap handler
            ld (0x0001), hl
	    ; Fall into map_kernel

;
;	Mapping set up for the Z280. This is hack to get us going
;
;	We map the low 64K 1:1 in kernel mode, keep the top 8K fixed and
;	map the others by 'bank' for user mode. We don't actually use the
;	hardware user mode yet. It's all just a bodge to get us up and
;	running
;
map_kernel:
	    push af
	    xor a
	    call map_process_a	; do all the logic in one place with
	    pop af		; kernel as entry 0 in the table
	    ret
	    ; map_process is called with HL either NULL or pointing to the
	    ; page mapping. Unlike the other calls it's allowed to trash AF
map_process:
	    ld a, h
	    or l
	    jr z, map_kernel
map_process_hl:
	    ld a, (hl)			; and fall through
	    ;
	    ; With a simple bank switching system you need to provide a
	    ; method to switch to the bank in A without corrupting any
	    ; other registers. The stack is safe in common memory.
	    ; For swap you need to provide what for simple banking is an
	    ; identical routine.
map_for_swap:
map_process_a:			; used by bankfork
	    push bc
	    push de
	    push hl
	    ld (mapreg), a	; bank
	    ld c,#0x08
;;	    ldctl hl,(c)
	    .byte 0xED, 0x66
	    push hl
	    ld l,#0xff		; MMU is I/O bank FF
;;	    ldctl (c),hl
	    .byte 0xED, 0x6E
	    ld l,a
	    ld h,#0
	    add hl,hl		; 16 words per bank
	    add hl,hl
	    add hl,hl
	    add hl,hl
	    add hl,hl
	    ld de,#frames	; Lazy - look it up it's only a hack for now
	    ld a,#0x10
	    out (0x44),a	; pointer to 0x10 (system pages)
	    ld bc,#0x0FF4
;;	    otirw
            .db 0xED, 0x93
	    pop hl
	    ld c,#0x08
;;	    ldctl (c),hl	; previous bank register
	    .byte 0xED, 0x6E
	    pop hl
	    pop de
	    pop bc
            ret

	    ;
	    ; Map the current process into memory. We do this by extracting
	    ; the bank value from u_page.
	    ;
map_process_always:
	    push af
	    push hl
	    ld hl, #U_DATA__U_PAGE
	    call map_process_hl
	    pop hl
	    pop af
	    ret

	    ;
	    ; Save the existing mapping. The place you save it to needs to
	    ; be in common memory as you have no idea what bank is live
            ;
map_save:   push af
	    ld a, (mapreg)
	    ld (mapsave), a
	    pop af
	    ret
	    ;
	    ; Restore the saved bank. Note that you don't need to deal with
	    ; stacking of banks (we never recursively use save/restore), and
	    ; that we may well call save and decide not to call restore.
	    ;
map_restore:
	    push af
	    ld a, (mapsave)
	    call map_process_a
	    pop af
	    ret
	    ;
	    ; Used for low level debug. Output the character in A without
	    ; corrupting other registers. May block. Interrupts and memory
	    ; state are undefined
	    ;
outchar:
	    push af
	    push bc
	    push de
	    push hl
	    ld l,#0xFE
	    call _io_bank_set
twait:	    in a,(0x12)
	    bit 0,a
	    jr z, twait
	    pop af
	    out (0x18),a
	    call _io_bank_set
	    pop hl
	    pop de
	    pop bc
	    pop af
            ret

;
;	These belong in core Z280 code eventually
;
	    .globl _io_bank_set
	    .globl _flush_cpu_cache


_io_bank_set:
	    ld c,#8
	    ex de,hl		; save the new value in DE
;;	    ldctl hl,(c)	; read old into HL
	    .byte 0xED, 0x66
	    ex de,hl		; switch bank
;;	    ldctl (c),hl	; new into HL
	    .byte 0xED, 0x6E
	    ex de,hl		; and back again to return old
	    ret

_flush_cpu_cache:
	    .byte 0xED, 0x65
	    ret
	    

;
;	Mapping tables for speed
;
frames:
	.word 0x0000		; Identity map kernel
	.word 0x0010
	.word 0x0020
	.word 0x0030
	.word 0x0040
	.word 0x0050
	.word 0x0060
	.word 0x0070
	.word 0x0080
	.word 0x0090
	.word 0x00A0
	.word 0x00B0
	.word 0x00C0
	.word 0x00D0
	.word 0x00E0		; Common
	.word 0x00F0		;

	.word 0x0100
	.word 0x0110
	.word 0x0120
	.word 0x0130
	.word 0x0140
	.word 0x0150
	.word 0x0160
	.word 0x0170
	.word 0x0180
	.word 0x0190
	.word 0x01A0
	.word 0x01B0
	.word 0x01C0
	.word 0x01D0
	.word 0x00E0		; Common
	.word 0x00F0		;

	.word 0x0200
	.word 0x0210
	.word 0x0220
	.word 0x0230
	.word 0x0240
	.word 0x0250
	.word 0x0260
	.word 0x0270
	.word 0x0280
	.word 0x0290
	.word 0x02A0
	.word 0x02B0
	.word 0x02C0
	.word 0x02D0
	.word 0x00E0		; Common
	.word 0x00F0		;

	.word 0x0300
	.word 0x0310
	.word 0x0320
	.word 0x0330
	.word 0x0340
	.word 0x0350
	.word 0x0360
	.word 0x0370
	.word 0x0380
	.word 0x0390
	.word 0x03A0
	.word 0x03B0
	.word 0x03C0
	.word 0x03D0
	.word 0x00E0		; Common
	.word 0x00F0		;

	.word 0x0400
	.word 0x0410
	.word 0x0420
	.word 0x0430
	.word 0x0440
	.word 0x0450
	.word 0x0460
	.word 0x0470
	.word 0x0480
	.word 0x0490
	.word 0x04A0
	.word 0x04B0
	.word 0x04C0
	.word 0x04D0
	.word 0x00E0		; Common
	.word 0x00F0		;

	.word 0x0500
	.word 0x0510
	.word 0x0520
	.word 0x0530
	.word 0x0540
	.word 0x0550
	.word 0x0560
	.word 0x0570
	.word 0x0580
	.word 0x0590
	.word 0x05A0
	.word 0x05B0
	.word 0x05C0
	.word 0x05D0
	.word 0x00E0		; Common
	.word 0x00F0		;

	.word 0x0600
	.word 0x0610
	.word 0x0620
	.word 0x0630
	.word 0x0640
	.word 0x0650
	.word 0x0660
	.word 0x0670
	.word 0x0680
	.word 0x0690
	.word 0x06A0
	.word 0x06B0
	.word 0x06C0
	.word 0x06D0
	.word 0x00E0		; Common
	.word 0x00F0		;

	.word 0x0700
	.word 0x0710
	.word 0x0720
	.word 0x0730
	.word 0x0740
	.word 0x0750
	.word 0x0760
	.word 0x0770
	.word 0x0780
	.word 0x0790
	.word 0x07A0
	.word 0x07B0
	.word 0x07C0
	.word 0x07D0
	.word 0x00E0		; Common
	.word 0x00F0		;

	.word 0x0700
	.word 0x0710
	.word 0x0720
	.word 0x0730
	.word 0x0740
	.word 0x0750
	.word 0x0760
	.word 0x0770
	.word 0x0780
	.word 0x0790
	.word 0x07A0
	.word 0x07B0
	.word 0x07C0
	.word 0x07D0
	.word 0x00E0		; Common
	.word 0x00F0		;


;
;	Interrupt vectors. Kernel and identity mapped
;
	.area _VECTORS

SUPER	.equ	0x0000		; Supervisor, all ints masked

	.globl _vectors

_vectors:
	.word	#SUPER		; 00	Reserved
	.word	invalid
	.word	#SUPER		; 04	NMI
	.word	nmi_handler
	.word	#SUPER		; 08	External Line A
	.word	external
	.word	#SUPER		; 0C	External Line B
	.word	external
	.word	#SUPER		; 10	External Line C
	.word	external
	.word	#SUPER		; 14	CTC 0
	.word	ctc0
	.word	#SUPER		; 18	CTC 1
	.word	ctc1
	.word	#SUPER		; 1C	Reserved
	.word	invalid		; The broken undocumented CTC
	.word	#SUPER		; 20	CTC 2
	.word	invalid
	.word	#SUPER		; 24	DMA 0
	.word	invalid
	.word	#SUPER		; 28	DMA 1
	.word	invalid
	.word	#SUPER		; 2C	DMA 2
	.word	invalid
	.word	#SUPER		; 30	DMA 3
 	.word	invalid
	.word	#SUPER		; 34	UART rx
	.word	ttyint
	.word	#SUPER		; 38	UART tx
	.word	invalid
	.word	#SUPER		; 3C	Single-step
	.word	sstep
	.word	#SUPER		; 40	Breakpoint
	.word	sigtrap
	.word	#SUPER		; 44	Divison by zero
	.word	sigfpe
	.word	#SUPER		; 48	Stack overflow
	.word	stackover
	.word	#SUPER		; 4C	Access violation
	.word	sigsegv
	.word	#SUPER		; 50	System call
	.word	invalid
	.word	#SUPER		; 54	Privileged instruction
	.word	sigill
	.word	#SUPER		; 58	EPU
	.word	sigill
	.word	#SUPER		; 5C	EPU
	.word	sigill
	.word	#SUPER		; 60	EPU
	.word	sigill
	.word	#SUPER		; 64	EPU
	.word	invalid
	.word	#SUPER		; 68	Reserved
	.word	invalid
	.word	#SUPER		; 6C	Reserved
	.word	invalid

	; What can follow then is 384 word size vectors for INT A B C
	; for IM2 style external I/O which we don't use

	.area _COMMONMEM

SIGILL	.equ	4
SIGTRAP	.equ	5
SIGFPE	.equ	8
SIGSEGV	.equ	11

	.globl _irq_source

_irq_source:
	.byte 0

invalid:
	ld hl,#invalidint
	call outstring
	jr trapexit

external:
	; Interrupt off the RC2014 bus
	push af
	xor a
irqcall:
	ld (_irq_source),a
	; This bit belongs in  a future Z280 lowlevel not here
	push de
	push hl
	xor a
	call _io_bank_set
	push hl
	call interrupt_handler
	pop hl
	call _io_bank_set
	pop hl
	pop de
	pop af
	jr trapexit
ctc0:
	push af
	ld a,#1
	jr irqcall
ctc1:
	push af
	ld a,#2
	jr irqcall

ttyint:
	push af
	ld a,#3
	jr irqcall

sstep:
sigtrap:
	push af
	ld a,#SIGTRAP
	jr sig_or_die
sigfpe:
	push af
	ld a,#SIGFPE
	jr sig_or_die
sigsegv:
	push af
	ld a,#SIGSEGV
	jr sig_or_die
sigill:
	push af
	ld a,#SIGILL
sig_or_die:
	; Once we have proper supervisor/user we can just check the pushed
	; status to see what to do. For now fudge it roughly.
	push af
	ld a,(_inint)
	or a
	jr nz, diediedie
	ld a,(U_DATA__U_INSYS)
	or a
	jr nz, diediedie
	; TODO - we need to go via the syscall signal path as we need this
	;  to be synchronous. Will need to be in the lowlevel-z280 code when
	;   we get there
	jr trapexit

stackover:
	ld sp,#kstack_top
	ld hl,#stackfault
	call outstring
	jp _platform_monitor

trapexit:
	; Discard the data
	inc sp
	inc sp
	; retil
	.byte 0xed, 0x55

invalidint:
	.asciz 'INVIRQ'
stackfault:
	.asciz 'STKFLT'

diediedie:
	call outcharhex
	pop af		; discard
	pop hl		; cause
	call outhlcolon
	pop hl		; status
	call outhlcolon
	pop hl		; pc
	call outhlcolon
	jp _platform_monitor

outhlcolon:
	call outcharhex
	ld a,#':'
	jp outhl
