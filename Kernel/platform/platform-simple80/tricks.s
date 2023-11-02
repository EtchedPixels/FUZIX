# 0 "tricks.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "tricks.S"
# 1 "kernelu.def" 1
; FUZIX mnemonics for memory addresses etc

U_DATA__TOTALSIZE .equ 0x200 ; 256+256 bytes @ F000
Z80_TYPE .equ 0 ; CMOS

Z80_MMU_HOOKS .equ 0

CONFIG_SWAP .equ 1

PROGBASE .equ 0x0000
PROGLOAD .equ 0x0100

; Mnemonics for I/O ports etc

CONSOLE_RATE .equ 115200

CPU_CLOCK_KHZ .equ 7372

; Z80 CTC ports
CTC_CH0 .equ 0xD0 ; CTC channel 0 and interrupt vector
CTC_CH1 .equ 0xD1 ; CTC channel 1 (periodic interrupts)
CTC_CH2 .equ 0xD2 ; CTC channel 2
CTC_CH3 .equ 0xD3 ; CTC channel 3

NBUFS .equ 4

;
; Select where to put the high code - in our case we need this
; in common
;


HIGHPAGE .equ 0 ; We only have 1 page byte and the low page
    ; isn't used
# 2 "tricks.S" 2
# 1 "../../cpu-z80u/kernel-z80.def" 1
# 3 "tricks.S" 2
# 1 "../../lib/z80usingle.S" 1
;
; Generic handling for single process in memory Z80 systems.
;
; This code could really do with some optimizing.
;
; FIXME: IRQ enable logic during swap
;
        .export _plt_switchout
        .export _switchin
        .export _dofork

 .common

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in. When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
;
; This function can have no arguments or auto variables.
_plt_switchout:
        di
        ; save machine state

        ld hl, 0 ; return code set here is ignored, but _switchin can

        ; return from either _switchout OR _dofork, so they must both write
        ; 14 with the following on the stack:
        push hl ; return code
 push bc
        push ix
        push iy
        ld (_udata + 14), sp ; this is where the SP is restored in _switchin

        ; find another process to run (may select this one again)
        call _getproc

        push hl
        call _switchin

        ; we should never get here
        call _plt_monitor

badswitchmsg: .ascii "_switchin: FAIL"
 .byte 13, 10, 0
swapped: .ascii "_switchin: SWAPPED"
 .byte 13, 10, 0

_switchin:
        di
 ld a,#1
 ld (_int_disabled),a
        pop bc ; return address - ok to trash BC here
        pop de ; new process pointer
;
; FIXME: do we actually *need* to restore the stack !
;
        push de ; restore stack
        push bc ; restore stack

 push de
        ld hl, 15
 add hl, de ; process ptr
 pop de

        ld a, (hl)

 or a
 jr nz, not_swapped
 ;
 ; We are still on the departing processes stack, which is
 ; fine for now.
 ;
 ld sp, _swapstack
 ;
 ; The process we are switching from is not resident. In
 ; practice that means it exited. We don't need to write
 ; it back out as it's dead.
 ;
 ld ix,(_udata + 0)
 ld a, (ix + 15)

 or a
 jr z, not_resident
 ;
 ; DE = process we are switching to
 ;
 push de ; save process
 ; We will always swap out the current process
 ld hl, (_udata + 0)
 push hl
 call _swapout
 pop hl
 pop de ; process to swap to
not_resident:
 push de ; called function may modify on stack arg so make
 push de ; two copies
 call _swapper
 pop de
 pop de

not_swapped:
        ; check u_data->u_ptab matches what we wanted
        ld hl, (_udata + 0) ; u_data->u_ptab
        or a ; clear carry flag
        sbc hl, de ; subtract, result will be zero if DE==HL
        jr nz, switchinfail

 ; wants optimising up a bit
 ld ix, (_udata + 0)
        ; next_process->p_status = 1
        ld (ix + 0), 1

 ; Fix the moved page pointers
 ; Just do one byte as that is all we use on this platform
 ld a, (ix + 15)
 ld (_udata + 2), a
        ; runticks = 0
        ld hl, 0
        ld (_runticks), hl

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        ld sp, (_udata + 14)

        pop iy
        pop ix
 pop bc
        pop hl ; return code

        ; enable interrupts, if the ISR isn't already running
        ld a, (_udata + 16)
 ; put th einterrupt status flag back correctly
 ld (_int_disabled),a
        or a
        ret nz ; in ISR, leave interrupts off
        ei
        ret ; return with interrupts on

switchinfail:
 call outhl
        ld hl, badswitchmsg
        call outstring
 ; something went wrong and we didn't switch in what we asked for
        jp _plt_monitor

fork_proc_ptr:
 .word 0 ; (C type is struct p_tab *) -- address of child process p_tab entry

;
; Called from _fork. We are in a syscall, the uarea is live as the
; parent uarea. The kernel is the mapped object.
;
_dofork:
        ; always disconnect the vehicle battery before performing maintenance
        di ; should already be the case ... belt and braces.

        pop de ; return address
        pop hl ; new process p_tab*
        push hl
        push de

        ld (fork_proc_ptr), hl

        ; Save the stack pointer and critical registers.
        ; When this process (the parent) is switched back in, it will be as if
        ; it returns with the value of the child's pid.
 ld hl,#0
        push hl ; #0 child
 push bc
        push ix
        push iy

        ; save kernel stack pointer -- when it comes back in the child we'll be in
        ; _switchin which will immediately return (appearing to be _dofork()
 ; returning) and with HL (ie return code) containing the child PID.
        ; Hurray.
        ld (_udata + 14), sp

        ; now we're in a safe state for _switchin to return in the parent
 ; process.


        ; Make a new process table entry, etc.

 ; Copy the parent properties into the temporary udata copy
 call _tmpbuf
 ld a,h
 or l
 jp z,ohpoo
 push hl
 ex de,hl
 ld hl, _udata
 ld bc, U_DATA__TOTALSIZE
 ldir

 ; Recover the buffer pointer
 pop hl
 push hl

 ; Make the child udata out of the temporary buffer
 push hl
        ld hl, (fork_proc_ptr)
        push hl
        call _makeproc
        pop bc
 pop bc

        ; in the child process, fork() returns zero.
 ;
 ; And we exit, with the kernel mapped, the child assembled in the
 ; copy area as if it had done a switchout, the parent meanwhile
 ; continues happily on

        ; now we're in a safe state for _switchin to return in the child
 ; process swap out the image and the new udata

 ; Stack the buffer as a second argument
 pop hl
 push hl
 push hl

 ld hl, (fork_proc_ptr)
 push hl
 call _swapout_new
 pop hl
 pop hl

 ; Buffer pointer is sitting top of stack still
 ; so use it as an argument to tmpfree
 call _tmpfree

 pop hl

 ld hl, (fork_proc_ptr)
        ; prepare return value in parent process -- HL = p->p_pid;
        ld de, 3
        add hl, de
        ld a, (hl)
        inc hl
        ld h, (hl)
        ld l, a
        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.
        pop iy
        pop ix
        pop bc
 pop de ; and throw the pid info away
 ; Return pid of child we forked into swap
        ret

ohpoo:
 ld hl,#nobufs
 call outstring
 jp _plt_monitor

nobufs:
 .ascii 'nobufs'
 .byte 0
;
; We can keep a stack in common because we will complete our
; use of it before we switch common block. In this case we have
; a true common so it's even easier.
;
 .ds 128
_swapstack:
# 4 "tricks.S" 2
