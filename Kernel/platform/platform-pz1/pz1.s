;
;	    rc2014 6502 platform functions
;

            .export init_early
            .export init_hardware
            .export _program_vectors
	    .export map_kernel
	    .export map_proc
	    .export map_proc_always
	    .export map_save_kernel
	    .export map_restore

	    .export _unix_syscall_i
	    .export _plt_interrupt_i
	    .export plt_doexec

            ; exported debugging tools
            .export _plt_monitor
	    .export _plt_reboot
            .export outchar
	    .export ___hard_di
	    .export ___hard_ei
	    .export ___hard_irqrestore
	    .export vector
	    .export _sys_stubs

	    .export _hd_data_in
	    .export _hd_data_out
	    .import _td_raw

	    .import interrupt_handler
	    .import _udata
	    .import _ramsize
	    .import _procmem
	    .import nmi_handler
	    .import unix_syscall_entry
	    .import kstack_top
	    .import istack_switched_sp
	    .import istack_top
	    .import _unix_syscall
	    .import _plt_interrupt
	    .import _kernel_flag
	    .import stash_zp
	    .import pushax
	    .import _chksigs
	    .import _plt_switchout
	    .import _need_resched
	    .import _relocator

	    .import outcharhex
	    .import outxa
	    .import incaxy

	    .import _create_init_common

            .include "kernel.def"
            .include "../../cpu-6502/kernel02.def"
	    .include "zeropage.inc"

            .include "ports.def"

RAM_SIZE_KB	.set	512
; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0x0200 upwards after the common data blocks)
; -----------------------------------------------------------------------------
            .segment "COMMONMEM"

_plt_monitor:
_plt_reboot:
	    jmp _plt_reboot	; We don't restore the data/bss so can't
				; restart

	    lda #3
	    sta IO_BANK_3		; top 16K to ROM 0
	    jmp ($FFFC)

___hard_di:
	    php
	    sei			; Save old state in return to C
	    pla			; Old status
	    rts
___hard_ei:
	    cli			; on 6502 cli enables IRQs!!!
	    rts

___hard_irqrestore:
	    and #4		; IRQ flag
	    bne irq_on
	    cli
	    rts
irq_on:
	    sei
	    rts

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .code

init_early:
	    ; Hack for now - create a common copy for init. We should then
	    ; recycle page 0 into a final process but that means awkward
	    ; handling - or does it - we wrap the bit ?? FIXME
	    jsr _create_init_common
	    lda #4
	    sta IO_BANK_0		; set low page to copy
            rts			; stack was copied so this is ok

init_hardware:
            ; set system RAM size for test purposes
	    lda #(RAM_SIZE_KB & $FF)
	    sta _ramsize
	    lda #(RAM_SIZE_KB >> 8)
	    sta _ramsize+1
	    lda #((RAM_SIZE_KB - 64) & $FF)
	    sta _procmem
	    lda #((RAM_SIZE_KB - 64) >> 8)
	    sta _procmem+1
            jmp program_vectors_k

	    ; copied into the stubs of each binary
_sys_stubs:
	    jmp syscall_entry
	    .byte 0
	    .word 0
	    .word 0
	    .word 0
	    .word 0
	    .word 0
	    .word 0

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .segment "COMMONMEM"

_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
	    sei
	    ;
	    ; our C caller will invoke us with the pointer in x,a
	    ; just pass it on
	    jsr map_proc
program_vectors_k:
	    lda #<vector
	    sta $FFFE
	    lda #>vector
	    sta $FFFF
	    lda #<nmi_handler
	    sta $FFFA
	    lda #>nmi_handler
	    sta $FFFB
	    jmp map_kernel

;
;	On a fully switching system with far copies like the 6509 this is
;	all basically a no-op
;
;	On a banked setup the semantics are:
;
;	map_proc_always()
;	Map the current process (ie the one with the live uarea)
;
;	map_kernel()
;	Map the kernel
;
;	map_proc(pageptr [in X,A])
;	if pageptr = 0 then map_kernel
;	else map_proc using the pageptr
;
;	map_save
;	save the current mapping
;
;	map_restore
;	restore the saved mapping
;
;	save/restore are used so that the kernel can play with its internal
;	banking/mappings without having to leave interrupts off all the time
;
;	ptr1 and tmp1 may be destroyed by these methods, but no other
;	temporaries.
;
map_proc_always:
	    pha
	    txa
	    pha
	    ldx _udata + U_DATA__U_PAGE
	    jsr map_bank_i
	    pla
	    tax
	    pla
	    rts
;
;	X,A points to the map table of this process
;
map_proc:
	    cmp #0
	    bne map_proc_2
	    cpx #0
	    bne map_proc_2
;
;	Map in the kernel below the current common, all registers preserved
;	the kernel lives in 0/1/2/3
;	Later we'll be clever and stuff _DISCARD and the copy blocks there or
;	something (that would also let us put RODATA in
;	common area just to balance out memory usages).
;
map_kernel:
	    pha
	    txa
	    pha
				; Common is left untouched as is ZP and S
	    ldx #0		; Kernel RAM
	    jsr map_bank_i
	    pla
	    tax
	    pla
	    rts

;
;	Entry point to map a linear bank range. We switch 4000-FFFF
;	0000-3FFF are switched on the task switch so are valid for
;	map_proc_always cases but not mapping an arbitrary process.
;	This is ok - when we add swap it uses map_for_swap and that will map
;	a 16K window in and out (which will need us to fix save/restore)
;
map_bank:
	    stx IO_BANK_0
map_bank_i:			; We are not mapping the first user page yet
	    stx cur_map
	    inx
	    stx IO_BANK_1
	    inx
	    stx IO_BANK_2
	    inx
	    stx IO_BANK_3
	    rts

; X,A holds the map table of this process
map_proc_2:
	    sta ptr1
	    stx ptr1+1
	    tya
	    pha
	    ldy #0
	    lda (ptr1),y	; 4 bytes if needed
	    tax
	    pla
	    tay
	    jmp map_bank_i


;
;	Restore mapping. This may not be sufficient. We may need to do a
;	careful 4 byte save/restore if we do clever stuff in future. We only
;	ever use this from the current kernel map when returning to kernel
;	so this is fine.
;
map_restore:
	    pha
	    txa
	    pha
	    ldx saved_map
	    jsr map_bank_i
	    pla
	    tax
	    pla
	    rts

;
;	Save the current mapping.
;	May not be sufficient if we want IRQs on while doing page tricks
;
map_save_kernel:
	    pha
	    lda cur_map
	    pha
	    jsr map_kernel
	    pla
	    sta saved_map	; always save the map in the right commonmem
	    pla
	    rts

cur_map:    .byte 0
saved_map:  .byte 0

; outchar: Wait for UART TX idle, then print the char in a without
; corrupting other registers

outchar:
	    pha
outcharw:
	    lda IO_SERIAL_0_FLAGS
            and #SERIAL_FLAGS_OUT_FULL
            bne outcharw
            pla
            sta IO_SERIAL_0_OUT
            rts

;
;	Code that will live in each bank at the same address and is copied
;	there on 6509 type setups. On 6502 it may well be linked once in
;	common space
;
	.segment "COMMONMEM"
;
;	Interrupt vector logic. Keep this in platform for the 6502 so that
;	we can use the shorter one for the CMOS chip
;
vector:
	    pha
	    txa
	    pha
	    tya
	    pha
	    cld

	    ; Save our zero page bits (and thus the C stack etc). The save
	    ; area is task specific so if we task switch all is good
	    jsr stash_zp
;
;	Q: do we want to spot BRK instructions and signal them ?
;
;
;	Save the old stack ptr (do we need this - can it even get
;	unbalanced ? Note that istack_switched_sp like the istack is
;	in the commondata so per context not in fact as it looks global.
;	Pre-emption relies on this.
;
	    tsx					; and save the 6502 stack ptr
	    stx istack_switched_sp		; in uarea/stacks

;
;	Configure the C stack to the i stack
;
	    lda #<istack_top
	    sta sp
	    lda #>istack_top
	    sta sp+1
;
;	This will map in the upper kernel pages via map_save_kernel, invoke
;	the handler and then return restoring the mapped pages of either
;	the kernel or calling user process to the high space.
;
	    jsr interrupt_handler

;	Check for kernel return path

	    lda _kernel_flag
	    bne kirqout
;
;	Pre-emption ?
;
	    lda _need_resched
	    beq no_preempt

	    ;
	    ; Put this task to sleep and continue with the next one. As we
	    ; have per task IRQ stacks out of necessity (the 6502 stack is
	    ; a fixed page) this isn't as scary as on some ports.
	    ;

	    lda	#0
	    sta _need_resched

	    ;
	    ; We need to keep on the kernel istack at this point because the
	    ; user C stack may not be self-consistent.
	    ;
	    lda	#1
	    sta _udata+U_DATA__U_INSYS
	    jsr	_chksigs
	    ; Mark ourselves Ready not Running
	    ldx _udata+U_DATA__U_PTAB
	    lda	#P_READY
	    sta a:P_TAB__P_STATUS_OFFSET,x
	    ; Switch out .. we will re-appear at some point
	    lda	_udata+U_DATA__U_PTAB
	    ldx _udata+U_DATA__U_PTAB+1
	    ; When we enter this code we will switch the low 16K underneath
	    ; us. Other stuff will run on a different low bank, and at some
	    ; point we will return with S correct and on our 6502 and C
	    ; stacks (we effectively have private IRQ stacks)
	    jsr _plt_switchout
	    ;
	    ; Now return to the old state having woken back up
	    ; Clear insys, move back onto the interrupt stack
	    ;
	    lda #0
	    sta _udata+U_DATA__U_INSYS

	    ;
	    ; Check what signals are now waiting for us
	    ;
no_preempt:
;
;	Signal handling on 6502 is foul as the C stack may be inconsistent
;	during an IRQ. We push a new complete rti frame below the official
;	one, along with a vector and the signal number. The glue in the
;	app is expected to switch to a signal stack or similar, pop the
;	values, invoke the signal handler and then return.
;
;
;	Get back on the user stack, but do not use the user C stack
;	between here and returning to user space
;
	    ldx istack_switched_sp
	    txs					; recover 6502 stack
	    jsr stash_zp			; recover 6502 zp bits
	    lda _udata+U_DATA__U_CURSIG
	    beq irqout
	    tay

	    ; Right now the stack holds Y X A P rti addr
	    ; Push a helper to clean up and restore the register state
	    lda #>(irqout-1)
	    pha
	    lda #<(irqout-1)
	    pha				; stack a return vector
	    tya
	    pha				; stack signal number
	    ldx #0
	    stx _udata+U_DATA__U_CURSIG
	    asl a
	    tay
	    lda _udata+U_DATA__U_SIGVEC+1,y	; Our vector (low)
	    pha				; stack half of vector
	    lda _udata+U_DATA__U_SIGVEC,y	; High half
	    pha				; stack rest of vector
	    txa
	    sta _udata+U_DATA__U_SIGVEC,y	; Wipe the vector
	    sta _udata+U_DATA__U_SIGVEC+1,y
	    lda #>(PROGLOAD + 16)
	    pha
	    lda #<(PROGLOAD + 16)
	    pha
	    lda #0
	    pha				; dummy flags, with irq enable
	    rti	    			; return on the fake frame
					; if the handler returns
					; rather than doing a longjmp
					; we'll end up at irqout and pop the
					; real frame
kirqout:
;
;	Get back on the user stack, but do not use the user C stack
;	until we leave
;
	    ldx istack_switched_sp
	    txs					; recover 6502 stack
	    jsr stash_zp			; recover 6502 zp bits

irqout:
	    pla
	    tay
	    pla
	    tax
	    pla
	    rti
;
;	    sp/sp+1 are the C stack of the userspace
;	    with the syscall number in X
;	    Y indicates the number of bytes of argument
;
syscall_entry:
	    php
	    sei
	    cld

	    stx _udata+U_DATA__U_CALLNO

	    ; No arguments - skip all the copying and stack bits
	    cpy #0
	    beq noargs

	    ; Remove the arguments. This is fine as by the time we go back
	    ; to the user stack we'll have finished with them
	    lda sp
	    sta ptr1
	    ldx sp+1
	    stx ptr1+1
	    jsr incaxy
	    sta sp
	    stx sp+1

	    ;
	    ;	We copy the arguments but need to deal with the compiler
	    ;   stacking in the reverse order. At this point ptr1 points
	    ;	to the last byte of the arguments (first argument). We go
	    ;	down the stack copying words up the argument list.
	    ;
	    ldx #0
copy_args:
	    dey
	    lda (ptr1),y		; copy the arguments over
	    sta _udata+U_DATA__U_ARGN+1,x
	    dey
	    lda (ptr1),y
	    sta _udata+U_DATA__U_ARGN,x
            inx
	    inx
	    cpy #0
	    bne copy_args
noargs:
	    ;
	    ; Now we need to stack switch. Save the adjusted stack we want
	    ; for return
	    ;
	    lda sp
	    pha
	    lda sp+1
	    pha
	    tsx
	    stx _udata+U_DATA__U_SYSCALL_SP
;
;	We save a copy of the high byte of sp here as we may need it to get
;	the brk() syscall right.
;
	    sta _udata+U_DATA__U_SYSCALL_SP + 1
;
;	FIXME: we should check here if there is enough 6502 stack left
;	and if so either copy and switch stacks or kill the process
;
;	Set up the C stack
;
	    lda #<kstack_top
	    sta sp
	    lda #>kstack_top
	    sta sp+1

;
;	Caution: We may enter here and context switch and another task
;	exit via its own syscall returning in its own memory context.
;
;	Don't assume anything we stored statically *except* the uarea
;	will be different. The uarea is banked in and out (or copied in
;	more awkward systems).
;
	    jsr unix_syscall_entry

;
;	Correct the system stack
;
	    ldx _udata+U_DATA__U_SYSCALL_SP
	    txs

;
;	From that recover the C stack and the syscall buf ptr
;
	    pla
	    sta sp+1
	    pla
	    sta sp
	    lda _udata+U_DATA__U_CURSIG
	    beq syscout
	    tay

	    ;
	    ;	The signal handler might make syscalls so we need to get
	    ;	our return saved and return the right value!
	    ;
	    lda _udata+U_DATA__U_ERROR
	    pha
	    lda _udata+U_DATA__U_RETVAL
	    pha
	    lda _udata+U_DATA__U_RETVAL+1
	    pha
	    lda #>(sigret-1)		; Return address
	    pha
	    lda #<(sigret-1)
	    pha

	    tya
	    pha				; signal
	    ldx #0
	    stx _udata+U_DATA__U_CURSIG
	    asl a
	    tay
	    lda _udata+U_DATA__U_SIGVEC+1,y	; Our vector
	    pha
	    lda _udata+U_DATA__U_SIGVEC,y
	    pha
	    txa
	    sta _udata+U_DATA__U_SIGVEC,y	; Wipe the vector
	    sta _udata+U_DATA__U_SIGVEC+1,y

	    ; Invoke the helper with signal and vector stacked
	    ; it will then return to syscout and recover the original
	    ; frame. If the handler made syscalls then we set the registers
	    ; up in sigret
	    cli
	    jmp (PROGLOAD + 16)

sigret:
	    pla		; Unstack the syscall return pieces
	    tax
	    pla
	    tay
	    pla
	    plp		; from original stack frame
	    rts

syscout:
;	We may be in decimal mode beyond this line.. take care
;
	    plp

;	Copy the return data over
;
	    ldy _udata+U_DATA__U_RETVAL
	    ldx _udata+U_DATA__U_RETVAL+1
;	Also sets Z for us
	    lda _udata+U_DATA__U_ERROR

	    rts

plt_doexec:
;
;	Start address of executable
;
	    stx ptr3+1
	    sta ptr3		; Save execution address in ptr3
	    stx ptr2+1		; Point ptr2 at base page + 16
	    lda #16
	    sta ptr2
	    ldy #0
	    lda (ptr2),y	; Get the signal vector pointer
	    sta PROGLOAD+16	; if we loaded high put the vector in
	    iny
	    lda (ptr2),y
	    sta PROGLOAD+17	; the low space where it is expected

;
;	Set up the C stack. Assumes our sp in ZP matches it
;
	    lda _udata+U_DATA__U_ISP
	    sta sp
	    ldx _udata+U_DATA__U_ISP+1
            stx sp+1

;
;	Set up the 6502 stack
;
	    ldx #$ff
	    txs
;
;	Relocation is done in kernel because the 6502 isn't quite smart
;	enough to do its own ZP relocations as far as I can tell (prove me
;	wrong....)
;
	    jsr _relocator
	    jmp (ptr3)		; Enter user application

;
;	Straight jumps no funny banking issues
;
_unix_syscall_i:
	    jmp _unix_syscall
_plt_interrupt_i:
	    jmp _plt_interrupt


;
;	Hard disk loops. In common memory because we do most I/O direct
;	to user space, and in asm for speed
;
;	TODO: bank switching support, swap.
;
;	On entry XA is the pointer in the memory space for the transfer,
;	and hd_mode indicates whether we are transferring to kernel, user
;	(or in future doing a swap transfer).
;
;	Note: hd_mode is in non common space. We can't read it once we've
;	switched.
;
_hd_data_in:
	sta ptr1
	stx ptr1+1
	lda _td_raw
	beq no_remapr
	jsr map_proc_always
no_remapr:
	ldy #0
hdin1:
	lda IO_DISK_DATA
	sta (ptr1),y
	lda IO_DISK_STATUS
	bne hdin_end		;status NOK, end early
	iny
	bne hdin1
	inc ptr1+1
hdin2:
	lda IO_DISK_DATA
	sta (ptr1),y
	lda IO_DISK_STATUS
	bne hdin_end		;status NOK, end early
	iny
	bne hdin2
	; This preserves A
hdin_end:
	jmp map_kernel

_hd_data_out:
	sta ptr1
	stx ptr1+1
	lda _td_raw
	beq no_remapw
	jsr map_proc_always
no_remapw:
	ldy #0
hdout1:
	lda (ptr1),y
	sta IO_DISK_DATA
	lda IO_DISK_STATUS
	bne hdout_end		;status NOK, end early
	iny
	bne hdout1
	inc ptr1+1
hdout2:
	lda (ptr1),y
	sta IO_DISK_DATA
	lda IO_DISK_STATUS
	bne hdout_end		;status NOK, end early
	iny
	bne hdout2
	lda IO_DISK_STATUS
hdout_end:
	jmp map_kernel
