;
;	    SBC2G support
;
;	This first chunk is mostly boilerplate to adjust for each
;	system.
;

            .module sbcv2

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl map_buffers
	    .globl map_kernel
	    .globl map_kernel_di
	    .globl map_kernel_restore
	    .globl map_proc
	    .globl map_proc_a
	    .globl map_proc_always
	    .globl map_proc_always_di
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
	    .globl null_handler
	    .globl ___sdcc_enter_ix

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

            .include "kernel.def"
            .include "../../cpu-z80/kernel-z80.def"

; Base address of SIO/2 chip 0x00

SIOA_D		.EQU	0x00
SIOB_D		.EQU	0x01
SIOA_C		.EQU	0x02
SIOB_C		.EQU	0x03

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
;	Interrupt flag. This needs to be in common memory for most memory
;	models. It starts as 1 as interrupts start off.
;
_int_disabled:
	    .db 1
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
;	ROM is long gone. Could look at reloading boot sector ?
;
_plt_reboot:
	    di
	    jr _plt_reboot

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (may be below 0x8000, only accessible when the kernel is
; mapped)
; -----------------------------------------------------------------------------
	    .area _CODE

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
	    ld hl,#sio_setup
	    ld bc,#0x0A00 + SIOA_C		; 10 bytes to SIOA_C
	    otir

	    ld hl,#sio_setup
	    ld bc,#0x0A00 + SIOB_C		; and to SIOB_C
	    otir

	    ld hl,#512
            ld (_ramsize), hl
	    ld hl,#448
            ld (_procmem), hl

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

	    ; Compiler helper vectors - in kernel bank only

	    ld	hl,#rstblock
	    ld	de,#8
	    ld	bc,#32
	    ldir

            im 1 ; set CPU interrupt mode

            ret

RTS_LOW	.EQU	0xEA

sio_setup:
	    .byte 0x00
	    .byte 0x18		; Reset
	    .byte 0x04
	    .byte 0xC4
	    .byte 0x01
	    .byte 0x19		; We want carrier events
	    .byte 0x03
	    .byte 0xE1
	    .byte 0x05
	    .byte RTS_LOW


;
;	Bank switching unsurprisingly must be in common memory space so it's
;	always available.
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
            ld hl, #interrupt_handler
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
;	Mapping set up for the SBCv2
;
;	The top 32K bank holds kernel code and pieces of common memory
;	The lower 32K is switched between the various user banks.
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
map_kernel_restore:
map_kernel_di:
map_kernel:
	    push af
	    xor a
	    ld (mapreg),a
	    out (0x30), a
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
map_for_swap:
map_proc_a:			; used by bankfork
	    ld (mapreg), a	; bank
	    out (0x30), a
	    inc a		; cheaper than push/pop
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
map_save_kernel:   push af
	    ld a, (mapreg)
	    ld (mapsave), a
	    xor a
	    ld (mapreg),a
	    out (0x30),a
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
	    ld (mapreg), a
	    out (0x30), a
	    pop af
	    ret
	    
	    ;
	    ; Used for low level debug. Output the character in A without
	    ; corrupting other registers. May block. Interrupts and memory
	    ; state are undefined
	    ;
outchar:
	    push af
ocloop_sio:
            xor a                   ; read register 0
            out (SIOA_C), a
	    in a,(SIOA_C)		; read Line Status Register
	    and #0x04			; get THRE bit
	    jr z,ocloop_sio
	    ; now output the char to serial port
	    pop af
	    out (SIOA_D),a
	    ret

	    .area _CODE
;
;	A little SIO helper
;
	.globl _sio_r
	.globl _sio2_otir

_sio2_otir:
	ld b,#0x06
	ld c,l
	ld hl,#_sio_r
	otir
	ret

;
;	Stub helpers for code compactness. Note that
;	sdcc_enter_ix is in the standard compiler support already
;
	.area _DISCARD

;
;	The first two use an rst as a jump. In the reload sp case we don't
;	have to care. In the pop ix case for the function end we need to
;	drop the spare frame first, but we know that af contents don't
;	matter
;

rstblock:
	jp	___sdcc_enter_ix
	.ds	5
___spixret:
	ld	sp,ix
	pop	ix
	ret
	.ds	3
___ixret:
	pop	af
	pop	ix
	ret
	.ds	4
___ldhlhl:
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	ret
