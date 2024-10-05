;
;	TMS9995 version
;
;	Just to get us going
;
        .export _plt_switchout
        .export _switchin
        .export _dofork
	.export _ramtop

#include "kernel.def"
#include "../../cpu-tms9995/kernel-tms9995.def"

	.common

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here
_ramtop:
	.word 0xF000	; FIXME check


;
;	The nice thing about branch-link architectures is it makes stack
;	helpers really clean. These always run with interrupts off.
;
save_userws:
	li	r0,user_ws
	li	r1,16
	ai	r13,-32		; Frame size
save_r:
	mov	*r0+,*r13+
	dec	r1
	jne	@save_r

	ai	r13,-32		; Fix the frame pointer back
	bl	*r11

restore_userws:
	li	r0,user_ws
	li	r1,16
restore_r:
	mov	*r13+,*r0+
	dec	r1
	jne	@restore_r
	bl	*r11


; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
;
_plt_switchout:
	limi	IRQ_OFF
	; Save the link
	dect	r13
	mov	r11,*r13
	dect	r13
	clr	*r13			; Return code
	;
	; Save the user workspace. We have a lot of state on this CPU but
	; we only take the hit on actual context switching not interrupts
	; and we don't switch much.
	;
	bl	@save_userws
	; Save the registers we need (r0-r7 are saved elsewhere when
	; relevant) but for C calling the func expects them to be lost
	; anyway
	dect	r13
	mov	r6,*r13
	dect	r13
	mov	r7,*r13
	dect	r13
	mov	r8,*r13
	dect	r13
	mov	r9,*r13
	dect	r13
	mov	r10,*r13
	; Save the FP and oddities
	dect	r13
	mov	r12,*r13
	dect	r13
	mov	r14,*r13
	dect	r13
	mov	r15,*r13
	mov	r13,@_udata + U_DATA__U_SP	; Save old SP

	bl	@map_proc_always
	bl	@stash_udata
	bl	@map_kernel

        ; set inint to false
	clr	@_udata + U_DATA__U_ININTERRUPT

        ; find another process to run (may select this one again) returns it
        ; in d
        bl	 @_getproc
	mov	r1,r4
        bl	@_switchin
        ; we should never get here
        bl	@_plt_monitor

stash_udata:
	li	r0,_udata
	li	r1, 0xBE00		; Ick FIXME hardcoded
	li	r2, 256			; 256 words of udata
stash_loop:
	mov	*r0+,*r1+
	dec	r2
	jne	@stash_loop
	rt

badswitchmsg:
	.ascii "_switchin: FAIL"
	.byte 13
	.byte 10
	.byte 0
	.even
;
;	process pointer is in r4
;
_switchin:
	limi	IRQ_OFF
	dect	r13
	mov	r11,*r13

	mov	r4,r8

	mov	@P_TAB__P_PAGE_OFFSET(r8),r0
	c	@_udata+U_DATA__U_PAGE,r0
	jeq	@nostash

	ci	r0,0
	jne	@not_swapped
	bl	@_swapper

not_swapped:
	mov	r8,r0
	bl	@map_proc_x

	; We must do this inline as we will be changing stack under
	; ourselves and without stack
	li	r0,0xBE00
	li	r1,_udata
	li	r2,256		; 256 words

unstash_loop:
	mov	*r0+,*r1+
	dec	r2
	jne	@unstash_loop

	; Now get the stack back sane
	mov	@_udata + U_DATA__U_SP,r13
	bl	@map_kernel
nostash:
        ; check u_data->u_ptab matches what we wanted
	; r8 still holds the proc pointer
	c	@_udata + U_DATA__U_PTAB,r8
	jne	@switchinfail;

	; Load into the byte side
	li	r0, P_RUNNING * 256
	movb	r0,@P_TAB__P_STATUS_OFFSET(r8)

	; fix up our pages as they may have changed
	mov 	@P_TAB__P_PAGE_OFFSET(r8),@_udata + U_DATA__U_PAGE
	mov 	@P_TAB__P_PAGE_OFFSET+2(r8),@_udata + U_DATA__U_PAGE+2

	clr	@_runticks

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        mov @_udata + U_DATA__U_SP,r13
	mov	*r13+,r15
	mov	*r13+,r14
	mov	*r13+,r12
	mov	*r13+,r10
	mov	*r13+,r9
	mov	*r13+,r8
	mov	*r13+,r7
	mov	*r13+,r6
	bl	@restore_userws
	mov	*r13+,r1	; Return code
	mov	*r13+,r11	; return address
	mov	@_udata + U_DATA__U_ININTERRUPT,r0
        jeq	@swtchdone	; in ISR, leave interrupts off
	limi	IRQ_ON
swtchdone:
        rt

switchinfail:
	li	r1, badswitchmsg
        bl	@outstring
	; something went wrong and we didn't switch in what we asked for
        b	@_plt_monitor

;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;	r4 is process pointer on entry
;
_dofork:
;        ; always disconnect the vehicle battery before performing maintenance
	limi	IRQ_OFF

	; Stack the return address

	dect	r13
	mov	r11,*r13

	dect	r13		; PID for return code
	mov	@P_TAB__P_PID_OFFSET(r4),*r13

	; Save it as we need it for the other context (32 bytes)
	bl	@save_userws
	; Save the registers we need (r0-r5 are saved elsewhere when
	; relevant) but for C calling the func expects them to be lost
	; anyway. Must match switchout.
	dect	r13
	mov	r6,*r13
	dect	r13
	mov	r7,*r13
	dect	r13
	mov	r8,*r13
	dect	r13
	mov	r9,*r13
	dect	r13
	mov	r10,*r13
	; Save the FP and oddities
	dect	r13
	mov	r12,*r13
	dect	r13
	mov	r14,*r13
	dect	r13
	mov	r15,*r13

	; And save R13 (our SP) into our udata so we can get it all back
	mov	r13, @_udata + U_DATA__U_SP

	;
	; Stash the udata for the parent switchin path
	;
	bl	@map_proc_always
	bl	@stash_udata
	bl	@map_kernel

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	mov	r4,r15		; save proc ptr in reg we can restore easiest

	;
	;	On entry r15 is the process pointer and r4 is scratch but
	;	still holds the same
	;
	ai	r4, P_TAB__P_PAGE_OFFSET
	li	r3, _udata+U_DATA__U_PAGE

	;	Copy the first bank
	;	TODO: if we do this IRQ enabled in future we need to update
	;	the map pointers too
	;
	;	The order here is critical. r4 is pointing into 0x8000-BFFF
	;	quite probably, so need to set that half first (the other
	;	is in common so good)
	;
	movb	*r4+,@0xFE79
	movb	*r3+,@0xFE7A
	bl	@copyblock
	bl	@map_kernel_di
	;	If the second bank is not the same copy the second bank
	cb	@-1(r3),*r3
	jeq	@copydone
	movb	*r4+,@0xFE79
	movb	*r3+,@0xFE7A
	bl	@copyblock
	bl	@map_kernel_di
	;	If the third bank is not the same copy the third bank
	cb	@-1(r3),*r3
	jeq	@copydone
	movb	*r4,@0xFE79
	movb	*r3,@0xFE7A
	bl	@copyblock
copydone:
	mov	@_udata+U_DATA__U_PTAB,r0
	bl	@map_proc_x
	bl	@stash_udata
	;	And this also puts back our memory map from the effects of
	;	the fork copy
	bl	@map_kernel_di

	li	r5,_udata
	mov	r15,r4
	bl	@_makeproc

	; any calls to map process will now map the childs memory

	mov	*r13, r15		; saved R15 is top
	ai	r13, 50			; remove stack frame (9 words + 16 user context)
        ; runticks = 0;
	clr	r1
	clr	@_runticks

        ; in the child process, fork() returns zero.


	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
	mov	*r13+,r11
        rt
;
;	Copy the page pair mapped at 0x8000 to the page pair mapped at 0x4000
;	The caller owns clean up
;
copyblock:
	li	r0, 0x4000
	li	r1, 0x8000
	mov	r0, r2		; 16K count too
copy:
	mov	*r1+,*r0+
	dect	r2
	jne	@copy

	rt
