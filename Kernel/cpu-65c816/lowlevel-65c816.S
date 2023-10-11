	.65c816
	.i16
	.a16
	
	.export _doexec
	.export unix_syscall_handler
	.export interrupt_handler
	.export nmi_handler
	.export _brk_limit

	.export illegal_inst
	.export trap_inst
	.export abort_inst
	.export emulation

	.export outstring
	.export outnewline
	.export outcharhex

	.export _sys_cpu
	.export _sys_cpu_feat
	.export _set_cpu_type
	.export _sys_stubs

	.export ___hard_di
	.export ___hard_ei
	.export ___hard_irqrestore

	.export _need_resched

#include "../build/kernel.def"
#include "kernel816.def"

	.code

; Will need to address 8bit binaries at some point...

_sys_cpu:
	.byte 3
_sys_cpu_feat:
	.byte 0x83

_set_cpu_type:
	rts

___hard_di:
	php
	pla
	sei
	rts

___hard_ei:
	php
	pla
	cli
	rts

___hard_irqrestore:
	lda 0,y
	pha
	plp
	iny
	iny
	rts
;
;	Custom brk() limit as U_DATA__U_SYSCLAL_SP is not our C stack
;
_brk_limit:
	ldx	U_DATA__U_SYSCALL_SP
	lda	1,x		; SP is next byte to use
	clc
	sbc	#512
	rts
;
;	We enter via BRK and are in the user map
;
unix_syscall_handler:
	; We can trash registers here fairly freely except Y which is the
	; user stack (and thus argument ptr)
	plx	; get return address byte
	phx
	dex
	sep	#0x20
	.a8
	lda	0,x		; low byte is the syscall, high undefined
	sta	U_DATA__U_CALLNO
	rep	#0x20
	.a16
	
	lda	0,y
	sta	U_DATA__U_ARGN
	lda	2,y
	sta	U_DATA__U_ARGN+2
	lda	4,y
	sta	U_DATA__U_ARGN+4
	lda	6,y
	sta	U_DATA__U_ARGN+6

	sep	#0x20
	.a8
	lda	#1
	sta	U_DATA__U_INSYS
	.a16
	;	We keep the user stack ptr on the CPU stack
	;	and the cpu stack pointer in the udata. Needs special
	;	handling for brk() as with 6502
	phy
	tsx
	stx	U_DATA__U_SYSCALL_SP

	; We should probably flip stack here and have a cpu stack for user
	; for kernel (in the user map ?) and interrupt

	ldy	#kstack_top
	jsr	map_kernel_di
	cli
	jsr	_unix_syscall
	sei

	jsr	map_proc_always
	sep	#0x20
	.a8
	stz	U_DATA__U_INSYS
	rep	#0x20
	.a16
	ldx	U_DATA__U_SYSCALL_SP
	txs
	ply
	lda	U_DATA__U_CURSIG
	bne	via_signal
	ldx	U_DATA__U_ERROR
	lda	U_DATA__U_RETVAL
	rti	

via_signal:
	lda	U_DATA__U_ERROR
	pha
	lda	U_DATA__U_RETVAL
	pha
	jsr	deliver_signal
	pla
	plx
	rti

deliver_signal:
	lda	U_DATA__U_CURSIG
	beq	sigdone
	dey
	dey
	sta	0,y
	asl	a
	tax
	lda	U_DATA__U_SIGVEC,x
	stz	U_DATA__U_SIGVEC,x
	beq	sigdone_y
	ldx	PROGLOAD+16
	cli
	jsr	(0,x)		; called fn will remove arg from user
	sei
	ldy	#kstack_top
	jsr	map_kernel_di
	jsr	_chksigs
	jsr	map_proc_always_di
	ldx	U_DATA__U_SYSCALL_SP
	txs
	ply
	bra	deliver_signal
sigdone_y:
	iny
	iny
sigdone:
	rts

_doexec:
	sei
	ldx	0,y	; execution address
	jsr	map_proc_always
	ldy	U_DATA__U_ISP
	txa
	ldx	#USER_SP
	txs
	tax
	sep	#0x20
	.a8
	stz	U_DATA__U_INSYS
	rep	#0x20
	.a16
	; This is fine as we are always in bank 0
	cli
	jmp	(0,x)
	
reinterrupt:
	rep	#0x20
	.a16
	jsr	_plt_reinterrupt
	bra	int_done

interrupt_handler:
	; Make sure we save all of the register bits
	rep	#$30
	.a16
	.i16
	pha
	phx
	phy
	cld			; no funnies with decimal

	; Now switch our data and DP to the kernel ones
	lda	#IRQ_DP		; interrupt has a private direct page
	tcd

	jsr	plt_interrupt_all

	rep	#0x20
	.a8
	lda	U_DATA__U_ININTERRUPT
	bne	reinterrupt
	inc	U_DATA__U_ININTERRUPT
	; do addr 0 check ??
	sep	#0x20
	.a16
	phy
	tsx
	stx	istack_switched_sp
	ldy	#istack_top	
	ldx	#IRQ_SP
	txs
	jsr	map_save_kernel
	jsr	_plt_interrupt
	rep	#0x20
	.a8
	lda	_need_resched
	bne	preemption
	sep	#0x20
	.a16
	jsr	map_restore
	ldx	istack_switched_sp
	txs
	ply
	rep	#0x20
	.a8
	stz	U_DATA__U_ININTERRUPT
	lda	U_DATA__U_INSYS
	sep	#0x20
	.a16
	bne	int_done
int_out:
	jsr	deliver_signal
int_done:
	lda	#KERNEL_DP	; ?? sep kernel/user ??
	tcd
	ply
	plx
	pla
	rti
	; Pre-emtpion is effectively faking a null syscall that
	; suspends/resumes
preemption:
	stz	_need_resched
	sep	#0x20
	inc	U_DATA__U_INSYS
	.a16
	lda	istack_switched_sp
	sta	U_DATA__U_SYSCALL_SP
	ldy	#kstack_top
	ldx	#kstack_cpu_top
	txs
	jsr	_chksigs
	ldx	U_DATA__U_PTAB
	sep	#0x20
	.a8
	lda	#P_RUNNING
	cmp	0,x
	bne	not_running
	lda	#P_READY
	sta	0,x
	lda	#PFL_BATCH
	ora	1,x
	sta	1,x
not_running:
	rep	#0x20
	.a16
	jsr	_plt_switchout
	sep	#0x20
	.a8
	stz	U_DATA__U_ININTERRUPT
	stz	U_DATA__U_INSYS
	rep	#0x20
	.a16
	jsr	map_proc_always_di
	ldx	U_DATA__U_SYSCALL_SP
	txs
	ply
	bra	int_out
;
;	Trap handlers
;
illegal_inst:
	rep	#$30
	.a16
	.i16
	pha
	phx
	cld
	lda	#4
sync_sig:
	pha
	sep	#0x20
	.a8
	lda	_kernel_flag
	bne	ktrap
	lda	U_DATA__U_ININTERRUPT
	bne	itrap
	rep	#0x20
	.a16
	; tos is the signal
	pla
	pha
	dey
	dey
	sta	0,y
	asl	a
	tax
	lda	U_DATA__U_SIGVEC,x
	stz	U_DATA__U_SIGVEC,x
	beq	trap_kill
	cli
	jsr	(0,x)
	; cleans up stack itself
	sei
	; If we get back here then it's probably curtains
	; but loop in case they reset the handler again
	pla
	bra sync_sig
trap_kill:
	pla
	swa
	dey
	dey
	sta	0,y
	jsr	_doexit
dead:	; Can't happen
	ldx	#exitfail
	jsr	outstring
	jmp	_plt_monitor	
exitfail:
	.ascii	"exitc"
	.byte	0
			

itrap:
	rep	#0x20
	.a16
	ldx	itrap_msg
outfail:
	jsr	outstring
	jmp	_plt_monitor
itrap_msg:
	.ascii	"itrap!"
	.byte 0

ktrap:
	rep	#0x20
	.a16
	ldx	#ktrap_msg
	bra	outfail
ktrap_msg:
	.ascii	"ktrap!"
	.byte	0

trap_inst:
	rep	#$30
	.a16
	.i16
	pha
	phx
	cld
	lda	#5
	bra	sync_sig

abort_inst:
	rep	#$30
	.a16
	.i16
	pha
	phx
	cld
	lda	#7
	bra	sync_sig

nmi_handler:
	rep	#$30
	ldx	#nmi_trap
	jsr	outstring
nmi_stop:
	jmp	_plt_monitor
nmi_trap:
	.ascii	"NMI!"
	.byte	0

emulation:
	rep	#0x30
	ldx	#emu_trap
	jsr	outstring
	jmp	_plt_monitor
emu_trap:
	.ascii	"EM!"
	.byte	0

outstring:
	sep	#0x20
	.a8
outstringl:
	lda	0,x
	beq	outdone1
	jsr	outchar		; outchar is called a8
	inx
	bra	outstringl

	.a16
outnewline:
	sep	#0x20
	.a8
	pha
	lda #10
	jsr outchar
	lda #10
	jsr outchar
	pla
outdone1:
	rep	#0x20
	.a16
	rts

outcharhex:
	sep #0x20
	.a8
	pha
	and #$f0
	lsr a
	lsr a
	lsr a
	lsr a
	cmp #10
	bcc deci1
	clc
	adc #7
deci1:
	clc
	adc #48			; ascii zero
	jsr outchar
	pla
	and #$0f
	cmp #10
	bcc deci2
	clc
	adc #7
deci2:
	clc
	adc #48
	jsr outchar
	rep #0x20
	.a16
	rts

_need_resched:
	.byte 0


;
;	The stubs is the same as the vector so use it for both
;	until the vector goes away.
;
_sys_stubs:
;	jmp	syscall_entry_8bit	; TODO i8/a8 entry point

	.byte 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
