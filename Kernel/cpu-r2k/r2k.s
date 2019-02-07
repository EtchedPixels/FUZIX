;
;	This implements bank switching for the R2K
;
;	At the moment to get it going we keep it in 64K chunks but the
;	move to smarter layouts is possible
;
;	Kernel
;	0000-0FFF	Common page of app 64K bank	[data]
;	1000-DFFF	Kernel space			[stack]
;	E000-FFFF	Rest of kernel	(try to pack code high so xpc is
;			code only for easy externa buffering) [xpc]
;
;	User
;	0000-DFFF	Common page and then app	[stack]
;	E000-FFFF	Rest of app stack etc		[xpc]
;
;	This will hopefully turn into
;
;	0000-brk	Common page and then app	[data]
;	brk-stackbot	undefined (leakage from data)
;	stackbot-DFFF	Stack continued	(contiguous]	[stack]
;	E000-FFFF	Stack				[xpc]
;
;
;	If we add split I/D it's a bit uglier because we can only
;	split data and root so we end up with code restricted between
;	1000 - stackbot (1000 as we can put data at 0000-1FFF in the split
;	I/D but we need the common code to be 0000-0FFF. Also need to watch
;	IRQ handlers and low memory buffers - will need to use xpc maybe ?
;
;	Note we can't (annoyingly) do XIP because our ROM code would have to
;	be aligned to suit our RAM code. Well except with 64K banks and only
;	128K of RAM. Less than useful.
;
;
		.module r2k

		.area _ COMMONMEM

;
;	Called with A = number of 64K banks
;	If we begin ROMming things then we'll need to think harder
;
;	Expects the kernel to be in the bottom of RAM in the bottom of the
;	address space
;
r2k_init_early:
		ld h,a		;	* 256
		ld l,#0
		rr hl
		rr hl		;	* 64
		ld (_ramsize),hl
		ld de,#64
		or a
		sbc hl,de
		ld (_procmem),hl
;
;	For simple 64K banks
;
		ld a,#USER_COMMONBASE
		ld b,a			; banks
		dec b			; Kernel
;
;	FIXME: fill in the common before duplicating
;
make_common:
		ld xpc,a
		push bc
		ld hl,#0x0000
		ld de,#0xE000
		ld bc,#0x1000
ldil0:
		ldi
		jr nz,ldil0
		pop bc
		add a,#16		; move on 64K
		djnz make_common
		ret

		.area _DISCARD

r2k_init_hardware:
		ret

		.area _CODE
;
;	We do this at load so we can avoid IRQ masking
;
_program_vectors:
		ret

;
;	Called from _fork. We can standardize this for the R2K
;
_dofork:
		ipset 1
		ld hl,(sp + 2)

	        ld (fork_proc_ptr), hl

		; Get pid
		ld hl,(hl + P_TAB__P_PID_OFFSET)

	        ; prepare return value in parent process -- HL = p->p_pid;

	        ; Save the stack pointer and critical registers.
	        ; When this process (the parent) is switched back in, it will
		; be as if it returns with the value of the child's pid.
	        push hl ; HL still has p->p_pid from above, the return 
	        push ix ; value in the parent
	        push iy

	        ; save kernel stack pointer -- when it comes back in the parent
		; we'll be in _switchin which will immediately return
		; (appearing to be _dofork() returning) and with HL (ie return
		; code) containing the child PID. Hooray.

	        ld (U_DATA__U_SP), sp

	        ; now we're in a safe state for _switchin to return in the
		; parent process.

	        ; --------- copy process ---------
	        ld ix, (fork_proc_ptr)

		;
		; We will enter this on the parent stack, copy it to the
		; child stack and return on the child's copy with the new
		; common
		;
        	call copy_and_map_process

	        ; now the copy operation is complete we can get rid of the stuff
	        ; _switchin will be expecting from our copy of the stack.
	        pop bc
	        pop ix		; 	we do need to keep ix safe
	        pop bc

	        ; Make a new process table entry, etc.
	        ld hl, (fork_proc_ptr)
	        push hl
	        call _newproc
	        pop bc 

	        ; runticks = 0;
		bool hl
		rr hl			; hl = 0
	        ld (_runticks), hl
	        ; in the child process, fork() returns zero.
	        ;
	        ; And we exit, with the kernel mapped, the child now being deemed
	        ; to be the live uarea. The parent is frozen in time and space as
	        ; if it had done a switchout().
        	ret

_platform_switchout:
		ipset 1
	        ; save machine state
		bool hl
		rr hl	; hl -> 0 , return code for non fork cases
	        ; U_DATA__U_SP with the following on the stack:
	        push hl ; return code
	        push ix
	        push iy
	        ld (U_DATA__U_SP), sp ; this is where the SP is restored
				      ;in _switchin

	        ; find another process to run (may select this one again)
	        call _getproc

	        push hl
	        call _switchin

	        ; we should never get here
	        jp _platform_monitor

badswitchmsg:
		.ascii "_switchin: FAIL"
        	.db 13, 10, 0
swapped: .ascii "_switchin: SWAPPED"
		.db 13, 10, 0


_switchin:
		; We really do have to block interrupts hard here
		; while we have an invalid stack
		ipset 3
		ld hl,(sp + 2)
		ld de,hl
		ld hl,(hl + P_TAB__P_PAGE_OFFSET)
		; l is now our target common
		;; FIXME if l == 0 need to swap when add swap support
		call map_process_a
		; A is preserved so we have the value for the common
		; switch
		ioio ld (DATASEG), a
		ld sp,(U_DATA__U_SP)
		ipres
		; Check we found the right things in our new common
		ld hl,(U_DATA__U_PTAB)
		or a
		sbc hl,de
		jr nz, switchinfail
		;
		; Set up ready to go
		;
		ld ix, (U_DATA__U_PTAB)
		ld P_TAB__P_STATUS_OFFSET(ix),#P_RUNNING
		; Update paging data if swapped
		ld a, P_TAB__P_PAGE_OFFSET(ix)
		ld (U_DATA__U_PAGE),a
		; clear runticks
		bool hl
		rr hl
		ld (_runticks), hl

		pop iy
		pop ix
		pop hl

		; Check if we need to return with interrupts on or off
		ld a,(U_DATA__U_ININTERRPT)
		or a
		ret nz
		ipset 0
		ret
switchinfail:
	        ; something went wrong and we didn't switch in what we asked for
        	call outhl
	        ld hl, #badswitchmsg
	        call outstring
	        jp _platform_monitor

		.area _COMMONMEM
;
;	Our default kernel map is
;
;	0000-0FFF	Common, udata, stacks [data]
;	1000-DFFF	Kernel code/data [stack]
;	E000-FFFF	Kernel code/data [xpc]
;
;	Some operations we modify the data size from 0 to create
;	a window at 1000 for block copies
;

map_kernel:
		push hl
		ld hl, #KERNEL_SEGS
		ioi ld (STACKSEG), l
		nop
		ioi ld (DATASEG), h
		nop
		ld l, #KERNEL_SEGSIZE
		ioi ld (SEGSIZE), l
		ld xpc, #KERNEL_XPC
		pop hl
		ret

;
;	For user processes the map is set up as
;	0000-0FFF	common (root)			[data]
;	1000-DFFF	data (14 x 4K)			[stack]
;	E000-FFFF	xpc (aligned with above)	[root]
;
;	We may change this in future so that we can reduce memory
;	consumption by using stack/data to split memory and allow brk()
;	to be efficient.
;

map_process:
		ld a,h
		or l
		ld a,(hl)
		jr z, map_kernel
		push af
		jr map_process_a
map_process_always:
		push af
		ld a, (U_DATA__U_PAGE)
map_process_a:
		inc a			; skip common
		ioi ld (STACKSEG),a
		nop
		add a, #13		; align the xpc
		ld xpc,a
		ld a, #USER_SEGSIZE
		ioi ld (SEGSIZE), a
		nop
		pop af
		ret
;
;	Save our map
;
map_save:
		push af
		ld (save_xpc), xpc
		ioi ld a, (STACKSEG)
		nop
		ld (save_stack), a
		ioi ld a, (DATASEG)
		nop
		ld (save_data), a
		ioi ld a, (SEGSIZE)
		nop
		ld (save_size), a
		pop af
		ret
;
;	Load our map
;
map_restore:
		push af
		ld xpc, (save_xpc)
		ld a, (save_stack)
		ioi ld (STACKSEG), a
		nop
		ioi ld a, (DATASEG)
		nop
		ld (save_data), a
		ioi ld a, (SEGSIZE)
		nop
		ld (save_size), a
		pop af
		ret


save_xpc:	.word 0
save_stack:	.word 0
save_data:	.word 0
save_size:	.word 0


;
;	fork() innards
;	HL is the process to copy to, udata gives us the one to copy from
;
copy_and_map_process:
		call map_process_always
		ld a, (ix + P_TAB__P_PAGE_OFFSET)
		inc a		; skip common
		; This is our base address (remembering the low 4K is common
		; and special)
		ld xpc, a
		; The top 8K is now the lowest 8K of the new process
		; The rest of memory is our parent process
		ld hl, #0x1000
		ld de, #0xE000
		ld b, #13		; copy 13 4K banks this way
maincp:
		push bc
		ld de, #0xE000
		ld bc, #0x1000
ldirl:		ldi			; ldir is broken on R2000
		jr nz, ldirl
		inc a
		ld xpc, a
		pop bc
		djnz maincp
		;
		; Now we need to change tack as the XPC is covering the XPC
		; that we want to copy from
		ld (STACKSEG),a
		; Our stackseg (ie everything except common and xpc is now
		; pointing at the top 8K of the old process
		ld hl, #0x1000		; actually 0xE000 of the parent
		ld de, #0xE000		; XPC already has child E000 at E000
		ld bc, #0x2000
ldirl2:		ldi
		jr nz, ldirl2
		; Everything except the common is now done
		; page gives us the common 
		ld a,(ix + P_TAB__P_PAGE_OFFSET)
		ld xpc, a
		;
		; We are potentially using the stack as we copy but that is
		; fine as an IRQ in this lot will push stuff and pop it but
		; not trash what we need.
		;
		; Ok E000-FFFF is now the common of the child
		ld hl,_u_data
		ld de,#_u_data+0xE000
		ld bc,#512
ldirl3:
		ldi
		jr nz, ldirl3
		;
		; Flip the common, and our live stack (which is ok as the
		; return address is on the copy too
		;
		; We are now the child
		;
		ld a,(ix + P_TAB__P_PAGE_OFFSET)
		ld (DATASEG),a
		;
		; Put our mappings back sane
		;
		call map_kernel
		ret

;
;	High speed interrupts for I/O buffering
;
;	2 x 128 byte buffers per port, all linterrupt driven
;
;	These routines could be made faster if you really need to.
;
		.area _COMMONMEM
sera_int:
		push af				;	10
		push de				;	10
		push hl				;	10
		push ix				;	12
		ld ix,#sera_queue		;	8
		ioi ld a,(SASR)			;	11
		or a				;	2
		jp p, sera_txi			;	7
sera_rxi:
		ioi ld a,(SADR)			;	11
		ipset 3				;	4
		call ser_queuer			;	12
		ipres				;	4
		ret nz				;	2 (8)
		set OVERFLOW,(ix+FLAGS)		;	13
sera_txi:	bit 3,a				;	4
		jp nz,sera_end			;	7
		ipset 3				;	4
		call ser_dequeuew		;	12
		jp z, sera_txstop		;	7
		ioi ld (SADR),a			;	10
sera_back:
		ipres				;	4
sera_end:	bit 5,a				;	4
		jp z, sera_done			;	7
		set OVERFLOW,(ix+FLAGS);	;	13
sera_done:
		pop ix				;	9
		pop hl				;	7
		pop de				;	7
		pop af				;	7
		pop af				;	7
		ret				;	8
sera_txstop:
		; silence the tx ready interrupt without data
		ioi ld (SBSR),a			;	10
		jp sera_back			;	7
serb_int:
		push af				;	10
		push de				;	10
		push hl				;	10
		push ix				;	12
		ld ix,#serb_queue		;	8
		ioi ld a,(SBSR)			;	11
		or a				;	2
		jp p, serb_txi			;	7
serb_rxi:
		ioi ld a,(SBDR)			;	11
		ipset 3				;	4
		call ser_queuer			;	12
		ipres				;	4
		ret nz				;	2 (8)
		set OVERFLOW,(ix+FLAGS)		;	13
serb_txi:	bit 3,a				;	4
		jp nz,serb_end			;	7
		ipset 3				;	4
		call ser_dequeuew		;	12
		jp z, serb_txstop		;	7
		ioi ld (SBDR),a			;	10
serb_back:
		ipres				;	4
serb_end:	bit 5,a				;	4
		jp z, serb_done			;	7
		set OVERFLOW,(ix+FLAGS);	;	13
serb_done:
		pop ix				;	9
		pop hl				;	7
		pop de				;	7
		pop af				;	7
		pop af				;	7
		ret				;	8
serb_txstop:
		; silence the tx ready interrupt without data
		ioi ld (SBSR),a			;	10
		jp serb_back			;	7
serc_int:
		push af				;	10
		push de				;	10
		push hl				;	10
		push ix				;	12
		ld ix,#serc_queue		;	8
		ioi ld a,(SCSR)			;	11
		or a				;	2
		jp p, serc_txi			;	7
serc_rxi:
		ioi ld a,(SCDR)			;	11
		ipset 3				;	4
		call ser_queuer			;	12
		ipres				;	4
		ret nz				;	2 (8)
		set OVERFLOW,(ix+FLAGS)		;	13
serc_txi:	bit 3,a				;	4
		jp nz,serc_end			;	7
		ipset 3				;	4
		call ser_dequeuew		;	12
		jp z, serc_txstop		;	7
		ioi ld (SCDR),a			;	10
serc_back:
		ipres				;	4
serc_end:	bit 5,a				;	4
		jp z, serc_done			;	7
		set OVERFLOW,(ix+FLAGS);	;	13
serc_done:
		pop ix				;	9
		pop hl				;	7
		pop de				;	7
		pop af				;	7
		ret				;	8
serc_txstop:
		; silence the tx ready interrupt without data
		ioi ld (SCSR),a			;	10
		jp serc_back			;	7
serd_int:
		push af				;	10
		push de				;	10
		push hl				;	10
		push ix				;	12
		ld ix,#serd_queue		;	8
		ioi ld a,(SDSR)			;	11
		or a				;	2
		jp p, serd_txi			;	7
serd_rxi:
		ioi ld a,(SDDR)			;	11
		ipset 3				;	4
		call ser_queuer			;	12
		ipres				;	4
		ret nz				;	2 (8)
		set OVERFLOW,(ix+FLAGS)		;	13
serd_txi:	bit 3,a				;	4
		jp nz,serd_end			;	7
		ipset 3				;	4
		call ser_dequeuew		;	12
		jp z, serd_txstop		;	7
		ioi ld (SDDR),a			;	10
serd_back:
		ipres				;	4
serd_end:	bit 5,a				;	4
		jp z, serd_done			;	7
		set OVERFLOW,(ix+FLAGS);	;	13
serd_done:
		pop ix				;	9
		pop hl				;	7
		pop de				;	7
		pop af				;	7
		pop af				;	7
		ret				;	8
serd_txstop:
		; silence the tx ready interrupt without data
		ioi ld (SDSR),a			;	10
		jp serd_back			;	7

;
;	ix is the pointer to the next byte to use for data
;	ix + 1 is the high address
;	ix + 2 is the pointer to last read
;
ser_dequeuer:
		ld h,(ix+RPAGE)			;	9
		ld l,(ix+RRPTR)			;	9
		inc hl		; point to next byte to consume  2
		res 7,l		; wrap 128 bytes in	4
		ld a,(ix+RWPTR)			;	9
		cp l		; if the next byte to consume collides  2
		ret z				;	2/8
		ld a,(hl)	; with the data ptr then we're empty ; 5
		ld (ix+RRPTR),l			;	10
		ret		; will return NZ for data valid  8
ser_queuer:
		ld hl,(ix+RWPTR)		;	11
		ld a,l				;	2
		cp (ix+RRPTR)			;	9
		ret z		; no room		2 / 8
		ld (hl),e			;	6
		inc hl				;	2
		res 7,l		; wrap 128 bytes in	4
		ld (ix+RWPTR),l			;	10
		ret		; NZ for ok	;	8
;
;	Same logic but for upper 128 bytes
;
ser_dequeuew:
		ld h,(ix+TPAGE)
		ld l,(ix+TRPTR)
		inc hl		; point to next byte to consume
		set 7,l		; wrap around upper 128 bytes
		ld a,(ix+TWPTR)
		cp l		; if the next byte to consume collides
		ret z
		ld a,(hl)	; with the data ptr then we're empty
		ld (ix+TRPTR),l
		ret		; will return NZ for data valid
ser_queuew:
		ld hl,(ix+TWPTR)
		ld a,l
		cp (ix+TRPTR)
		ret z		; no room
		ld (hl),e
		inc hl
		set 7,l		; wrap around upper 128 bytes
		ld (ix+TWPTR),l
		ret		; NZ for ok

