# 0 "z80-mbc2.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "z80-mbc2.S"
;
; Z80-MBC2 support
;
; This first chunk is mostly boilerplate to adjust for each
; system.
;

 ; exported symbols
 .export init_early
 .export init_hardware
 .export interrupt_handler
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
 .export map_for_swap
 .export plt_interrupt_all
 .export _kernel_flag
 .export _int_disabled

 ; exported debugging tools
 .export _plt_monitor
 .export _plt_reboot
 .export outchar

# 1 "kernelu.def" 1
; FUZIX mnemonics for memory addresses etc
;
;
; The U_DATA address. If we are doing a normal build this is the start
; of common memory. We do actually have a symbol for udata so
; eventually this needs to go away
;
U_DATA__TOTALSIZE .equ 0x200 ; 256+256 bytes @ F000
;
; Space for the udata of a switched out process within the bank of
; memory that it uses. Normally placed at the very top
;
U_DATA_STASH .equ 0x7E00 ; 7E00-7FFF
;
; Z80 systems start program space at 0, and load at 0x100 so that the
; low 256 bytes are free for syscall vectors and the like, with some
; also used as a special case by the CP/M emulator.
;
PROGBASE .equ 0x0000
PROGLOAD .equ 0x0100
;
; CPU type
; 0 = CMOS Z80
; 1 = NMOS Z80 (also works with CMOS)
; 2 = Z180
;
; If either NMOS or CMOS may be present pick NMOS as the NMOS build
; contains extra code to work around an erratum n the NUMS Z80
;
Z80_TYPE .equ 0 ; CMOS Z80
;
; For special platforms that have external memory protection hardware
; Just say 0.
;
Z80_MMU_HOOKS .equ 0
;
; Set this if the platform has swap enabled in config.h
;
CONFIG_SWAP .equ 1
;
; The number of disk buffers. Must match config.h
;
NBUFS .equ 5
# 33 "z80-mbc2.S" 2
# 1 "../../cpu-z80u/kernel-z80.def" 1
# 34 "z80-mbc2.S" 2

;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

 .export _bufpool
 .buffers

_bufpool:
 .ds 520 * NBUFS

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (kept even when we task switch)
; -----------------------------------------------------------------------------
 .common
;
; Interrupt flag. This needs to be in common memory for most memory
; models. It starts as 1 as interrupts start off.
;
_int_disabled:
 .byte 1
;
; This method is invoked early in interrupt handling before any
; complex handling is done. It's useful on a few platforms but
; generally a ret is all that is needed
;
plt_interrupt_all:
 ret

;
; If you have a ROM monitor you can get back to then do so, if not
; fall into reboot.
;
_plt_monitor:
;
; Reboot the system if possible, halt if not. On a system where the
; ROM promptly wipes the display you may want to delay or wait for
; a keypress here (just remember you may be interrupts off, no kernel
; mapped so hit the hardware).
;
_plt_reboot:
 di
 jr _plt_reboot

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (may be below 0x8000, only accessible when the kernel is
; mapped)
; -----------------------------------------------------------------------------
 .code

;
; This routine is called continually when the machine has nothing else
; it needs to execute. On a machine with entirely interrupt driven
; hardware this could just halt for interrupt.
;
_plt_idle:
 halt
 ret

;
; This routine is called very early, before the boot code shuffles
; things into place. We do the ttymap here mostly as an example but
; even that really ought to be in init_hardware.
;
init_early:
 ret

; -----------------------------------------------------------------------------
; DISCARD is memory that will be recycled when we exec init
; -----------------------------------------------------------------------------
 .discard

;
; After the kernel has shuffled things into place this code is run.
; It's the best place to breakpoint or trace if you are not sure your
; kernel is loading and putting itself into place properly.
;
; It's required jobs are to set up the vectors, ramsize (total RAM),
; and procmem (total memory free to processs), as well as setting the
; interrupt mode but *not* enabling interrupts. Many platforms also
; program up support hardware like PIO and CTC devices here.
;
init_hardware:
 ld hl, 128
 ld (_ramsize), hl
 ld hl, 64
 ld (_procmem), hl

 ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
 ld hl, 0
 push hl
 call _program_vectors
 pop hl

 im 1 ; set CPU interrupt mode

 ret

;
; Bank switching unsurprisingly must be in common memory space so it's
; always available.
;
 .commondata

mapreg:
 .byte 0 ; Our map register is write only so keep a copy
mapsave:
 .byte 0 ; Saved copy of the previous map (see map_save)

_kernel_flag:
 .byte 1 ; We start in kernel mode

;
; This is invoked with a NULL argument at boot to set the kernel
; vectors and then elsewhere in the kernel when the kernel knows
; a bank may need vectors writing to it.
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
 ld a, 0xC3 ; JP instruction
 ld (0x0038), a
 ld hl, interrupt_handler
 ld (0x0039), hl

 ; set restart vector for FUZIX system calls
 ld (0x0030), a ; (rst 30h is unix function call vector)
 ld hl, unix_syscall_entry
 ld (0x0031), hl

 ld (0x0000), a
 ld hl, null_handler ; to Our Trap Handler
 ld (0x0001), hl

 ; and fall into map_kernel

;
; Mapping set up for the MBC2
;
; The top 32K bank holds kernel code and pieces of common memory
; The lower 32K is switched between the various user banks.
;
; We know the ROM mapping is already off
;
; The _di versions of the functions are called when we know interrupts
; are definitely off. In our case it's not useful information so both
; symbols end up at the same code.
;
; FIXME: need to double check IRQ blocking in the non _di paths
;
map_buffers:
map_kernel_di:
map_kernel:
 di
 push af
do_map_kernel:
 ld a, 0x0D
 out (0x01),a
 xor a
map_and_out:
 out (0x00), a
 ld (mapreg),a
 ld a,(_int_disabled)
 or a
 jr nz, dont_ei
 ei
dont_ei:
 pop af
 ret
 ; map_proc is called with HL either NULL or pointing to the
 ; page mapping. Unlike the other calls it's allowed to trash AF
map_proc:
 ld a, h
 or l
 jr z, map_kernel
map_proc_hl:
 di
 ld a, (hl) ; and fall through
 ;
 ; With a simple bank switching system you need to provide a
 ; method to switch to the bank in A without corrupting any
 ; other registers. The stack is safe in common memory.
 ; For swap you need to provide what for simple banking is an
 ; identical routine.
map_for_swap:
 di
map_proc_a: ; used by bankfork
 ld (mapreg), a ; bank
 push af
 push af
 ld a, 0x0D ; Set bank
 out (0x01),a
 pop af
 jr map_and_out

 ;
 ; Map the current process into memory. We do this by extracting
 ; the bank value from u_page.
 ;
map_proc_always_di:
map_proc_always:
 push af
 push hl
 ld hl, _udata + 2
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
 ld a, (mapreg)
 ld (mapsave), a
 jr do_map_kernel
 ;
 ; Restore the saved bank. Note that you don't need to deal with
 ; stacking of banks (we never recursively use save/restore), and
 ; that we may well call save and decide not to call restore.
 ;
map_restore:
 push af
 di
 ld a, 0x0D
 out (0x01),a
 ld a, (mapsave)
 ld (mapreg), a
 jr map_and_out

 ;
 ; Used for low level debug. Output the character in A without
 ; corrupting other registers. May block. Interrupts and memory
 ; state are undefined
 ;
outchar:
 push af
 di
 ld a, 0x01
 out (0x01),a
 pop af
 out (0x00),a
 ld a,(_int_disabled)
 or a
 ret nz
 ei
 ret

;
; Disk block I/O. Needs to be in common as we directly read/write
; user space when we can
;

vd_map:
 ld bc, OP_RD_PORT
 ld a,(_td_raw)
 or a
 ret z
 dec a
 jp z, map_proc_always
 ld a, (_td_page)
 jp map_for_swap

 .export _vd_read
 .export _vd_write

_vd_read:
 pop de
 pop hl
 push hl
 push de
 push bc
 call vd_map

 ld a, OP_READ_SECTOR
 out (OP_PORT),a
 inir ; transfer first 256 bytes
 inir ; transfer second 256 bytes
 jp map_kernel ; map kernel then return

_vd_write:
 pop de
 pop hl
 push hl
 push de
 push bc
 call vd_map

 ld a, OP_WRITE_SECTOR
 out (OP_PORT),a
 otir ; transfer first 256 bytes
 otir ; transfer second 256 bytes
 jp map_kernel ; map kernel then return
