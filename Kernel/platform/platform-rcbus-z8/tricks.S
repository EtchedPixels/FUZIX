#include "../../cpu-z8/kernel-z8.def"
#include "kernel.def"

	.common

	.export _plt_switchout
	.export _switchin
	.export _dofork

; Do simple banking until we get it up and going nicely

_plt_switchout:
	clr r3
	push r3
	push r3
	push r11
	push r10
	push r9
	push r8
	push r7
	push r6
	push r5
	push r4
	ld r2,254
	ld r3,255
	ld r15,#>U_DATA__U_SP
	ld r14,#<U_DATA__U_SP
	lde @rr14,r2
	incw rr14
	lde @rr14,r3
	call map_proc_always
	ld r14,#>_udata
	ld r15,#<_udata
	ld r12,#>U_DATA_STASH
	ld r13,#<U_DATA_STASH
	clr r0
l0:
	lde r2,@rr14
	incw rr14
	lde r3,@rr14
	incw rr14
	lde @rr12,r2
	incw @rr12
	lde @rr12,r3
	incw @rr12
	djnz r0, l0

	call map_kernel

	call _getproc
	push r3
	push r2
	call _switchin
	call _plt_monitor

_switchin:
	incw 254	; skip return (discarded)
	incw 254
	pop r14		; process to switch to
	pop r15

	ld r12,r14
	ld r13,r15
	add r13,#P_TAB__P_PAGE_OFFSET
	adc r12,#0

	lde r3,@rr12

	; TODO swap

	ld r12,#>U_DATA__U_PTAB
	ld r13,#<U_DATA__U_PTAB
	lde r0,@rr12
	incw rr12
	lde r1,@rr12
	sub r0,r14
	jr nz, copyback
	sub r1,r15
	jr z, skip_copyback
copyback:
	; Map process r3
	call map_proc_a

	push r14
	push r15
	ld r14,#>_udata
	ld r15,#<_udata
	ld r12,#>U_DATA_STASH
	ld r13,#<U_DATA_STASH
	clr r0
l1:
	lde r2,@rr12
	incw rr12
	lde r3,@rr12
	incw rr12
	lde @rr14,r2
	incw @rr14
	lde @rr14,r3
	incw @rr14
	djnz r0, l1

	pop r15
	pop r14

	ld r12,#>U_DATA__U_PTAB
	ld r13,#<U_DATA__U_PTAB
	lde r0,@rr12
	incw rr12
	lde r1,@rr12
	sub r10,r14
	jr nz, switchfail
	sub r1,r15
	jr nz, switchfail

skip_copyback:
	; 12/13 is a pointer to U_PTAB + 1 at this point for either path
	lde r1,@rr12
	decw rr12
	lde r0,@rr12
	ld r3,#P_RUNNING
	lde @rr0,r3	; Status is byte 0
	add r0,#P_TAB__P_PAGE_OFFSET
	adc r0,#0
	; Now r0 points to PAGE_OFFSET
	lde r3,@rr0		; bank number

	ld r12,#>U_DATA__U_PAGE
	ld r13,#<U_DATA__U_PAGE
	lde @rr12,r3

	clr r3
	ld r12,#>_runticks
	ld r13,#<_runticks
	lde @rr12,r3
	incw rr12
	lde @rr12,r3

	ld r12,#>U_DATA__U_SP
	ld r13,#<U_DATA__U_SP
	lde r2,@rr12
	incw rr12
	lde r3,@rr12
	; Stack back
	ld 254,r2
	ld 255,r3

	ld r12,#>U_DATA__U_ININTERRUPT
	ld r13,#<U_DATA__U_ININTERRUPT
	lde r0,@rr12

	pop r4
	pop r5
	pop r6
	pop r7
	pop r8
	pop r9
	pop r10
	pop r11
	pop r2		; return value
	pop r3

	or r0,r0
	jr nz, in_isr
	ei
in_isr:
	ret

switchfail:
	call outr14
	ld r14,#>badswitchmsg
	ld r15,#<badswitchmsg
	call outstring
	jp _plt_monitor


; Copy 48K between two banks - this one is a bit of a mess with the
; fixed banking setup.
; Should optimize this to do the low 48K fast, then the upper
; Also to use ldei and a buffer in register space
copypages:
	push r15
	push r14
	push r11
	push r10
	push r9
	push r8
	push r7
	push r6
	push r5
	push r4
	clr r14	; pointer
	clr r15
	ld r13,#0xFF	; pointer to MMU register
	ld r12,r13
	ld r2,2
	push 2
	; 8 bytes a loop for 48K
	ld r2,#24
	clr r3
loop:
	and 2,#0xBF	; I/O map on
	; copy from r0 to r1
	lde @rr12,r0	; Set source bank
	or 2,#0x40	; I/O map off
	push r14	; save low ptr
	lde r4,@rr14
	incw rr14
	lde r5,@rr14
	incw rr14
	lde r6,@rr14
	incw rr14
	lde r7,@rr14
	incw rr14
	lde r8,@rr14
	incw rr14
	lde r9,@rr14
	incw rr14
	lde r10,@rr14
	incw rr14
	lde r11,@rr14
	pop r14		; will never cross a boundary mid process
	and 2,#0xBF	; I/O map on
	lde @rr12,r1	; Set dest bank
	lde @rr14,r4
	incw rr14
	lde @rr14,r5
	incw rr14
	lde @rr14,r6
	incw rr14
	lde @rr14,r7
	incw rr14
	lde @rr14,r8
	incw rr14
	lde @rr14,r9
	incw rr14
	lde @rr14,r10
	incw rr14
	lde @rr14,r11
	incw rr14
	djnz r3,loop
	djnz r2,loop
	pop 2		; Restore I/O map
	pop r4
	pop r5
	pop r6
	pop r7
	pop r8
	pop r9
	pop r10
	pop r11
	pop r14
	pop r15
	ret

_dofork:
	ld r15,#2
	call __gargr2

	ld r14,r2
	ld r15,r3
	add r3,#P_TAB__P_PID_OFFSET
	adc r2,#0
	lde r0,@rr2
	incw rr2
	lde r1,@rr2
	; r0/r1 is now pid

	push r1
	push r0		; stack pid as return
	push r11
	push r10
	push r9
	push r8
	push r7
	push r6
	push r5
	push r4
	ld r2,254
	ld r3,255
	ld r15,#>U_DATA__U_SP
	ld r14,#<U_DATA__U_SP
	lde @rr14,r2
	incw rr14
	lde @rr14,r3

	; Stack set up complete

	ld r12,r14
	ld r13,r15
	add r13,#P_TAB__P_PAGE_OFFSET
	adc r12,#0
	lde r1,@rr12	; dest
	ld r12,#>U_DATA__U_PAGE
	ld r13,#<U_DATA__U_PAGE
	lde r0,@rr12	; source

	call copypages

	call map_proc_always

	; UDATA copy

	push r14
	push r15

	; udata to parent stash
	ld r14,#>_udata
	ld r15,#<_udata
	ld r12,#>U_DATA_STASH
	ld r13,#<U_DATA_STASH
	clr r0
l2:
	lde r2,@rr14
	incw rr14
	lde r3,@rr14
	incw rr14
	lde @rr12,r2
	incw @rr12
	lde @rr12,r3
	incw @rr12
	djnz r0,l2

	call map_kernel

	pop r15
	pop r14

	ld r2,#>_udata
	ld r3,#<_udata
	push r3
	push r2
	push r15
	push r14
	call _makeproc
	add 255,#4
	adc 255,#0
	clr r3
	ld r12,#>_runticks
	ld r13,#<_runticks
	lde @rr13,r3
	incw rr12
	lde @rr13,r3
	; Fix up stack and return as child

	adc 255,#10	; frame we pushed
	adc 254,#0
	ret		; to caller of dofork

	.data

	.export _need_resched

_need_resched:
	.byte	0

badswitchmsg:
	.ascii	"badswitch"
	.byte	0
