# 0 "sbcv2.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "sbcv2.S"
;
; SBC v2 support
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

# 1 "kernel.def" 1
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
Z80_TYPE .equ 1 ; NMOS (IRQ bugs) Z80
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
;
; Configuration for the Zeta floppy controller. Specific to that
; driver rather than generic.
;
; FDC9266 floppy controller ports
FDC_CCR .equ 0 ; No CCR
FDC_MSR .equ 0x36 ; 8272 Main Status Register (R/O)
FDC_DATA .equ 0x37 ; 8272 Data Port (R/W)
FDC_DOR .equ 0x38 ; Digital Output Register (W/O)

CPU_CLOCK_KHZ .equ 8000 ; 8MHz is usual top

;
; Values for the PPI port
;
ppi_port_a .equ 0x60
ppi_port_b .equ 0x61
ppi_port_c .equ 0x62
ppi_control .equ 0x63

PPIDE_CS0_LINE .equ 0x08
PPIDE_CS1_LINE .equ 0x10
PPIDE_WR_LINE .equ 0x20
PPIDE_RD_LINE .equ 0x40
PPIDE_RST_LINE .equ 0x80

PPIDE_PPI_BUS_READ .equ 0x92
PPIDE_PPI_BUS_WRITE .equ 0x80

ppide_data .equ PPIDE_CS0_LINE
# 33 "sbcv2.S" 2
# 1 "../../cpu-z80u/kernel-z80.def" 1
# 34 "sbcv2.S" 2

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
 xor a
 out (0x78),a ; On the RBCv2 this vanishes the RAM low
 out (0x7C),a ; and on both this puts back the ROM low
 rst 0

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (may be below 0x8000, only accessible when the kernel is
; mapped)
; -----------------------------------------------------------------------------
 .code

 .export _ttymap
;
; This routine is called very early, before the boot code shuffles
; things into place. We do the ttymap here mostly as an example but
; even that really ought to be in init_hardware.
;
init_early:
 ; FIXME: code goes here to check for PropIO v2 nicely
 ; Passes the minimal checking
 ld a,0xA5
 out (0xAB),a ; Write A5 data
 ld a,0x55
 out (0xAB),a ; Overwrite
 ld a,0x00
 out (0xAA),a ; Issue a NOP
 in a,(0xAB)
 cp #0xA5 ; Should see byte 1 again
 ret nz
 ld hl,0x0102
 ld (_ttymap+1), hl ; set tty map to 0,2,1 for prop
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
 ;
 ; We usually have 512K but the board can be built with 128K
 ;
 ld hl,0x00FF ; we don't have anything here
 ld a,0x81 ; map page 1
 out (0x78),a
 ld (hl),a
 ld a,0x85 ; map page 5 (or page 1 again on 128K)
 out (0x78),a
 dec (hl)
 ld a,0x81
 out (0x78),a
 cp (hl)
 ld hl,512
 jr z, ramsize512
 ld hl,128
ramsize512:
 ld (_ramsize), hl
 ld de,64 ; and 64K for kernel (so 128K gets you only
    ; two user banks which is OK single user)
 or a
 sbc hl,de
 ld (_procmem), hl

 ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
 ld hl, 0
 push hl
 call _program_vectors
 pop hl

 ; Compiler helper vectors - in kernel bank only

 ld hl,rstblock
 ld de,8
 ld bc,32
 ldir

 im 1 ; set CPU interrupt mode

 ret

;
; Bank switching unsurprisingly must be in common memory space so it's
; always available.
;
 .common

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
; Mapping set up for the SBCv2
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
map_buffers:
 ; for us no difference. We could potentially use a low 32K bank
 ; for buffers but it's not clear it would gain us much value
map_kernel_di:
map_kernel:
 push af
 ; ROMWBW TPA is last but one bank (last bank is high space)
 ; so for now we hardcode this. We should ask ROMWBW at boot
 ld a,0x8E
 ld (mapreg),a
 out (0x78), a
 pop af
 ret
 ; map_proc is called with HL either NULL or pointing to the
 ; page mapping. Unlike the other calls it's allowed to trash AF
map_proc:
 ld a, h
 or l
 jr z, map_kernel
map_proc_hl:
 ld a, (hl) ; and fall through
 ;
 ; With a simple bank switching system you need to provide a
 ; method to switch to the bank in A without corrupting any
 ; other registers. The stack is safe in common memory.
 ; For swap you need to provide what for simple banking is an
 ; identical routine.
map_for_swap:
map_proc_a: ; used by bankfork
 dec a ; We bias by 1 because 0 is a valid user
 ld (mapreg), a ; bank
 out (0x78), a
 inc a ; cheaper than push/pop
 ret

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
 ld a,14 ; Kernel map (see map_kernel)
 ld (mapreg),a
 out (0x78),a
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
 out (0x78), a
 pop af
 ret

 ;
 ; Used for low level debug. Output the character in A without
 ; corrupting other registers. May block. Interrupts and memory
 ; state are undefined
 ;
outchar:
 push af
twait: in a,(0x6D)
 bit 5,a
 jr z, twait
 pop af
 out (0x68),a
 ret

;
; PropIO v2 block transfers
;
; This is glue specific to the PropIO driver to customise it for a
; given platform. Basically it's the code in common space to do the
; data transfer into any bank.
;

; FIXME: Move these somewhere better
BLKPARAM_ADDR_OFFSET equ 0
BLKPARAM_IS_USER_OFFSET equ 2
BLKPARAM_SWAP_PAGE equ 3

 .export _plt_prop_sd_read
 .export _plt_prop_sd_write

_plt_prop_sd_read:
 pop de
 pop hl
 push hl
 push de
 ld a,(_td_raw)
 or a
 jr z, do_read
 dec a
 ld a, (_td_page)
 jr nz, do_read_a
 ld a, (_udata + 2)
do_read_a:
 call map_proc_a
do_read:
 ld bc,0xAB
 inir
 inir
 jp map_kernel

_plt_prop_sd_write:
 pop de
 pop hl
 push hl
 push de
 ld a,(_td_raw)
 or a
 jr z, do_write
 dec a
 ld a, (_td_page)
 jr nz, do_write_a
 ld a, (_udata + 2)
do_write_a: call map_proc_a
do_write: ld bc,0xAB
 otir
 otir
 jp map_kernel

;
; Stub helpers for code compactness: TODO

 .discard
;
; The first two use an rst as a jump. In the reload sp case we don't
; have to care. In the pop ix case for the function end we need to
; drop the spare frame first, but we know that af contents don't
; matter
;

rstblock:
