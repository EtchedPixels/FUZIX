# 0 "../lib/8080fixedbank.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "../lib/8080fixedbank.S"
# 1 "../lib/../lib/8080fixedbank-core.S" 1
;
; For a purely bank based architecture this code is common and can be
; used by most platforms
;
; The caller needs to provide the standard map routines along with
; map_kernel_a which maps in the kernel bank in a. This code assumes
; that the bank can be encoded in 8bits.
;

 .setcpu 8080

 .common

.export _plt_switchout

;
; The ABI requires we preserve BC
;

;
; Switch a process out and run the next one
;
_plt_switchout:
 lxi h,0 ; save a 0 return
 ;
 ; We don't have any state to save but the pid and BC
 ; and (alas) a pile of compiler non-reentrant crap
 ;
 push b ; old BC
 push h ; pid
 lhld __tmp
 push h
 lhld __hireg
 push h
 lhld __tmp2
 push h
 lhld __tmp2+2
 push h
 lhld __tmp3
 push h
 lhld __tmp3+2
 push h
 lhld __retaddr
 push h
 lxi h,0
 dad sp
 shld U_DATA__U_SP ; Save the sp for a switch in
 call map_proc_always_di
 ;
 ; Save the udata into process state
 ;
 ; FIXME: can we skip this if the process is defunct or would
 ; it even make sense to defer it (with magic in swapout) ?
 ;
 lxi h,_udata
 lxi d,U_DATA_STASH
 call copy512 ; No ldir 8(
 ; Back to kernel
 call map_kernel_di
 call _getproc ; Who goes next
 push h
 call _switchin ; Run it
 ; Should never hit this
 call _plt_monitor

;
; Switch a process back in
;
.export _switchin

;
; Switch to a given process
;
_switchin:
 di
 pop b
 pop d ; DE is now our process to run
 push d
 push b

 call map_kernel_di

 ;
 ; Is it swapped (page 0)
 ;
 lxi h,P_TAB__P_PAGE_OFFSET
 dad d

 lxi sp,_swapstack
 mov a,m ; A is now our bank
 ora a
 jnz not_swapped
 ;
 ; Swapping time. Interrupts back on
 ;
 ei
 xra a
 sta _int_disabled
 ;
 ; Can we rely on the compiler not mashing the stacked var ?
 ;
 push h
 push d
 call _swapper
 pop d
 pop h
 ;
 ; It should now be back in memory
 ;
 mvi a,1
 sta _int_disabled
 di
 mov a,m
 ;
 ; Check if we need to recover the udata (we were last to run)
 ;
not_swapped:
 mov c,a
 ; DE is the process to run, get the udata process into HL
 lhld U_DATA__U_PTAB
 ; See if our udata is live - this is common, if we sleep and we are
 ; the next to run for example.
 mov a,h
 cmp d
 jnz copyback
 mov a,l
 cmp e
 jz skip_copyback
 ;
 ; Recover the udata
 ;
copyback:
 mov a,c
 call map_proc_a

 push d
 lxi h,U_DATA_STASH
 lxi d,_udata
 call copy512
 pop d

 lhld U_DATA__U_SP ; A valid stack to map on
 sphl

 call map_kernel
 ;
 ; Did we find the right process ?
 ;
 lhld U_DATA__U_PTAB
 mov a,h
 cmp d
 jnz switchinfail
 mov a,l
 cmp e
 jnz switchinfail

skip_copyback:
 ;
 ; Mark us as running, clear our preemption counter
 ; and set the interrupt flags
 ;
 lhld U_DATA__U_PTAB
 mvi a,P_RUNNING
 mov m,a
 ldhi P_TAB__P_PAGE_OFFSET
 ldax d
 sta U_DATA__U_PAGE
 lxi h,0
 shld _runticks
 lhld U_DATA__U_SP
 sphl
 ;
 ; Recover our parent frame pointer and return code
 ;
 pop h
 shld __retaddr
 pop h
 shld __tmp3+2
 pop h
 shld __tmp3
 pop h
 shld __tmp2+2
 pop h
 shld __tmp2
 pop h
 shld __hireg
 pop h
 shld __tmp
 pop h
 pop b
 lda U_DATA__U_ININTERRUPT
 sta _int_disabled
 ora a
 rnz
 ei
 ret

switchinfail:
 call outhl
 lxi h,badswitchmsg
 call outstring
 call _plt_monitor

badswitchmsg:
 .ascii 'badsw'
 .byte 0


fork_proc_ptr:
 .word 0

.export _dofork
;
; The heart of fork
;
_dofork:
 ldsi 2
 lhlx

 shld fork_proc_ptr

 lxi d,P_TAB__P_PID_OFFSET
 dad d
 mov a,m
 inx h
 mov h,m
 mov l,a
 ;
 ; We don't have any state to save but the pid and BC
 ; and (alas) a pile of compiler non-reentrant crap
 ;
 push b ; old BC
 push h ; pid
 lhld __tmp
 push h
 lhld __hireg
 push h
 lhld __tmp2
 push h
 lhld __tmp2+2
 push h
 lhld __tmp3
 push h
 lhld __tmp3+2
 push h
 lhld __retaddr
 push h
 lxi h,0
 dad sp
 shld U_DATA__U_SP
 ;
 ; We are now in a safe state to work
 ;
 lhld fork_proc_ptr
 ldhi P_TAB__P_PAGE_OFFSET
 xchg
 mov c,m
 lda U_DATA__U_PAGE

 call bankfork

 call map_proc_always
 ;
 ; Copy the parent udata and stack into the parent stash
 ; The live udata becomes that of the child
 ;
 lxi h,_udata
 lxi d,U_DATA_STASH
 call copy512

 call map_kernel

 lxi h,16 ; Drop the stack frame
 dad sp
 sphl

 ;
 ; Manufacture the child udata state
 ;
 lxi h,_udata
 push h
 lhld fork_proc_ptr
 push h
 call _makeproc
 pop d
 pop d
 ;
 ; Timer ticks
 ;
 lxi h,0
 shld _runticks
 ;
 ; Recover B and return HL = 0 (child)
 ;
 pop b
 ret

.export bouncebuffer
.export _swapstack

bouncebuffer:
 .ds 256 ; Do we really need 256 ?
_swapstack:

.export _need_resched

_need_resched:
 .byte 0
# 2 "../lib/8080fixedbank.S" 2

;
; Fast copy 512 bytes from H to D
;
copy512:
 mvi b,64
copy8:
 mov a,m
 stax d
 inx h
 inx d
 mov a,m
 stax d
 inx h
 inx d
 mov a,m
 stax d
 inx h
 inx d
 mov a,m
 stax d
 inx h
 inx d
 mov a,m
 stax d
 inx h
 inx d
 mov a,m
 stax d
 inx h
 inx d
 mov a,m
 stax d
 inx h
 inx d
 mov a,m
 stax d
 inx h
 inx d
 dcr b
 jnz copy8
 ret
