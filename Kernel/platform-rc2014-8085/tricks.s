# 0 "tricks.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "tricks.S"
# 1 "../kernel-8085.def" 1
; Keep these in sync with struct u_data;;

# 1 "../platform/kernel.def" 1
# 4 "../kernel-8085.def" 2
# 29 "../kernel-8085.def"
; Keep these in sync with struct p_tab;;
# 46 "../kernel-8085.def"
; Keep in sync with struct blkbuf


; Currently only used for 8085
# 2 "tricks.S" 2
# 1 "../lib/8085fixedbank.S" 1
# 1 "../lib/../lib/8085fixedbank-core.S" 1
;
; For a purely bank based architecture this code is common and can be
; used by most platforms
;
; The caller needs to provide the standard map routines along with
; map_kernel_a which maps in the kernel bank in a. This code assumes
; that the bank can be encoded in 8bits.
;

 .setcpu 8085

 .common

.export _plt_switchout

;
; The ABI requires we preserve BC
;

;
; Switch a process out and run the next one
;
_plt_switchout:
 push b
 lxi h,0
 push h ; Save a 0 argment
 ; TODO save temps
 lxi h,0
 dad sp
 shld _udata+14 ; Save the sp for a switch in
 call map_proc_always_di
 ;
 ; Save the udata into process state
 ;
 ; FIXME: can we skip this if the process is defunct or would
 ; it even make sense to defer it (with magic in swapout) ?
 ;
 lxi h,_udata
 lxi d,0xDE00
 call copy512 ; No ldir 8(
 ; Back to kernel
 call map_kernel_di
 call _getproc ; Who goes next
 push d
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
 lxi h,15
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
 lhld _udata+0
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
 lxi h,0xDE00
 lxi d,_udata
 call copy512
 pop d

 lhld _udata+14 ; A valid stack to map on
 sphl

 call map_kernel
 ;
 ; Did we find the right process ?
 ;
 lhld _udata+0
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
 lhld _udata+0
 mvi a,1
 mov m,a
 ldhi 15
 ldax d
 sta _udata+2
 lxi h,0
 shld _runticks
 lhld _udata+14
 sphl
 ;
 ; Recover our parent frame pointer and return code
 ; TODO restore temps
 ;
 pop d
 pop b
 lda _udata+16
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

 lxi d,3
 dad d
 mov a,m
 inx h
 mov h,m
 mov l,a
 ;
 ; We don't have any state to save but the pid and framepointer 
 ; and (alas) a pile of compiler non-reentrant crap
 ;
 push b
 push h
 ; TODO temporaries
 lxi h,0
 dad sp
 shld _udata+14
 ;
 ; We are now in a safe state to work
 ;
 lhld fork_proc_ptr
 ldhi 15
 xchg
 mov c,m
 lda _udata+2

 call bankfork

 call map_proc_always
 ;
 ; Copy the parent udata and stack into the parent stash
 ; The live udata becomes that of the child
 ;
 lxi h,_udata
 lxi d,0xDE00
 call copy512

 call map_kernel

 pop h ; Get rid of saved pid
 pop h ; and C runtime state
 ; TODO correct pops to match new temporaries

 ;
 ; Manufacture the child udata state
 ;
 lxi h,_udata
 push h
 lhld fork_proc_ptr
 push h
 call _makeproc
 pop b
 pop b
 ;
 ; Timer ticks
 ;
 lxi h,0
 shld _runticks
 ;
 ; Frame pointer
 ;
 xchg ; return 0 in DE for child
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
# 2 "../lib/8085fixedbank.S" 2

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
# 3 "tricks.S" 2

 .common

.export bankfork

; Always called with interrupts off

bankfork:
 sta patch1+1 ; interrupts. Might be good to go to a cleaner
 mov a,c ; approach on a faster 8085 ?
 sta patch2+1
 lxi h,0
 dad sp
 shld copy_done+1 ; patch stack restore in

 ; Go from the break to 0-5
 lhld _udata+30
 lxi d,-6 ; move down 6 for the copier loop
 dad d
 sphl
 mvi a,0xff ; end between 5 and 0 (which is fine)
 sta patch3+1
 lxi h,copy_stack
 jmp copier
 ;
 ; Go from DE00 to the stack pointer
 ;
copy_stack:
 lxi sp,0xDE00-6
 ; Trickier .. need to work out where to stop
 lhld _udata+8
 lxi d,-0x0106 ; 6 for the underrun 0x100 for the round down
 dad d
 mov a,h
 sta patch3+1
 lxi h,copy_done
 jmp copier
copy_done:
 lxi h,0
 sphl
 ret

copier:
 shld patch4+1
loop:
    ; sp points to top of block
patch1:
 mvi a,0 ; 7
 out 0xFF ; source bank 10
 pop h ; 10
 pop d ; 10
 pop b ; 10
patch2:
 mvi a,0 ; 7
 out 0xFF ; dest bank 10
 push b ; 11
 push d ; 11
 push h ; sp now back where it started 11
 lxi h,-6 ; 10
 dad sp ; 10
 sphl ; sp ready for next burst 5
 mov a,h ; 5
patch3:
 cpi 0 ; wrapped to FFFx 7
 jnz loop ; 10

;
; 144 cycles per 6 bytes = 24 per byte which is actually not far off
; a naive Z80 implementation and about half a good one. Still means
; a second to do the fork() bank copy on a 1MHz 8080. Not quite so bad
; on a 6MHz 8085 though 8)
;
; We halt at somewhere around xx05-xx00 so we have to tidy up by hand
; or accept an underrun. We go the overlap approach on the grounds
; it's cheap and our main overcopy is at most 5 bytes in common,
; whilst the bank to bank overcopy is harmless and small
;
;
 mvi a,1
 out 0xFF
patch4:
 jmp 0
