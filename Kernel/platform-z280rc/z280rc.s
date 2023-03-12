;
;	    Z280 RC support.
;
;	    Minimal for now to get us up and running
;

            .module z280rc

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl map_buffers
	    .globl map_kernel
	    .globl map_kernel_di
	    .globl map_kernel_restore
	    .globl map_process
	    .globl map_process_di
	    .globl map_process_a
	    .globl map_process_always
	    .globl map_process_always_di
	    .globl map_save_kernel
	    .globl map_restore
	    .globl map_for_swap
	    .globl plt_interrupt_all
	    .globl _kernel_flag
	    .globl _int_disabled

            ; exported debugging tools
            .globl _plt_monitor
            .globl _plt_reboot
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl istack_top
            .globl istack_switched_sp
            .globl unix_syscall_entry
            .globl outcharhex
	    .globl outstring
	    .globl outhl
	    .globl null_handler
	    .globl nmi_handler
	    .globl kstack_top

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

            .include "kernel.def"
            .include "../kernel-z80.def"


	.area _COMMONMEM
;
;	These must be 4K aligned so put them at the start of common
;	space for now. This will all change when we start running
;	with the MMU being used properly.
;
;	Interrupt vectors. Kernel and identity mapped
;
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
plt_interrupt_all:
	    ret

;
;	If you have a ROM monitor you can get back to then do so, if not
;	fall into reboot.
;
_plt_monitor:
;
;	Reboot the system if possible, halt if not. On a system where the
;	ROM promptly wipes the display you may want to delay or wait for
;	a keypress here (just remember you may be interrupts off, no kernel
;	mapped so hit the hardware).
;
_plt_reboot:
	jr _plt_reboot

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
	    ld h,#0			; top 16 bits of 24bit vector
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
	    ld hl,#0xBBFF	; TODO: revise as we get going
				; MMU on S and U, no split I/D
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

_int_disabled:
	    .db 1	; With interrupts off
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
;	running.
;
;	Some day we probably want the buffers outside of the main map, at
;	least until we do split I/D somehow.
;
map_buffers:
map_kernel:
map_kernel_di:
map_kernel_restore:
	    push af
	    xor a
	    call map_process_a	; do all the logic in one place with
	    pop af		; kernel as entry 0 in the table
	    ret
	    ; map_process is called with HL either NULL or pointing to the
	    ; page mapping. Unlike the other calls it's allowed to trash AF
map_process:
map_process_di:
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
	    add hl,de
	    ld a,#0x10
	    out (0xF1),a	; PDR pointer to 0x10 (system pages)
	    ld bc,#0x10F4	; 16 words to F4
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
map_process_always_di:
	    push af
	    push hl
	    ld hl, #_udata + U_DATA__U_PAGE
	    call map_process_hl
	    pop hl
	    pop af
	    ret

	    ;
	    ; Save the existing mapping. The place you save it to needs to
	    ; be in common memory as you have no idea what bank is live
            ;
map_save_kernel:
	    push af
	    ld a, (mapreg)
	    ld (mapsave), a
	    xor a
	    call map_process_a	; kernel is map 0
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
	.word 0x000A		; Identity map kernel
	.word 0x001A
	.word 0x002A
	.word 0x003A
	.word 0x004A
	.word 0x005A
	.word 0x006A
	.word 0x007A
	.word 0x008A
	.word 0x009A
	.word 0x00AA
	.word 0x00BA
	.word 0x00CA
	.word 0x00DA
	.word 0x00EA		; Common
	.word 0x00FA		;

	.word 0x010A
	.word 0x011A
	.word 0x012A
	.word 0x013A
	.word 0x014A
	.word 0x015A
	.word 0x016A
	.word 0x017A
	.word 0x018A
	.word 0x019A
	.word 0x01AA
	.word 0x01BA
	.word 0x01CA
	.word 0x01DA
	.word 0x00EA		; Common
	.word 0x00FA		;

	.word 0x020A
	.word 0x021A
	.word 0x022A
	.word 0x023A
	.word 0x024A
	.word 0x025A
	.word 0x026A
	.word 0x027A
	.word 0x028A
	.word 0x029A
	.word 0x02AA
	.word 0x02BA
	.word 0x02CA
	.word 0x02DA
	.word 0x00EA		; Common
	.word 0x00FA		;

	.word 0x030A
	.word 0x031A
	.word 0x032A
	.word 0x033A
	.word 0x034A
	.word 0x035A
	.word 0x036A
	.word 0x037A
	.word 0x038A
	.word 0x039A
	.word 0x03AA
	.word 0x03BA
	.word 0x03CA
	.word 0x03DA
	.word 0x00EA		; Common
	.word 0x00FA		;

	.word 0x040A
	.word 0x041A
	.word 0x042A
	.word 0x043A
	.word 0x044A
	.word 0x045A
	.word 0x046A
	.word 0x047A
	.word 0x048A
	.word 0x049A
	.word 0x04AA
	.word 0x04BA
	.word 0x04CA
	.word 0x04DA
	.word 0x00EA		; Common
	.word 0x00FA		;

	.word 0x050A
	.word 0x051A
	.word 0x052A
	.word 0x053A
	.word 0x054A
	.word 0x055A
	.word 0x056A
	.word 0x057A
	.word 0x058A
	.word 0x059A
	.word 0x05AA
	.word 0x05BA
	.word 0x05CA
	.word 0x05DA
	.word 0x00EA		; Common
	.word 0x00FA		;

	.word 0x060A
	.word 0x061A
	.word 0x062A
	.word 0x063A
	.word 0x064A
	.word 0x065A
	.word 0x066A
	.word 0x067A
	.word 0x068A
	.word 0x069A
	.word 0x06AA
	.word 0x06BA
	.word 0x06CA
	.word 0x06DA
	.word 0x00EA		; Common
	.word 0x00FA		;

	.word 0x070A
	.word 0x071A
	.word 0x072A
	.word 0x073A
	.word 0x074A
	.word 0x075A
	.word 0x076A
	.word 0x077A
	.word 0x078A
	.word 0x079A
	.word 0x07AA
	.word 0x07BA
	.word 0x07CA
	.word 0x07DA
	.word 0x00EA		; Common
	.word 0x00FA		;

	.word 0x070A
	.word 0x071A
	.word 0x072A
	.word 0x073A
	.word 0x074A
	.word 0x075A
	.word 0x076A
	.word 0x077A
	.word 0x078A
	.word 0x079A
	.word 0x07AA
	.word 0x07BA
	.word 0x07CA
	.word 0x07DA
	.word 0x00EA		; Common
	.word 0x00FA		;


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
	ld a,(_udata + U_DATA__U_ININTERRUPT)
	or a
	jr nz, diediedie
	ld a,(_udata + U_DATA__U_INSYS)
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
	jp _plt_monitor

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
	jp _plt_monitor

outhlcolon:
	call outcharhex
	ld a,#':'
	jp outhl
