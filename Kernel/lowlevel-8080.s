# 0 "lowlevel-8080.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "lowlevel-8080.S"

;
; 8085 low level code
;
; Much the same as the Z80 code except we don't provide in and out
; helpers because it's impossible to make them re-entrant for any port
;
 .setcpu 8080

# 1 "kernel-8085.def" 1
; Keep these in sync with struct u_data;;

# 1 "platform/kernel.def" 1
# 4 "kernel-8085.def" 2
# 29 "kernel-8085.def"
; Keep these in sync with struct p_tab;;
# 46 "kernel-8085.def"
; Keep in sync with struct blkbuf


; Currently only used for 8085
# 11 "lowlevel-8080.S" 2

;
; CPU setup and properties. As we are hardcoded for 8085 this isn't
; too hard
;
  .data

.export _sys_cpu
.export _sys_cpu_feat
.export _sys_stubs

_sys_cpu:
 .byte 1 ; 8080 family
_sys_cpu_feat:
 .byte 1 ; 8085 feature set

_sys_stubs:
 jmp unix_syscall_entry
 nop
 nop
 nop
 nop
 nop
 nop
 nop
 nop
 nop
 nop
 nop
 nop
 nop

.export deliver_signals_2
.export deliver_signals

  .common

deliver_signals:
 lda _udata+17
 ora a
 rz
deliver_signals_2:
 mov l,a
 mvi h,0
 dad h
 lxi d,_udata+34
 dad d
 mov e,m
 inx h
 mov d,m


 ; Build the return frame
 lxi b,signal_return
 push b

 mov c,a ; save the signal number to pass into the helper

 xra a
 sta _udata+17
 ;
 ; Do we need to zero check de here ?
 ;
 mov a,d
 ora e
 jz signal_return ; raced
 ;
 ; Off we go. DE = vector C = signal
 ;
 ; FIXME: if we ever have 8080 binaries with different load
 ; addresses we will need to fix this
 ;
 ei
 lhld 0x0100 +16 ; signal vector
 pchl
signal_return:
 di
 lxi h,0
 dad sp
 shld _udata+8
 lxi sp,kstack_top
 mvi a,1
 sta _int_disabled
 call map_kernel_di
 call _chksigs
 call map_proc_always_di
 lhld _udata+8
 sphl
 jmp deliver_signals

.export unix_syscall_entry

unix_syscall_entry:
 push b ; Must preserve the frame pointer


 sta _udata+7
 ; Oh for LDIR
 ; Unroll this for speed. Syscall arguments into constant locations
 lxi h,6
 dad sp
 mov e,m
 inx h
 mov d,m
 inx h
 xchg
 shld _udata+18
 xchg
 mov e,m
 inx h
 mov d,m
 inx h
 xchg
 shld _udata+18 +2
 xchg
 mov e,m
 inx h
 mov d,m
 inx h
 xchg
 shld _udata+18 +4
 xchg
 mov e,m
 inx h
 mov d,m
 inx h
 xchg
 shld _udata+18 +6
 xchg

 di

 ; We are now in kernel space
 mvi a,1
 sta _udata+6
 ; Switch stacks
 ; On 8080 this is a bit more long winded as we have to go via HL
 lxi h,0
 dad sp
 shld _udata+8
 lxi sp, kstack_top
 ;
 ; Now map the kernel and call it
 ;
 call map_kernel_di
 ei
 call _unix_syscall
 xchg
 ;
 ; Remember fork and execve don't necessarily return this way and fork
 ; can do it twice
 ;
 di
 call map_proc_always
 xra a
 sta _udata+6
 ; Switch stack back
 lhld _udata+8
 sphl
 lhld _udata+10
 xchg
 lhld _udata+12
 ;
 ; Signal check
 ;
 lda _udata+17
 ora a
 jnz via_signal
unix_return:
 mov a,h
 ora l
 jz not_error
 stc
 ; Carry and errno in HL as expected
 jmp unix_pop
not_error:
 ; Retval in HL as the Z80 kernel returns it
 xchg
unix_pop:
 pop b
 ; ret must directly follow the ei
 ei
 ret
via_signal:
 ;
 ; Stack the state (a signal doing a syscall will change the
 ; U_DATA fields but we must return the old error/status)
 ;
 lhld _udata+12
 push h
 lhld _udata+10
 push h
 ;
 ; And into the signal delivery path
 ;
 call deliver_signals_2
 pop d
 pop h
 jmp unix_return

;
; Called when execve() completes to transition to the user, as we
; don't return from execve() via the syscall path
;
;
.export _doexec

_doexec:
 di
 call map_proc_always
 pop b
 pop d
 lhld _udata+26
 sphl
 xra a
 sta _udata+6
 xchg
 mov d,h
 mvi e,0
 ei
 pchl
;
; NULL trap. Must live in common space
;
; FIXME: Rewrite 68000 style as a synchronous trap
;
.export null_handler

null_handler:
 lda _udata+6
 ora a
 jnz trap_illegal
 lda _udata+16
 ora a
 jnz trap_illegal
 lxi h,7
 push h
 lhld _udata+0
 inx h
 inx h
 inx h ; P_PID_OFFSET
 mov e,m
 inx h
 mov d,m
 push d
 lxi h,39
 push h
 call unix_syscall_entry
 lxi h,0xffff ; exit -1
 push h
 dcx h
 push h
 call unix_syscall_entry
 ; Never returns

trap_illegal:
 lxi h,illegalmsg
traphl:
 call outstring
 call _plt_monitor

.export nmi_handler

nmi_handler:
 call map_kernel_di
 lxi h,nmimsg
 jmp traphl

illegalmsg:
 .ascii '[illegal]'
 .byte 0
nmimsg:
 .ascii '[NMI]'
 .byte 0

;
; Interrupts are similar to Z80 but we have a lot less state
; to store, and rather trickier juggling to get signals nice
;
.export interrupt_handler

interrupt_handler:
 push psw

 push b
 push d
 push h
 call plt_interrupt_all
 ; Switch stacks
 lxi h,0
 dad sp
 shld istack_switched_sp
 lxi sp,istack_top

 ;
 ; Map the kernel
 ;
 lda 0
 call map_save_kernel
 cpi 0xC3
 cnz null_pointer_trap
 ;
 ; Set up state and enter kernel
 ;
 mvi a,1
 sta _udata+16
 sta _int_disabled
 ;
 ; What we avoid in register saves over Z80 we make up for in
 ; runtime stuff
 ;
 ; C temporaries
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

 call _plt_interrupt

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

 ;
 ; Do we need to task switch ?
 ;
 lda _need_resched
 ora a
 jnz preemption
 ;
 ; Switch stacks back
 ;
 call map_restore
 lhld istack_switched_sp
 sphl
intout:
 xra a
 sta _udata+16

 ; Usually null but some platforms
    ; have hacks for Z80 peripherals

 lda _udata+6
 ora a
 jnz interrupt_pop
 call deliver_signals
 ;
 ; Restore registers and done
 ;
interrupt_pop:
 xra a
 sta _int_disabled
 pop h
 pop d
 pop b

 pop psw
 ei
 ret

null_pointer_trap:
 mvi a,0xc3
 sta 0
 lxi h,11
trap_signal:
 push h
 lhld _udata+0
 push h
 call _ssig
 pop h
 pop h
 ret

;
; Now the scary stuff - preempting
;
preemption:
 xra a
 sta _need_resched
 ;
 ; Save our original stack in syscall_s
 ; Move to our kernel stack (free because we don't preempt
 ; in kernel
 ;
 lhld istack_switched_sp
 shld _udata+8
 lxi sp,kstack_top

 ;
 ; So any Z80 style devices see an 'iret'
 ;


 ;
 ; Mark ourselves as in a system call
 ;
 mvi a,1
 sta _udata+6
 call _chksigs
 lhld _udata+0
 mvi a,1
 cmp m
 jnz not_running
 mvi m,2
 ;
 ; Punish the process for using all of its time.
 ;
 inx h
 mvi a,4
 ora m
 mov m,a
not_running:
 ;
 ; We will disappear into this and reappear somewhere else. In
 ; time we will reappear here
 ;
 call _plt_switchout
 ;
 ; We are back in the land of the living so no longer in
 ; syscall or interrupt state
 ;
 xra a
 sta _udata+16
 sta _udata+6
 ;
 ; Get our mapping back
 ;
 call map_proc_always_di
 ;
 ; And our stack
 ;
 lhld _udata+8
 sphl
 lda _udata+17
 ora a
 cnz deliver_signals_2
 jmp interrupt_pop

;
; Debug code
;
.export outstring

outstring:
 mov a,m
 ora a
 rz
 call outchar
 inx h
 jmp outstring

.export outstringhex

outstringhex:
 mov a,m
 ora a
 rz
 call outcharhex
 mvi a,0x20
 call outchar
 inx h
 jmp outstringhex

.export outnewline

outnewline:
 mvi a,0x0d
 call outchar
 mvi a,0x0a
 jmp outchar

.export outhl

outhl:
 push psw
 mov a,h
 call outcharhex
 mov a,l
 call outcharhex
 pop psw
 ret

.export outde

outde:
 push psw
 mov a,d
 call outcharhex
 mov a,e
 call outcharhex
 pop psw
 ret

.export outbc

outbc:
 push psw
 mov a,b
 call outcharhex
 mov a,c
 call outcharhex
 pop psw
 ret

.export outcharhex

outcharhex:
 push b
 push psw
 mov c,a
 rar
 rar
 rar
 rar
 call outnibble
 mov a,c
 call outnibble
 pop psw
 pop b
 ret

outnibble:
 ani 0x0f
 cpi 10
 jc numeral
 adi 7
numeral:
 adi 0x30 ; '0'
 jmp outchar


.export ___hard_ei

___hard_ei:
 xra a
 sta _int_disabled
 ei
 ret

.export ___hard_di

___hard_di:
 lxi h, _int_disabled
 di
 mov a,m
 mvi m,1
 mov e,a
 ret

.export ___hard_irqrestore

___hard_irqrestore:
 lxi h,2
 dad sp
 di
 mov a,m
 sta _int_disabled
 ora a
 rnz
 ei
 ret

;
; Identify 8080 variants. We don't worry about Z80 variants. The 8085
; kernel doesn't work on a Z80 so we don't care which one we have.
;
.export _cpu_detect

_cpu_detect:
 ; Ok start with the flags
 mvi a,255
 inr a
 push psw
 pop h
 mov a,l
 ani 0x82
 cpi 0x80
 jz lr35902
 ora a
 jnz is808x
 lxi d,0 ; Z80: we don't care which kind. It's simply not allowed
 ret
lr35902:
 lxi d,0 ; also not allowed
 ret
is808x:
 xra a
 .byte 0x20 ; RIM on 8085, no-op on 8080
 ora a
 jnz is8085 ; it changed must be an 8085
 ;
 ; But it could really be 0
 ;
 inr a
 .byte 0x20 ; RIM / no-op
 ora a
 jz is8085
 ;
 ; TODO: check for KP580M1
 ;
 lxi h,8080
 ;
 ; But wait it might be a 9080
 ;
 mvi a,255
 ani 255
 push psw
 pop h
 mov a,l
 ani 0x10 ; half carry is zero on AMD
 rnz
 mvi h,0x90
 ret
is8085:
 lxi h,0x8085
 ret


;
; We need to worry about bits of this in interrupt save and restore
;
;
; I/O helpers. ACK lacks inline assembly which is annoying. Doubly so
; because 8080/8085 can only access variable ports by self modifying
; code.. which is fun with interrupts. Bletch;
;
; For speed critical cases you need asm stubs, for the others these
; will do.
;
.export _in
.export _out

_in:
 lda _int_disabled
 push psw
 lxi h,4
 dad sp
 mov e,m
 di
 sta inpatch+1
inpatch:
 in 0
 mov l,a
 mvi h,0
popout:
 pop psw
 ora a
 rnc
 ei
 ret

_out:
 lda _int_disabled
 push psw
 lxi h,4
 dad sp
 mov a,m
 di
 sta outpatch+1
 inx h
 inx h
 mov a,m
outpatch:
 out 0
 jmp popout

.export _set_cpu_type

_set_cpu_type:
 ret
