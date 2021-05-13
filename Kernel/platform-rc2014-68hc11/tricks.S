;
;	68HC11 version
;
;	We do not save the temporaries. The direct page is tied to each
;	map so the direct page working registers are also per process. Only
;	the interrupt path needs to save and restore them.
;
;	The only ugly we have is that because we are using the 16Kx4 memory
;	banking model and our code density isn't quite good enough we need
;	to switch udata objects on task switches. With the 56K/8K MMU we
;	could avoid this!
;
;	We don't save a lot of stuff. The compiler assumes D X and Y will
;	be changed by the called function anyway. The soft register state
;	lives in DP so is switched with the task switch and copied by fork
;
        .globl platform_switchout
        .globl switchin
        .globl dofork
	.globl ramtop

#include "kernel.def"
#include "../kernel-hc11.def"

	.section .common

; ramtop must be in common for single process swapping cases
; and its a constant for the others from before init forks so it'll be fine
; here
ramtop:
	.word 0xBE00

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
platform_switchout:
	sei
	clra
	psha
	psha				; Return code
	sts udata + U_DATA__U_SP	; Save old SP

	jsr map_process_always
	jsr stash_udata
	jsr map_kernel

        ; set inint to false
	clr inint

        ; find another process to run (may select this one again) returns it
        ; in d
        jsr getproc
        jsr switchin
        ; we should never get here
        jsr platform_monitor

stash_udata:
	ldx #udata
	ldy #0xBE00		; Ick FIXME hardcoded
stash_loop:
	ldd ,x
	inx
	inx
	std ,y
	iny
	iny
	cpx #udata+512
	bne stash_loop
	rts

badswitchmsg:
	.ascii "_switchin: FAIL"
	.byte 13
	.byte 10
	.byte 0

;
;	On entry D holds the process to switch in
;
switchin:
	sei
	std	switch_proc_ptr
	xgdx

	; FIXME: sort out offset rules
	ldd	P_TAB__P_PAGE_OFFSET,x
	cmpb	udata+U_DATA__U_PAGE+1
	beq nostash

	subd	#0		;compare with 0
	bne	not_swapped
	jsr	swapper
	ldx	switch_proc_ptr
	ldab	P_TAB__P_PAGE_OFFSET+1,x

not_swapped:
	jsr	map_process_x

	; We must do this inline as we will be changing stack under
	; ourselves and without stack
	ldx	#0xBE00
	ldy	#udata
unstash_loop:
	ldd	,x
	inx
	inx
	std	,y
	iny
	iny
	cpx	#0xC000		; BE000 + 0200
	bne	unstash_loop

	; Now get the stack back sane
	lds	udata + U_DATA__U_SP
	jsr	map_kernel
nostash:
        ; check u_data->u_ptab matches what we wanted
	ldx	udata + U_DATA__U_PTAB
	cpx	switch_proc_ptr
	bne	switchinfail

	ldab	#P_RUNNING
	ldx	switch_proc_ptr
	stab	P_TAB__P_STATUS_OFFSET,x

	; fix up our pages as they may have changed
	ldd P_TAB__P_PAGE_OFFSET,x
	std udata + U_DATA__U_PAGE
	ldaa P_TAB__P_PAGE_OFFSET+2,x
	staa udata + U_DATA__U_PAGE+2

	clra
	clrb
	std runticks

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        lds udata + U_DATA__U_SP
	ldaa inint
        beq swtchdone		; in ISR, leave interrupts off
	cli
swtchdone:
	pula			; recover return code
	pulb
        rts

switchinfail:
	ldx #badswitchmsg
        jsr outstring
	; something went wrong and we didn't switch in what we asked for
        jmp platform_monitor

; Must not put this in ZP ?
;
; Move to commondata ??
;
fork_proc_ptr: .word 0 ; (C type is struct p_tab *) -- address of child process p_tab entry

;
;	Called from _fork. We are in a syscall, the uarea is live as the
;	parent uarea. The kernel is the mapped object.
;
dofork:
;        ; always disconnect the vehicle battery before performing maintenance
        sei	 ; should already be the case ... belt and braces.

	; new process in D, get parent pid

	std fork_proc_ptr
	xgdx

	ldx P_TAB__P_PID_OFFSET,x
	pshx				; Push the PID

	sts udata + U_DATA__U_SP

        ; now we're in a safe state for _switchin to return in the parent
	; process.

	;
	;	Assumes ptr1 still holds the new process ptr
	;

	jsr fork_copy

        ; now the copy operation is complete we can get rid of the stuff
	; _switchin will be expecting from our copy of the stack.
	pulx

	ldx #udata
	pshx
	ldd fork_proc_ptr
        jsr makeproc
	pulx

	; any calls to map process will now map the childs memory

        ; runticks = 0;
	clra
	clrb
	std runticks

        ; in the child process, fork() returns zero.

	; And we exit, with the kernel mapped, the child now being deemed
	; to be the live uarea. The parent is frozen in time and space as
	; if it had done a switchout().
        rts

;
;	On entry ptr1 points to the process table of the child, and
;	the U_DATA is still not fully modified so holds the parents bank
;	number. This wants optimising to avoid copying all the unused
;	space!
;
;	Copy memory
;
#if 0
fork_copy:
	ldx fork_proc_ptr
	ldaa P_TAB__P_PAGE_OFFSET,x
	ldab udata+U_DATA__U_PAGE
	bsr bank2bank
	inx
	ldaa P_TAB__P_PAGE_OFFSET,x
	ldab udata+U_DATA__U_PAGE+1
	bsr bank2bank
	inx
	ldaa P_TAB__P_PAGE_OFFSET,x
	ldab udata+U_DATA__U_PAGE+2
	bsr bank2bank

	ldx udata+U_DATA__U_PTAB
	jsr map_process_x
	jsr stash_udata
	jmp map_kernel_di
#endif
;
;	Copier until we switch to proper 16K banked
;
fork_copy:
	ldx fork_proc_ptr
	ldaa P_TAB__P_PAGE_OFFSET+1,x
	ldab udata+U_DATA__U_PAGE+1
	psha
	pshb
	jsr bank2bank
	pulb
	pula
	inca
	incb
	psha
	pshb
	jsr bank2bank
	pulb
	pula
	inca
	incb
	jsr bank2bank
	ldx udata+U_DATA__U_PTAB
	jsr map_process_x
	jsr stash_udata
	jmp map_kernel_di

;
;	Copy 16K from page A to page B. Caller is responsible for putting
;	banks back sane. This is akin to the 6803 code except that we have
;	the y register we can use to slightly speed up the copying.
;
bank2bank:
	pshx
	stab 0xFE79		; Switch to source bank
	staa patch1+1		; Patch the destination bank in
	staa 0xFE79		; Switch to destination bank
	stab patch2+1		; And patch the source bank in
	stab 0xFE79		; Now switch to source bank
	ldx #0x4000		; Start address
copyloop:
	ldy ,x
patch1:
	ldaa #0
	staa 0xFE79		; Switch bank to secondary
	sty ,x
	inx
	inx
patch2:
	ldaa #0
	staa 0xFE79		; And back
	cpx #0x8000		; End at 0x8000
	bne copyloop
	pulx
	rts

;
;	These need to live in common space as we have user mapped some
;	of the time we access them. Ideally we'd move these to a special
;	kernel DP area in common iram: TODO
;
work:	.word	0
work2:	.word	0
switch_proc_ptr: .word 0
