# 1 "lowlevel-68hc11.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "lowlevel-68hc11.S"

 .file "lowlevel-68hc11"
 .mode mshort


 .globl di
 .globl ei
 .globl irqrestore

 .globl unix_syscall_entry
 .globl dispatch_process_signal
 .globl interrupt_handler

 .globl outnewline
 .globl outcharhex
 .globl outstring
 .globl outx
 .globl outy
 .globl outd

# 1 "platform/kernel.def" 1
; FUZIX mnemonics for memory addresses etc

; (this is struct u_data from kernel.h)
; 256+256 bytes.
U_DATA__TOTALSIZE = 0x200

PROGLOAD = 0x0100
# 22 "lowlevel-68hc11.S" 2
# 1 "kernel-hc11.def" 1
# 23 "lowlevel-68hc11.S" 2

 .section code

 .globl set_cpu_type
 .globl sys_cpu
 .globl sys_cpu_feat
 .globl sys_stubs

set_cpu_type:
 rts ; no variants to care about
sys_cpu:
 .byte 2 ; 6800 class CPU
sys_cpu_feat:
 .byte 5 ; 6800 with 6801/3 and 68HC11 features
sys_stubs:
 swi
 rts
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
 nop

 .section common

di:
 tpa ; return cc codes in D
 sei
 rts

ei:
 cli
 rts

irqrestore: ; D holds the return from di where A is the cc
 tap ; we trash overflow and carry but they are assumed
 rts ; clobbered anyway


outnewline:
 ldab #0x0d
 bsr outchar_call
 ldab #0x0a
 bra outchar_call


outcharhex:
 pshb
 lsrb
 lsrb
 lsrb
 lsrb
 bsr outnibble
 pulb
 pshb
 bsr outnibble
 pulb
 rts

outnibble:
 andb #0x0F
 cmpb #0x0A
 ble outh2
 addb #0x07
outh2: addb #0x30
outchar_call:
 jmp outchar

outstring:
 ldab ,x
 beq outsdone
 bsr outchar_call
 inx
 bra outstring

outx:
 xgdx
 pshx ; actually the old D
 bsr outcharhex
 tab
 bsr outcharhex
 pulx
 xgdx
outsdone:
 rts

outy:
 xgdy
 pshy ; actually the old D
 bsr outcharhex
 tab
 bsr outcharhex
 puly
 xgdy
 rts

outd:
 psha
 pshb
 bsr outcharhex
 tab
 bsr outcharhex
 pulb
 pula
 rts


;
; This is slightly odder than most platforms. At the point we
; are called the arguments should already be in U_foo as we may be
; doing a pure bank switch environment and will switch to the kernel
; stack before this far call via the EEPROM
;
unix_syscall_entry:
 ldaa #1
 staa 6 ; we may want to use udata-> tricks ?
 jsr map_kernel ; no-op in pure banked
 cli
 jsr unix_syscall
 sei
 clr 6
 jmp map_process_always ; no-op in pure banked
 ; signal processing happens in per platform code in case we are
 ; pure banked
 ; Caller must save errno and return value before invoking signal
 ; processing.

;
; May be a trampoline via a far call and bank switch, but not our
; problem in this code. We do however assume the caller stack switched
; for us to the IRQ stack
;
interrupt_handler:
 ldaa #1
 staa 16
 jsr map_save_kernel
 ldaa #1
 staa inint
 jsr platform_interrupt
 clr inint
 tst need_resched
 beq noswitch
 clr need_resched
 ; Save the stack pointer across
 ldd istack_switched_sp
 std 8
 lds #kstack_top
 jsr chksigs
 ldx 0
 ldab 0,x
 cmpb #1
 bne not_running
 ldab #2
 stab 0,x
not_running:
 jsr platform_switchout
 jsr map_process_always
 ; caller will switch back to stack in X
 ldx 8
 rts
noswitch:
 jsr map_restore
 ldx istack_switched_sp
 rts ; caller will do the final stack flip

nmi_handler:
 lds #istack_top - 2
 jsr map_kernel
 ldx #nmimsg
 jsr outstring
 jsr platform_monitor

nmimsg:
 .ascii "[NMI]"
 .byte 13,10,0

;
; Runs in kernel banking
;
dispatch_process_signal:
 ldab 17
 bne dosig
 rts
dosig:
 clr 17
 clra
 ldx #32
 abx
 abx
 ldd ,x
 jmp sigdispatch ; platform provides. Calls d on the
    ; user bank and stack. If it returns
    ; then will go back via
    ; dispatch_process_signal


;
; Illegal instruction trap helper. Should send a signal
;
sigill:
 rts

;
; Helper for byte swap
;
 .globl swab

swab:
 psha
 tba
 pulb
 rts
# 293 "lowlevel-68hc11.S"
 .sect .softregs
 .globl _.tmp
 .globl _.z,_.xy
_.tmp: .dc.w 1; .type _.tmp,@object ; .size _.tmp,2
_.z: .dc.w 1; .type _.z,@object ; .size _.z,2
_.xy: .dc.w 1; .type _.xy,@object ; .size _.xy,2
 .sect .softregs
 .globl _.frame
_.frame: .dc.w 1; .type _.frame,@object ; .size _.frame,2
 .sect .softregs
 .globl _.d1,_.d2
_.d1: .dc.w 1; .type _.d1,@object ; .size _.d1,2
_.d2: .dc.w 1; .type _.d2,@object ; .size _.d2,2
 .sect .softregs
 .globl _.d3,_.d4
_.d3: .dc.w 1; .type _.d3,@object ; .size _.d3,2
_.d4: .dc.w 1; .type _.d4,@object ; .size _.d4,2

 .globl memcpy

memcpy:
 xgdy
 tsx
 ldd 5,x
 ldx 3,x ; SRC = X, DST = Y
 cpd #0
 beq End
 pshy
 inca ; Correction for the deca below
L0:
 psha ; Save high-counter part
L1:
 ldaa 0,x ; Copy up to 256 bytes
 staa 0,y
 inx
 iny
 decb
 bne L1
 pula
 deca
 bne L0
 puly ; Restore Y to return the DST
End:
 xgdy
 rts

 .globl memset

;;; D = dst Pmode
;;; 2,sp = src SImode
;;; 6,sp = size HImode (size_t)
memset:
 xgdx
 tsy
 ldab 6,y
 ldy 7,y ; DST = X, CNT = Y
 beq L3
 pshx
L2:
 stab 0,x ; Fill up to 256 bytes
 inx
 dey
 bne L2
 pulx ; Restore X to return the DST
L3:
 xgdx
 rts

;
; This one isn't taken from gcc	
;
 .globl strlen

strlen:
 xgdx
 ldd #0
L4:
 tst ,x
 beq L5
 inx
 addd #1
 bra L4
L5: rts

;
; Support routines (FIXME copy over)
;
 .globl ___ashrsi3
 .globl ___ashlsi3
 .globl ___lshlhi3
 .globl ___lshlsi3
 .globl ___lshrhi3
 .globl ___lshrsi3
 .globl ___one_cmplsi2
 .globl ___mulhi3


___ashrsi3:
___ashlsi3:
___lshlhi3:
___lshlsi3:
___lshrsi3:
___lshrhi3:
___one_cmplsi2:
___mulhi3:
 rts
