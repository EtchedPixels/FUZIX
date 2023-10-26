# 0 "tricks.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "tricks.S"
# 1 "kernelu.def" 1
; UZI mnemonics for memory addresses etc
# 2 "tricks.S" 2
# 1 "../../cpu-z80u/kernel-z80.def" 1
# 3 "tricks.S" 2

# 1 "../../lib/z80ufixedbank.S" 1

# 1 "../../lib/../lib/z80ufixedbank-core.s" 1
;
; For a purely bank based architecture this code is common and can be
; used by most platforms
;
; The caller needs to provide the standard map routines along with
; map_kernel_a which maps in the kernel bank in a. This code assumes
; that the bank can be encoded in 8bits.
;
        .export _plt_switchout
        .export _switchin
        .export _dofork

 .export _need_resched

 .common

; __switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in. When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
;
; This function can have no arguments or auto variables.
;
_plt_switchout:
        ld hl, #0 ; return code set here is ignored, but _switchin can
        ; return from either _switchout OR _dofork, so they must both write
        ; 14 with the following on the stack:
        push hl ; return code
 push bc ; register variable
        push ix
        push iy
 ld hl,(__tmp)
 push hl
 ld hl,(__hireg)
 push hl
 ld hl,(__tmp2)
 push hl
 ld hl,(__tmp2+2)
 push hl
 ld hl,(__tmp3)
 push hl
 ld hl,(__tmp3+2)
 push hl
 ld hl,(__retaddr)
 push hl
        ld (_udata + 14), sp ; this is where the SP is restored in _switchin

 ; Stash the uarea back into process memory
 call map_proc_always
 ld hl, #_udata
 ld de, #0xEE00
 ld bc, #0x0200
 ldir
 call map_kernel

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
        pop bc ; return address (we can trash bc here - we will restore one)
        pop de ; new process pointer
;
; FIXME: do we actually *need* to restore the stack !
;
        push de ; restore stack
        push bc ; restore stack

 ld a,#1
 ld (_int_disabled),a

;
; FIXME: map_kernel_di ?
;
 call map_kernel

        ld hl, #15
 add hl, de ; process ptr



 ;
 ; Always use the swapstack, otherwise when we call map_kernel
 ; having copied the udata stash back to udata we will crap
 ; somewhere up the stackframe and it's then down to luck
 ; if those bytes are discarded or not.
 ;
 ; Yes - this was a bitch to debug, please don't break it !
 ;
 ld sp, #_swapstack

        ld a, (hl)

 or a
 jr nz, not_swapped

 ;
 ; Re-enable interrupts while we swap. This is ok because
 ; we are not on the IRQ stack when switchin is invoked.
 ;
 ; There are two basic cases
 ; #1: pre-emption. Not in a system call, must avoid
 ; re-entering pre-emption logic, Z80 lowlevel code sets U_INSYS
 ; #2: kernel syscall. Also protected by 6
 ;
 ei
 xor a
 ld (_int_disabled),a
 push hl
 push de
 call _swapper
 pop de
 pop hl
 ld a,#1
 ld (_int_disabled),a
 di

 ld a, (hl)
not_swapped:
 push hl
 ld hl, (_udata + 0)
 or a
 sbc hl, de
 pop hl
 jr z, skip_copyback ; Tormod's optimisation: don't copy the
    ; the stash back if we are the task who
    ; last owned the real udata
 ld a,(hl)
 ; Pages please !
 call map_proc_a

        ; bear in mind that the stack will be switched now, so we can't use it
 ; to carry values over this point

 exx ; thank goodness for exx 8)
 ld hl, #0xEE00
 ld de, #_udata
 ld bc, #0x0200
 ldir
 exx

 ; In the non swap case we must set so before we use the stack
 ; otherwise we risk corrupting the restored stack frame
        ld sp, (_udata + 14)
 call map_kernel

        ; check u_data->u_ptab matches what we wanted
        ld hl, (_udata + 0) ; u_data->u_ptab
        or a ; clear carry flag
        sbc hl, de ; subtract, result will be zero if DE==HL
        jr nz, switchinfail

skip_copyback:
 ; wants optimising up a bit
 ld ix, (_udata + 0)
        ; next_process->p_status = 1
        ld (ix + 0), #1

 ; Fix the moved page pointers
 ; Just do one byte as that is all we use on this platform
 ld a, (ix + 15)
 ld (_udata + 2), a
        ; runticks = 0
        ld hl, #0
        ld (_runticks), hl

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        ld sp, (_udata + 14)

 pop hl
        ld (__retaddr),hl
 pop hl
 ld (__tmp3+2),hl
 pop hl
 ld (__tmp3),hl
 pop hl
 ld (__tmp2+2),hl
 pop hl
 ld (__tmp2),hl
 pop hl
 ld (__hireg),hl
 pop hl
 ld (__tmp),hl
        pop iy ; register variables
        pop ix
 pop bc
        pop hl ; return code

        ; enable interrupts, if we didn't pre-empt in an ISR
        ld a, (_udata + 16)
 ld (_int_disabled),a
        or a
        ret nz ; Not an ISR, leave interrupts off
        ei
        ret ; return with interrupts on

switchinfail:
 call outhl
        ld hl, #badswitchmsg
        call outstring
 ; something went wrong and we didn't switch in what we asked for
        jp _plt_monitor


;
; Called from _fork. We are in a syscall, the uarea is live as the
; parent uarea. The kernel is the mapped object.
;
_dofork:
        ; always disconnect the vehicle battery before performing maintenance
        di ; should already be the case ... belt and braces.

 ;
 ; FIXME: we should no longer need interrupts off for most of a
 ; fork() call.
 ;
        pop de ; return address
        pop hl ; new process p_tab*
        push hl
        push de

        ld (fork_proc_ptr), hl

        ; prepare return value in parent process -- HL = p->p_pid;
        ld de, #3
        add hl, de
        ld a, (hl)
        inc hl
        ld h, (hl)
        ld l, a

        ; Save the stack pointer and critical registers.
        ; When this process (the parent) is switched back in, it will be as if
        ; it returns with the value of the child's pid.
        push hl ; HL still has p->p_pid from above, the return value in the parent
 push bc
        push ix
        push iy
 ld hl,(__tmp)
 push hl
 ld hl,(__hireg)
 push hl
 ld hl,(__tmp2)
 push hl
 ld hl,(__tmp2+2)
 push hl
 ld hl,(__tmp3)
 push hl
 ld hl,(__tmp3+2)
 push hl
 ld hl,(__retaddr)
 push hl

        ; save kernel stack pointer -- when it comes back in the parent we'll be in
        ; _switchin which will immediately return (appearing to be _dofork()
 ; returning) and with HL (ie return code) containing the child PID.
        ; Hurray.

        ld (_udata + 14), sp

        ; now we're in a safe state for _switchin to return in the parent
 ; process.

        ; --------- copy process ---------

        ld hl, (fork_proc_ptr)
        ld de, #15
        add hl, de
        ; load p_page
        ld c, (hl)
 ; load existing page ptr
 ld a, (_udata + 2)

 ; FIXME: We will redefine this to expect udata as parent and (hl)
 ; as child so it's also clean for multibank. For now just make
 ; sure HL happens to be right
 call bankfork ; do the bank to bank copy

 ; Copy done

 call map_proc_always

 ; We are going to copy the uarea into the parents uarea stash
 ; we must not touch the parent uarea after this point, any
 ; changes only affect the child
 ld hl, #_udata ; copy the udata from common into the
 ld de, #0xEE00 ; target process
 ld bc, #0x0200
 ldir
 ; Return to the kernel mapping
 call map_kernel

        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.
 ; ix/iy are untouched so don't need a restore BC is not so does
 ld hl,18
 add hl,sp
 ld sp,hl
        pop bc
 pop af ; and the pid

        ; Make a new process table entry, etc.
 ld hl,#_udata
 push hl
        ld hl, (fork_proc_ptr)
        push hl
        call _makeproc
        pop af
 pop af

        ; runticks = 0;
        ld hl, #0
        ld (_runticks), hl
        ; in the child process, fork() returns zero.
 ;
 ; And we exit, with the kernel mapped, the child now being deemed
 ; to be the live uarea. The parent is frozen in time and space as
 ; if it had done a switchout().
        ret

 .commondata
;
; For the moment
;
bouncebuffer:
 .ds 256
;
; We can keep a stack in common because we will complete our
; use of it before we switch common block. In this case we have
; a true common so it's even easier. This can share with the bounce
; buffer used by bankfork as we won't switchin mid way through the
; banked fork() call.
;
_swapstack:
_need_resched: .byte 0
fork_proc_ptr: .word 0 ; (C type is struct p_tab *) -- address of child process p_tab entry
# 3 "../../lib/z80ufixedbank.S" 2
;
; This is related so we will keep it here. Copy the process memory
; for a fork. a is the page base of the parent, c of the child
;
; Assumption - fits into a fixed number of whole 256 byte blocks
;

 .common

bankfork:
 ld b, >0xEE00 - 0x0000
 ld hl, 0x0000 ; base of memory to fork (vectors included)
bankfork_1:
 push bc ; Save our counter and also child offset
 push hl
 call map_proc_a
 ld de, bouncebuffer
 ld bc, 256
 ldir ; copy into the bounce buffer
 pop de ; recover source of copy to bounce
    ; as destination in new bank
 pop bc ; recover child page number
 push bc
 ld b, a ; save the parent bank id
 ld a, c ; switch to the child
 call map_proc_a
 push bc ; save the bank pointers
 ld hl, bouncebuffer
 ld bc, 256
 ldir ; copy into the child
 pop bc ; recover the bank pointers
 ex de, hl ; destination is now source for next bank
 ld a, b ; parent bank is wanted in a
 pop bc
 djnz bankfork_1 ; rinse, repeat
 ret
# 5 "tricks.S" 2
