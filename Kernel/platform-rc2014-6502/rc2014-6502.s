;
;	    rc2014 6502 platform functions
;

            .export init_early
            .export init_hardware
            .export _program_vectors
	    .export map_kernel
	    .export map_process
	    .export map_process_always
	    .export map_save_kernel
	    .export map_restore

	    .export _unix_syscall_i
	    .export _platform_interrupt_i
	    .export platform_doexec

            ; exported debugging tools
            .export _platform_monitor
	    .export _platform_reboot
            .export outchar
	    .export ___hard_di
	    .export ___hard_ei
	    .export ___hard_irqrestore
	    .export vector
	    .export _sys_stubs

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
	    .import _platform_interrupt
	    .import _kernel_flag
	    .import stash_zp
	    .import pushax
	    .import _chksigs
	    .import _platform_switchout
	    .import _need_resched

	    .import outcharhex
	    .import outxa
	    .import incaxy

	    .import _create_init_common

            .include "kernel.def"
            .include "../kernel02.def"
	    .include "zeropage.inc"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0x0200 upwards after the common data blocks)
; -----------------------------------------------------------------------------
            .segment "COMMONMEM"

_platform_monitor:
_platform_reboot:
	    lda #0
	    sta $FE7B		; top 16K to ROM 0
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
	    ; recycle page 32 into a final process but that means awkward
	    ; handling - or does it - we wrap the bit ?? FIXME
	    jsr _create_init_common
	    lda #36
	    sta $FE78		; set low page to copy
            rts			; stack was copied so this is ok

init_hardware:
            ; set system RAM size for test purposes
	    lda #0
	    sta _ramsize
	    lda #2
	    sta _ramsize+1
	    lda #192
	    sta _procmem
	    lda #1
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
	    jsr map_process
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
;	map_process_always()
;	Map the current process (ie the one with the live uarea)
;
;	map_kernel()
;	Map the kernel
;
;	map_process(pageptr [in X,A])
;	if pageptr = 0 then map_kernel
;	else map_process using the pageptr
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
map_process_always:
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
map_process:
	    cmp #0
	    bne map_process_2
	    cpx #0
	    bne map_process_2
;
;	Map in the kernel below the current common, all registers preserved
;	the kernel lives in 32/33/34/35
;	Later we'll be clever and stuff _DISCARD and the copy blocks there or
;	something (that would also let us put RODATA in
;	common area just to balance out memory usages).
;
map_kernel:
	    pha
	    txa
	    pha
				; Common is left untouched as is ZP and S
	    ldx #$20		; Kernel RAM
	    jsr map_bank_i
	    pla
	    tax
	    pla
	    rts

;
;	Entry point to map a linear bank range. We switch 4000-BFFF
;	C000-FFFF are constant, 0000-3FFF are switched on the task switch
;	so are valid for map_process_always cases but not mapping an
;	arbitrary process. This is ok - when we add swap it uses
;	map_for_swap and that will map a 16K window in and out (which will
;	need us to fix save/restore)
;
map_bank:
	    stx $FE78
map_bank_i:			; We are not mapping the first user page yet
	    stx cur_map
	    inx
	    stx $FE79
	    inx
	    stx $FE7A
	    inx
	    stx $FE7B
	    rts

; X,A holds the map table of this process
map_process_2:
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
	    lda $FEC5
	    and #$20
	    beq outcharw
	    pla
	    sta $FEC0
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
	    jsr _platform_switchout
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

platform_doexec:
;
;	Start address of executable
;
	    stx ptr1+1
	    sta ptr1		; Save execution address in ptr1
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
;	Set up the C stack. FIXME: assumes for now our sp in ZP matches it
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
	    ldx #>PROGLOAD	; For the relocation engine
	    lda #ZPBASE
	    ldy #0
	    jmp (ptr1)		; Enter user application

;
;	Straight jumps no funny banking issues
;
_unix_syscall_i:
	    jmp _unix_syscall
_platform_interrupt_i:
	    jmp _platform_interrupt


;
;	Disk copier (needs to be in common), call with ints off
;	for now
;
;	AX = ptr, length always 512, src and page in globals
;
;	Uses ptr3/4 as 1/2 are reserved for the mappers
;
;	FIXME: map_process_always doesn't map the low 16K for 6502 so we
;	have more work to do here to support swap.
;

	.export _hd_read_data,_hd_write_data,_hd_map

_hd_read_data:
	sta ptr3
	stx ptr3+1		; Save the target

	;
	;	We must flip banks before we play mmu pokery, or it will
	; undo all our work. This means our variables must be commondata
	;
	lda _hd_map
	beq hd_kmap
	jsr map_process_always
hd_kmap:
	ldy #0
	jsr hd_read256
	inc ptr3+1
	jsr hd_read256
	jsr map_kernel
	rts

hd_read256:
	lda $FE10
	sta (ptr3),y
	iny
	bne hd_read256
	rts

_hd_write_data:
	sta ptr3
	stx ptr3+1		; Save the target

	;
	;	We must flip banks before we play mmu pokery, or it will
	; undo all our work. This means our variables must be commondata
	;
	lda _hd_map
	beq hd_kmapw
	jsr map_process_always
hd_kmapw:
	ldy #0
	jsr hd_write256
	inc ptr3+1
	jsr hd_write256
	jsr map_kernel
	rts

hd_write256:
	lda (ptr3),y
	sta $FE10
	iny
	bne hd_write256
	rts

	.segment "COMMONDATA"

_hd_map:
	.res 1
