# 0 "tricks.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "tricks.S"
;
; For simple banked systems there is a standard implementation. The
; only reason to do otherwise is for speed. A custom bank logic aware
; bank to bank copier will give vastly better fork() performance.
;
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

;
; The number of disk buffers. Must match config.h
;
NBUFS .equ 5
# 7 "tricks.S" 2
# 1 "../../cpu-z80u/kernel-z80.def" 1
# 8 "tricks.S" 2

;
; All of the fixed bank support is available as a library routine,
; however it is a performance sensitive area. Start with
;
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
        ld hl, 0 ; return code set here is ignored, but _switchin can
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
 ld hl, _udata
 ld de, U_DATA_STASH
 ld bc, U_DATA__TOTALSIZE
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

 ld a,1
 ld (_int_disabled),a

;
; FIXME: map_kernel_di ?
;
 call map_kernel

        ld hl, 15
 add hl, de ; process ptr


 ;
 ; Always use the swapstack, otherwise when we call map_kernel
 ; having copied the udata stash back to udata we will crap
 ; somewhere up the stackframe and it's then down to luck
 ; if those bytes are discarded or not.
 ;
 ; Yes - this was a bitch to debug, please don't break it !
 ;
 ld sp, _swapstack

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
 push de
 call _swapper
 pop de
 pop de ; Save an extra copy as C uses the argument stack slot
 pop hl
 ld a,1
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
 ld hl, U_DATA_STASH
 ld de, _udata
 ld bc, U_DATA__TOTALSIZE
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
        ld hl, badswitchmsg
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
        ld de, 3
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
        ld de, 15
        add hl, de
        ; load p_page
        ld c, (hl)
 ; load existing page ptr
 ld a, (_udata + 2)

 ; FIXME: We will redefine this to expect udata as parent and (hl)
 ; as child so it's also clean for multibank. For now just make
 ; sure HL happens to be right
 ; bankfork must preserve ix and iy
 call bankfork ; do the bank to bank copy

 ; Copy done

 call map_proc_always

 ; We are going to copy the uarea into the parents uarea stash
 ; we must not touch the parent uarea after this point, any
 ; changes only affect the child
 ld hl, _udata ; copy the udata from common into the
 ld de, U_DATA_STASH ; target process
 ld bc, U_DATA__TOTALSIZE
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
 ld hl,_udata
 push hl
        ld hl, (fork_proc_ptr)
        push hl
        call _makeproc
        pop af
 pop af

        ; runticks = 0;
        ld hl, 0
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
 ld b, >U_DATA_STASH - PROGBASE
 ld hl, PROGBASE ; base of memory to fork (vectors included)
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
# 14 "tricks.S" 2
;
; As well as using "usermem_std-z80.rel" in your link file for the
; userspace access operations.
;
; We can also use the standard fast user copiers because all the
; kernel objects we need to copy to/from userspace exist in the
; user mapping.
;
# 1 "../../lib/z80uuser1.s" 1
;
; Alternative user memory copiers for systems where all the kernel
; data that is ever moved from user space is visible in the user
; mapping.
;

        ; exported symbols
        .export __uget
        .export __ugetc
        .export __ugetw

        .export __uput
        .export __uputc
        .export __uputw

        .export __uzero

;
; We need these in common as they bank switch
;
 .common
;
; The basic operations are copied from the standard one. Only the
; blk transfers are different. uputget is a bit different as we are
; not doing 8bit loop pairs.
;
uputget:
        ; load DE with the byte count
        ld c, (ix + 8) ; byte count
        ld b, (ix + 9)
 ld a, b
 or c
 ret z ; no work
        ; load HL with the source address
        ld l, (ix + 4) ; src address
        ld h, (ix + 5)
        ; load DE with destination address (in userspace)
        ld e, (ix + 6)
        ld d, (ix + 7)
 ret ; Z is still false

__uputc:
 ld hl,2
 add hl,sp
 ld e,(hl)
 inc hl
 inc hl
 ld a,(hl)
 inc hl
 ld h,(hl)
 ld l,a
 call map_proc_always
 ld (hl), e
 jp map_kernel ; map the kernel back below common

__uputw:
 ld hl,2
 add hl,sp
 ld e,(hl)
 inc hl
 ld d,(hl) ; value
 inc hl
 ld a,(hl)
 inc hl
 ld h,(hl)
 ld l,a
 call map_proc_always
 ld (hl), e
 inc hl
 ld (hl), d
 jp map_kernel

__ugetc:
 pop de
 pop hl
 push hl
 push de
 call map_proc_always
        ld l, (hl)
 ld h, 0
 jp map_kernel

__ugetw:
 pop de
 pop hl
 push hl
 push de
 call map_proc_always
        ld a, (hl)
 inc hl
 ld h, (hl)
 ld l, a
 jp map_kernel

__uput:
 push ix
 ld ix, 0
 add ix, sp
 push bc
 call uputget ; source in HL dest in DE, count in BC
 jr z, uput_out ; but count is at this point magic
 call map_proc_always
 ldir
uput_out:
 call map_kernel
 pop bc
 pop ix
 ld hl, 0
 ret

__uget:
 push ix
 ld ix, 0
 add ix, sp
 push bc
 call uputget ; source in HL dest in DE, count in BC
 jr z, uput_out ; but count is at this point magic
 call map_proc_always
 ldir
 jr uput_out

;
__uzero:
 ld hl,2
 add hl,sp
 push bc
 ld e,(hl)
 inc hl
 ld d,(hl)
 inc hl
 ld c,(hl)
 inc hl
 ld b,(hl)
 ld a, b ; check for 0 copy
 or c
 jr z, popout
 call map_proc_always
 ld l,e
 ld h,d
 ld (hl), 0
 dec bc
 ld a, b
 or c
 jr z, popout
 inc de
 ldir
popout:
 pop bc
 jp map_kernel
# 23 "tricks.S" 2
