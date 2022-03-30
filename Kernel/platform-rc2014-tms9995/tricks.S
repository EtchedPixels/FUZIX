;
;	TMS9995 version
;
;	Just to get us going
;
        .export _platform_switchout
        .export _switchin
        .export _dofork
	.export _ramtop

#include "kernel.def"
#include "../kernel-tms9995.def"

	.common

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here
_ramtop:
	.word 0xF000	; FIXME check

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
;
_platform_switchout:
	limi	IRQ_OFF
	; Save the link
	dect	r13
	mov	r11,*r13
	dect	r13
	clr	*r13			; Return code
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

	bl	@map_process_always
	bl	@stash_udata
	bl	@map_kernel

        ; set inint to false
	clr	@_inint

        ; find another process to run (may select this one again) returns it
        ; in d
        bl	 @_getproc
	mov	r1,r4
        bl	@_switchin
        ; we should never get here
        bl	@_platform_monitor

stash_udata:
	li	r0,_udata
	li	r1, 0xBE00		; Ick FIXME hardcoded
	li	r2, 256		; 256 words of udata
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


	mov	@P_TAB__P_PAGE_OFFSET(r3),r0
	c	@_udata+U_DATA__U_PAGE,r0
	jeq	@nostash

	ci	r0,0
	jne	@not_swapped
	bl	@_swapper

	mov	@2(r13),r4		; proc ptr
	mov	@P_TAB__P_PAGE_OFFSET(r3),r0

not_swapped:
	bl	@map_process_x

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
	; r3 still holds the proc pointer
	c	@_udata + U_DATA__U_PTAB,r4
	jne	@switchinfail

	; Load into the byte side
	li	r0, P_RUNNING * 256
	movb	r0,@P_TAB__P_STATUS_OFFSET(r4)

	; fix up our pages as they may have changed
	mov 	@P_TAB__P_PAGE_OFFSET(r4),@_udata + U_DATA__U_PAGE
	mov 	@P_TAB__P_PAGE_OFFSET+2(r4),@_udata + U_DATA__U_PAGE+2

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
	mov	*r13+,r11
	mov	*r13+,r1	; Return code
	mov	@_inint,r0
        jeq	@swtchdone	; in ISR, leave interrupts off
	limi	IRQ_ON
swtchdone:
        rt

switchinfail:
	li	r0, badswitchmsg
        bl	@outstring
	; something went wrong and we didn't switch in what we asked for
        b	@_platform_monitor

;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
_dofork:
;        ; always disconnect the vehicle battery before performing maintenance
	limi	IRQ_OFF

	mov	*r13,r3		; process pointer
	dect	r13
	mov	*r11,r13

	dect	r13		; PID for return code
	mov	@P_TAB__P_PID_OFFSET(r3),*r13

	; Save the registers we need (r0-r7 are saved elsewhere when
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

	mov	r13, @_udata + U_DATA__U_SP

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	mov	r3,r15		; save proc ptr in reg we can restore easiest

	;
	;	Not smart about unused space. Wants fixing ?
	;
	;	On entry r15 is the process pointer and r3 is scratch but
	;	still holds the same
	;
	ai	r3, P_TAB__P_PAGE_OFFSET
	li	r4, _udata+U_DATA__U_PAGE
	li	r5, 3

copyblock:
	movb	*r3+,@0xFE79	; Map at 16K (dest)
	movb	*r4+,@0xFE7A	; Map at 32K (source)

	li	r0, 0x4000
	li	r1, 0x8000
	mov	r0, r2		; 16K count too

copy:
	mov	*r1+,*r0+
	dect	r2
	jne	@copy

	dec	r5
	jne	@copyblock

	mov	@_udata+U_DATA__U_PTAB,r0
	bl	@map_process_x
	bl	@stash_udata
	bl	@map_kernel_di

	li	r5,_udata
	mov	r15,r4
	bl	@_makeproc

	; any calls to map process will now map the childs memory

	ai	r13, 22			; remove stack frame (11 words)
        ; runticks = 0;
	clr	r1
	clr	@_runticks

        ; in the child process, fork() returns zero.


	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
	mov	*r13+,r11
        rt
