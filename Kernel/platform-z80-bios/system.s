;
;	    FuzixBIOS System
;
;	This first chunk is mostly boilerplate to adjust for each
;	system.
;

            .module fuzixbios

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl map_buffers
	    .globl map_kernel
	    .globl map_kernel_di
	    .globl map_process
	    .globl map_process_a
	    .globl map_process_always
	    .globl map_process_always_di
	    .globl map_save_kernel
	    .globl map_restore
	    .globl map_for_swap
	    .globl platform_interrupt_all
	    .globl _kernel_flag
	    .globl _int_disabled

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
            .globl outcharhex
	    .globl null_handler
	    .globl _init_hardware_c

	    .globl _fuzixbios_serial_txready
	    .globl _fuzixbios_serial_tx
	    .globl _fuzixbios_reboot
	    .globl _fuzixbios_monitor
	    .globl _fuzixbios_set_bank

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

            .include "kernel.def"
            .include "../kernel-z80.def"

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
platform_interrupt_all:
	    ret
;
;	If you have a ROM monitor you can get back to then do so, if not
;	fall into reboot.
;
_platform_monitor:
	    jp _fuzixbios_monitor
;
;	Reboot the system if possible, halt if not. On a system where the
;	ROM promptly wipes the display you may want to delay or wait for
;	a keypress here (just remember you may be interrupts off, no kernel
;	mapped so hit the hardware).
;
_platform_reboot:
	    jp _fuzixbios_reboot

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
	    ld hl,#128
            ld (_ramsize), hl
	    ld hl,#64
            ld (_procmem), hl

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

            im 1 ; set CPU interrupt mode

            jp _init_hardware_c

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

	    call map_process

            ; set restart vector for FUZIX system calls
            ld (0x0030), a   ;  (rst 30h is unix function call vector)
            ld hl, #unix_syscall_entry
            ld (0x0031), hl

            ld (0x0000), a   
            ld hl, #null_handler   ;   to Our Trap Handler
            ld (0x0001), hl

	    ; and fall into map_kernel

map_buffers:
map_kernel_di:
map_kernel:
	    push af
map_kernel_out:
	    xor a
map_a_out:
	    ld (mapreg),a
	    call _fuzixbios_set_bank
	    pop af
	    ret
	    ; map_process is called with HL either NULL or pointing to the
	    ; page mapping. Unlike the other calls it's allowed to trash AF
map_process:
	    ld a, h
	    or l
	    jr z, map_kernel
map_process_hl:
	    di
	    ld a, (hl)			; and fall through
	    ;
	    ; With a simple bank switching system you need to provide a
	    ; method to switch to the bank in A without corrupting any
	    ; other registers. The stack is safe in common memory.
	    ; For swap you need to provide what for simple banking is an
	    ; identical routine.
map_for_swap:
map_process_a:			; used by bankfork
	    ld (mapreg), a	; bank
	    jp _fuzixbios_set_bank

	    ;
	    ; Map the current process into memory. We do this by extracting
	    ; the bank value from u_page.
	    ;
map_process_always_di:
map_process_always:
	    push af
	    push hl
	    ld hl, #_udata + U_DATA__U_PAGE
	    call map_process_hl
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
	    ld a, (mapreg)
	    ld (mapsave), a
	     jr map_kernel_out
	    ;
	    ; Restore the saved bank. Note that you don't need to deal with
	    ; stacking of banks (we never recursively use save/restore), and
	    ; that we may well call save and decide not to call restore.
	    ;
map_restore:
	    push af
	    ld a, (mapsave)
	    jr map_a_out
	    
	    ;
	    ; Used for low level debug. Output the character in A without
	    ; corrupting other registers. May block. Interrupts and memory
	    ; state are undefined
	    ;
outchar:
	    push bc
	    push de
	    push hl
	    push af
txwait:
	    ld l,#0
	    call _fuzixbios_serial_txready
	    ld a,l
	    or a
	    jr z,txwait
	    pop af
	    call _fuzixbios_serial_tx
	    pop hl
	    pop de
	    pop bc
	    ret
