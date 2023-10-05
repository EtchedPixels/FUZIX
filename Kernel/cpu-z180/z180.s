; 2014-12-19 William R Sowerbutts
; An attempt at generic Z180 support code, based on N8VEM Mark IV and DX-Design P112 targets

        .module z180
        .z180

        ; exported symbols
        .globl z180_init_hardware
        .globl z180_init_early
        .globl _init_hardware_c
        .globl _program_vectors
        .globl _copy_and_map_proc
        .globl interrupt_table ; not used elsewhere but useful to check correct alignment
        .globl hw_irqvector
        .globl _irqvector
        .globl _need_resched
	.globl _int_disabled
	.globl _udata

        ; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl _makeproc
	.globl _udata
        .globl _runticks
        .globl _chksigs
        .globl _getproc
        .globl _plt_monitor
        .globl _switchin
        .globl _plt_switchout
        .globl _dofork
	.globl map_buffers
        .globl map_kernel
	.globl map_kernel_restore
        .globl map_proc_always
        .globl map_kernel_di
        .globl map_proc_always_di
        .globl map_save_kernel
        .globl map_restore
	.globl map_for_swap
        .globl unix_syscall_entry
        .globl null_handler
        .globl nmi_handler
        .globl interrupt_handler
        .globl outchar
        .globl outcharhex
        .globl outhl, outde, outbc
        .globl outnewline
        .globl outstring
        .globl outstringhex
	.globl ___sdcc_enter_ix

.ifne CONFIG_SWAP
	.globl _do_swapout
	.globl _swapout
.endif
        .include "kernel.def"
        .include "../../cpu-z180/z180.def"
        .include "../../cpu-z80/kernel-z80.def"

; -----------------------------------------------------------------------------
; Initialisation code
; -----------------------------------------------------------------------------
        .area _DISCARD

z180_init_early:
        ; Assumes we enter with BBR=CBR and CBAR=x0 where x>0.
        ; If we are running in the first 64K we need to first copy ourselves elsewhere
        in0 a, (MMU_BBR)
        cp #(OS_BANK + FIRST_RAM_BANK)
        jr z, dommu             ; we're in position already
        cp #(OS_BANK + FIRST_RAM_BANK + 0x10)
        jr nc, finalcopy        ; greater than -- we're running above 64K
        ; we are running at least in part in the first 64K of RAM -- copy us up to 128KB temporarily
        ld a, #((0x20 + FIRST_RAM_BANK) >> 4)
        call copykernel
.if DEBUGCOMMON
        ld a, #'<'
        call outchar
        ld a, #'C'
        call outchar
        ld a, #'='
        call outchar
.endif
        ; reprogram the MMU to use the copy at 128KB
        ld a, #(0x20 + FIRST_RAM_BANK)
        out0 (MMU_BBR), a
        out0 (MMU_CBR), a
.if DEBUGCOMMON
        call outcharhex
        ld a, #'>'
        call outchar
.endif

finalcopy:
        ; copy us into position
        ld a, #((OS_BANK+FIRST_RAM_BANK) >> 4)
        call copykernel

dommu:
.if DEBUGCOMMON
        ld a, #'<'
        call outchar
        ld a, #'C'
        call outchar
        ld a, #'='
        call outchar
.endif
        ; reprogram the MMU to use the copy at bottom of RAM
        ld a, #(OS_BANK + FIRST_RAM_BANK)
        out0 (MMU_BBR), a       ; low 60K
        out0 (MMU_CBR), a       ; upper 4K (including our stack)
.if DEBUGCOMMON
        call outcharhex
        ld a, #'>'
        call outchar
.endif
        ; program MMU for 60KB/4KB bank/common1 split.
        ld a, #0xF0
        out0 (MMU_CBAR), a

        ret

copykernel:
        out0 (DMA_DAR0B), a

        ; HL = BBR << 4
        xor a
        ld h, a
        in0 l, (MMU_BBR)        ; read BBR value
        add hl, hl              ; shift left 4 bits
        add hl, hl
        add hl, hl
        add hl, hl

        ; load the DMA engine registers with source (HL<<8), destination (bank 
        ; register programmed already), and count (64KB)
        out0 (DMA_BCR0L), a     ; A=0 still
        out0 (DMA_BCR0H), a
        out0 (DMA_DAR0L), a
        out0 (DMA_DAR0H), a
        out0 (DMA_SAR0L), a
        out0 (DMA_SAR0H), l
        out0 (DMA_SAR0B), h
        ld bc, #0x0240
        out0 (DMA_DMODE), b     ; 0x02 - memory to memory, burst mode
        out0 (DMA_DSTAT), c     ; 0x40 - enable DMA channel 0
        ; in burst mode the Z180 CPU stops until the DMA completes
        ret

z180_init_hardware:
        ; setup interrupt vectors for the kernel bank
	ld a,#0xC3
        ; Set vector for jump to NULL
        ld (0x0000), a   
        ld hl, #null_handler  ;   to Our Trap Handler
        ld (0x0001), hl

        ld (0x0066), a  	; Set vector for NMI
        ld hl, #nmi_handler
        ld (0x0067), hl

	ld hl,#rstblock		; Install rst helpers
	ld de,#8
	ld bc,#32
	ldir

        ; program Z180 interrupt table registers
        ld hl, #interrupt_table ; note table MUST be 32-byte aligned!
        out0 (INT_IL), l
        ld a, h
        ld i, a
        im 1 ; set CPU interrupt mode for INT0

        ; set up system tick timer
        xor a
        out0 (TIME_TCR), a
        ld hl, #((CPU_CLOCK_KHZ * (1000/Z180_TIMER_SCALE) / TICKSPERSEC) - 1)
        out0 (TIME_RLDR0L), l
        out0 (TIME_RLDR0H), h
        ld a, #0x11         ; enable downcounting and interrupts for timer 0 only
        out0 (TIME_TCR), a

        ; Enable illegal instruction trap (vector at 0x0000)
        ; Enable external interrupts (INT0/INT1/INT2) 
        ld a, #0x87
        out0 (INT_ITC), a

        jp _init_hardware_c

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
        .area _CODE

_copy_and_map_proc:
        di          ; just to be sure
        pop bc      ; temporarily store return address
        pop de      ; function argument -- pointer to base page number
        push de     ; put stack back as it was
        push bc

        ; overwrites the full 64KB of target process memory space

        ; WARNING: 
        ; assumes processes page numbers are only 8-bits wide
        ; assumes kernel is physically 64K aligned
        ; assumes processes have a full 64K allocated to them

        ld bc, #0x0240
        out0 (DMA_DMODE), b     ; 0x02 - memory to memory, burst mode

        ; load destination page number into HL
        ld a, (de)
        ld b, a                 ; stash copy in B -- BC remains unmodified hereafter
        ld l, a
        ld h, #0

        ; shift left 4 bits
        add hl, hl
        add hl, hl
        add hl, hl
        add hl, hl
        ; now bottom four bits of H holds the top four bits of physical address,
        ; while the top four bits of L hold the next four bits.
        out0 (DMA_DAR0B), h

        ; source bank -- kernel is always 64K aligned
        ld a, #((OS_BANK + FIRST_RAM_BANK) >> 4)
        out0 (DMA_SAR0B), a

        ; Copy vectors -- virtual 0000 to 0080
        ld de, #0x0080
        out0 (DMA_BCR0H), d     ; 0x0080 bytes to copy
        out0 (DMA_BCR0L), e
        out0 (DMA_DAR0H), l     ; computed destination page
        out0 (DMA_DAR0L), d
        out0 (DMA_SAR0H), d     ; source is kernel, always 64K aligned
        out0 (DMA_SAR0L), d
        ; call dump_dma_state
        out0 (DMA_DSTAT), c     ; 0x40 - enable DMA channel 0
        ; CPU stalled until DMA completes

        ; Clone 0x7F into virtual 0080 through 0100 (kernel has code here, reserved in userspace)
        ; In the future interrupt stubs may go in here for processes with less than 64K allocated
        out0 (DMA_BCR0H), d     ; 0x80 bytes
        out0 (DMA_BCR0L), e
        dec e                   ; 0x80 -> 0x7F
        out0 (DMA_SAR0B), h
        out0 (DMA_SAR0H), l
        out0 (DMA_SAR0L), e
        ; no need to set DAR0B, DAR0H, DAR0L since they naturally ends up there after the above copy
        ; call dump_dma_state
        out0 (DMA_DSTAT), c     ; 0x40 - enable DMA channel 0
        ; CPU stalled until DMA completes

        ; Copy common memory code from kernel bank (from end of U_DATA to end of memory)
	;  ld de, #(0x10000 - U_DATA__TOTALSIZE - _udata) ; copy to end of memory
	; Only the linker isn't smart enough....
	or a
	push hl
	ld hl,#0
	ld de,#_udata + U_DATA__TOTALSIZE
	sbc hl,de
	ex de,hl
	pop hl
        out0 (DMA_BCR0H), d     ; set byte count
        out0 (DMA_BCR0L), e
        ld de, #(_udata + U_DATA__TOTALSIZE)
        ld a, #((OS_BANK + FIRST_RAM_BANK) >> 4)    ; source bank -- kernel is always 64K aligned
        out0 (DMA_SAR0B), a
        out0 (DMA_SAR0H), d     ; source is kernel, always 64K aligned
        out0 (DMA_SAR0L), e
        ; compute dest address; (HL << 8) + DE
        out0 (DMA_DAR0L), e
        ld a, l
        add d
        out0 (DMA_DAR0H), a
        ld a, h
        jr nc, bankok
        inc a
bankok: out0 (DMA_DAR0B), a
        ; call dump_dma_state
        out0 (DMA_DSTAT), c     ; 0x40 - enable DMA channel 0
        ; CPU stalled until DMA completes
        ; note we just overflowed at least one, possibly both DMA bank registers

        ; Copy user code (ie fill in the middle, 0x100 up to end of U_DATA) from the current process
        ; compute dest address; (HL << 8) + 0x0100
        xor a
        out0 (DMA_DAR0L), a
        ld a, l
        inc a
        out0 (DMA_DAR0H), a
        ld a, h
        jr nc, bankok2
        inc a
bankok2:out0 (DMA_DAR0B), a
        ; compute source address from current process 
        in0 l, (MMU_CBR)        ; get current process memory address
        ld h, #0
        add hl, hl              ; shift left 4 bits
        add hl, hl
        add hl, hl
        add hl, hl
        inc hl                  ; add in 0x100 start offset
        xor a
        out0 (DMA_SAR0L), a
        out0 (DMA_SAR0H), l
        out0 (DMA_SAR0B), h
        ld de, #(_udata + U_DATA__TOTALSIZE - 0x100) ; byte count
        out0 (DMA_BCR0H), d     ; set byte count
        out0 (DMA_BCR0L), e
        ; call dump_dma_state
        out0 (DMA_DSTAT), c     ; 0x40 - enable DMA channel 0
        ; CPU stalled until DMA completes

        ; finally reprogram the MMU to bring the new process common memory into context
        ; note this replaces the stack, but we just copied it over.
.if DEBUGCOMMON
        ld a, #'<'
        call outchar
        ld a, #'C'
        call outchar
        ld a, #'='
        call outchar
        ld a, b
        call outcharhex
        ld a, #'>'
        call outchar
.endif
        out0 (MMU_CBR), b

        ret ; was jp map_kernel but we never change MMU_BBR

_program_vectors:
        ; copy_and_map_proc has all the fun now
        ret

fork_proc_ptr: .dw 0 ; (C type is struct p_tab *) -- address of child process p_tab entry

;
;   Called from _fork. We are in a syscall, the uarea is live as the
;   parent uarea. The kernel is the mapped object.
;
_dofork:
        ; always disconnect the vehicle battery before performing maintenance
        di ; should already be the case ... belt and braces.

        pop de  ; return address
        pop hl  ; new process p_tab*
        push hl
        push de


        ld (fork_proc_ptr), hl

        ; prepare return value in parent process -- HL = p->p_pid;
        ld de, #P_TAB__P_PID_OFFSET
        add hl, de
        ld a, (hl)
        inc hl
        ld h, (hl)
        ld l, a

        ; Save the stack pointer and critical registers.
        ; When this process (the parent) is switched back in, it will be as if
        ; it returns with the value of the child's pid.
        push hl ; HL still has p->p_pid from above, the return value in the parent
        push ix
        push iy

        ; save kernel stack pointer -- when it comes back in the parent we'll be in
        ; _switchin which will immediately return (appearing to be _dofork()
        ; returning) and with HL (ie return code) containing the child PID.
        ; Hooray.
        ld (_udata + U_DATA__U_SP), sp

        ; now we're in a safe state for _switchin to return in the parent
        ; process.

        ; --------- copy process ---------
        ld hl, (fork_proc_ptr)
        ld de, #P_TAB__P_PAGE_OFFSET
        add hl, de
        push hl
        call _copy_and_map_proc
        pop hl

        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.
        pop bc
        pop bc
        pop bc

        ; Make a new process table entry, etc.
	ld hl, #_udata
	push hl
        ld hl, (fork_proc_ptr)
        push hl
        call _makeproc
        pop bc 
	pop bc

        ; runticks = 0;
        ld hl, #0
        ld (_runticks), hl
	;
        ; in the child process, fork() returns zero.
        ;
        ; And we exit, with the kernel mapped, the child now being deemed
        ; to be the live uarea. The parent is frozen in time and space as
        ; if it had done a switchout().
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; DEBUGGING
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; mmu_cbar_msg:  .ascii "MMU: CBAR="
;;                .db 0
;; mmu_cbr_msg:   .ascii ", CBR="
;;                .db 0
;; mmu_bbr_msg:   .ascii ", BBR="
;;                .db 0
;; 
;; mmu_state_dump:
;;             ld hl, #mmu_cbar_msg
;;             call outstring
;;             in0 a, (MMU_CBAR)
;;             call outcharhex
;;             ld hl, #mmu_cbr_msg
;;             call outstring
;;             in0 a, (MMU_CBR)
;;             call outcharhex
;;             ld hl, #mmu_bbr_msg
;;             call outstring
;;             in0 a, (MMU_BBR)
;;             call outcharhex
;;             call outnewline
;;             ret
;; ;------------------------------------------------------------------------------
;; dmamsg1:    .ascii "[DMA source="
;;             .db 0
;; dmamsg2:    .ascii ", dest="
;;             .db 0
;; dmamsg3:    .ascii ", count="
;;             .db 0
;; dmamsg4:    .ascii "]"
;;             .db 13, 10, 0
;; 
;; dump_dma_state:
;;         push af
;;         push hl
;;         push de
;;         push bc
;; 
;;         ld hl, #dmamsg1
;;         call outstring
;; 
;;         in0 a, (DMA_SAR0B)
;;         call outcharhex
;;         in0 a, (DMA_SAR0H)
;;         call outcharhex
;;         in0 a, (DMA_SAR0L)
;;         call outcharhex
;;         
;;         ld hl, #dmamsg2
;;         call outstring
;; 
;;         in0 a, (DMA_DAR0B)
;;         call outcharhex
;;         in0 a, (DMA_DAR0H)
;;         call outcharhex
;;         in0 a, (DMA_DAR0L)
;;         call outcharhex
;; 
;;         ld hl, #dmamsg3
;;         call outstring
;; 
;;         in0 a, (DMA_BCR0H)
;;         call outcharhex
;;         in0 a, (DMA_BCR0L)
;;         call outcharhex
;; 
;;         ld hl, #dmamsg4
;;         call outstring
;; 
;;         pop bc
;;         pop de
;;         pop hl
;;         pop af
;;         ret
;; 
;; 
;; dumpbuf: .ds 16
;; 
;; dump_process_memory:
;;         ; enter with the 64K bank to dump in A (low 4 bits only)
;;         out0 (DMA_SAR0B), a
;; 
;;         ld hl, #0
;;         ld a, #0x02
;;         out0 (DMA_DMODE), a     ; 0x02 - memory to memory, burst mode
;;         xor a
;;         out0 (DMA_SAR0H), a
;;         out0 (DMA_SAR0L), a
;;         ld a, #((OS_BANK + FIRST_RAM_BANK) >> 4)
;;         out0 (DMA_DAR0B), a
;; 
;; nextblock:
;;         ; set dest to our target buffer
;;         ld a, #<dumpbuf
;;         out0 (DMA_DAR0L), a
;;         ld a, #>dumpbuf
;;         out0 (DMA_DAR0H), a
;;         ; 16 bytes
;;         xor a
;;         out0 (DMA_BCR0H), a
;;         ld a, #0x10
;;         out0 (DMA_BCR0L), a
;;         ld a, #0x40
;;         out0 (DMA_DSTAT), a     ; 0x40 - enable DMA channel 0
;;         ; DMA does the copy
;; 
;;         ; print address
;;         call outhl
;;         ld a, #':'
;;         call outchar
;;         ld a, #' '
;;         call outchar
;; 
;;         ; print data
;;         ex de, hl
;;         ld hl, #dumpbuf
;;         ld b, #0x10
;; nextbyte:
;;         ld a, (hl)
;;         call outcharhex
;;         ld a, #' '
;;         call outchar
;;         inc hl
;;         djnz nextbyte
;;         ex de, hl
;; 
;;         call outnewline
;; 
;;         ld de, #0x10
;;         add hl, de
;;         ld a, h
;;         or l
;;         jr nz, nextblock
;;         ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK
; -----------------------------------------------------------------------------
        .area _COMMONMEM

        ; MUST arrange for this table to be 32-byte aligned
        ; linked immediately after commonmem.s
interrupt_table:
        .dw z180_irq1               ;     1    INT1 external interrupt - ? disconnected
        .dw z180_irq2               ;     2    INT2 external interrupt - SD card socket event
        .dw z180_irq3               ;     3    Timer 0
        .dw z180_irq_unused         ;     4    Timer 1
        .dw z180_irq_unused         ;     5    DMA 0
        .dw z180_irq_unused         ;     6    DMA 1
        .dw z180_irq_unused         ;     7    CSI/O
        .dw z180_irq8               ;     8    ASCI 0
        .dw z180_irq9               ;     9    ASCI 1
        .dw z180_irq_unused         ;     10
        .dw z180_irq_unused         ;     11
        .dw z180_irq_unused         ;     12
        .dw z180_irq_unused         ;     13
        .dw z180_irq_unused         ;     14
        .dw z180_irq_unused         ;     15
        .dw z180_irq_unused         ;     16

hw_irqvector:	.db 0
_irqvector:	.db 0
_need_resched:	.db 0
_int_disabled:	.db 1

z80_irq:
        push af
        xor a
        jr z180_irqgo

z180_irq1:
        push af
        ld a, #1
        jr z180_irqgo

z180_irq2:
        push af
        ld a, #2
        jr z180_irqgo

z180_irq3:
        push af
        ld a, #3
        ; fall through -- timer is likely to be the most common, we'll save it the jr
z180_irqgo:
        ld (hw_irqvector), a
        ; quick and dirty way to debug which interrupt is jamming us up ...
        ;    add #0x30
        ;    .globl outchar
        ;    call outchar
        pop af
        jp interrupt_handler

; z180_irq4:
;         push af
;         ld a, #4
;         jr z180_irqgo
; 
; z180_irq5:
;         push af
;         ld a, #5 
;         jr z180_irqgo
; 
; z180_irq6:
;         push af
;         ld a, #6
;         jr z180_irqgo
; 
; z180_irq7:
;         push af
;         ld a, #7
;         jr z180_irqgo

z180_irq8:
        push af
        ld a, #8
        jr z180_irqgo

z180_irq9:
        push af
        ld a, #9
        jr z180_irqgo

; z180_irq10:
;         push af
;         ld a, #10
;         jr z180_irqgo
; 
; z180_irq11:
;         push af
;         ld a, #11
;         jr z180_irqgo
; 
; z180_irq12:
;         push af
;         ld a, #12
;         jr z180_irqgo
; 
; z180_irq13:
;         push af
;         ld a, #13
;         jr z180_irqgo
; 
; z180_irq14:
;         push af
;         ld a, #14
;         jr z180_irqgo
; 
; z180_irq15:
;         push af
;         ld a, #15
;         jr z180_irqgo
; 
; z180_irq16:
;         push af
;         ld a, #16
;         jr z180_irqgo
z180_irq_unused:
        push af
        ld a, #0xFF
        jr z180_irqgo

.ifne CONFIG_SWAP
;
;	Swapping requires the swap out is done on a different stack so we
;	can flip the common of the victim for the write out
;
_swapout:
	pop de
	pop hl
	push hl
	push de			; HL is now the process to swap
	ld b,h			; Save the process pointer
	ld c,l
	ld de,#P_TAB__P_PAGE_OFFSET
	add hl,de
	ld a,(hl)		; Get the page info for this process
	ld hl,#0
	add hl,sp		; get the current SP into a register pair
	ld sp,#swapinstack
	in0 e,(MMU_CBR)		; save the old mapping
	out0 (MMU_CBR),a	; Swap the common to the victim
	push hl			; Save old stack frame
	push de			; Save old mapping
	push bc			; Argument for do_swapout
	call _do_swapout	; Swap the victim out in its own context
	pop bc			; discard
	pop bc			; Recover old mapping
	pop de			; Recover old stack
	ex de,hl		; DE is now return value, HL the stack
	di
	out0 (MMU_CBR), c	; Restore map first in case we takn an IRQ
	ld sp,hl		; Switch stack
	ex de,hl		; Return value back into HL
	jp map_kernel		; Ensure the lower mapping is fixed up if needed
.endif
;
; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
_plt_switchout:
	di
        ; save machine state
        ld hl, #0 ; return code set here is ignored, but _switchin can 
        ; return from either _switchout OR _dofork, so they must both write 
        ; U_DATA__U_SP with the following on the stack:
        push hl ; return code
        push ix
        push iy
        ld (_udata + U_DATA__U_SP), sp ; this is where the SP is restored in _switchin

        ; no need to stash udata on this platform since common memory is dedicated
        ; to each process.

        ; find another process to run (may select this one again)
        call _getproc

        push hl
        call _switchin

        ; we should never get here
        jp _plt_monitor

badswitchmsg: .ascii "_switchin: FAIL"
            .db 13, 10, 0
swapped: .ascii "_switchin: SWAPPED"
            .db 13, 10, 0

_switchin:
        di
        pop bc  ; return address
        pop de  ; new process pointer (struct p_tab *)
        push de ; restore stack (WRS: AC thinks this may not be required -- he's probably right!)
        push bc ; restore stack

        ; probably not reqired since we're only called from kernel code ...
        ; call map_kernel

        ld hl, #P_TAB__P_PAGE_OFFSET
        add hl, de  ; now HL points at the p_page value for the next process

        ; map in the common memory for the new process -- this swaps common
        ; memory and the stack under our feet so let's hope that common memory
        ; contains a copy of this code, eh?

.ifne CONFIG_SWAP
	.globl _swapper

        ld a, (hl)
	or a
	jr nz, is_resident
	; Swap in the new process (and maybe out an old one)
	ld sp, #swapstack
	ei
	xor a
	ld (_int_disabled),a
	push hl
	push de
	call _swapper
	pop de
	pop hl
	ld a,#1
	ld (_int_disabled),a
	di
.endif
is_resident:
	;
	; We have no valid stack at this point
	;
	ld a, (hl)
        ; out0 (MMU_BBR), a -- WRS: leave the kernel mapped in
        out0 (MMU_CBR), a

        ; sanity check: u_data->u_ptab matches what we wanted?
        ld hl, (_udata + U_DATA__U_PTAB) ; u_data->u_ptab
        or a                    ; clear carry flag
        sbc hl, de              ; subtract, result will be zero if DE==IX
        jr nz, switchinfail

        ; wants optimising up a bit
        ld ix, (_udata + U_DATA__U_PTAB)
        ; next_process->p_status = P_RUNNING
        ld P_TAB__P_STATUS_OFFSET(ix), #P_RUNNING

        ; WRS -- we can skip this for now until we start re-arranging processes in
        ; memory and/or support swapping to disk?
        ;; ; Fix the moved page pointers
        ;; ; Just do one byte as that is all we use on this platform
        ld a, P_TAB__P_PAGE_OFFSET(ix)
        ld (_udata + U_DATA__U_PAGE), a

        ; runticks = 0
        ld hl, #0
        ld (_runticks), hl

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
	;
	; Only from here is the stack valid again
	;
        ld sp, (_udata + U_DATA__U_SP)

        pop iy
        pop ix
        pop hl ; return code

        ; enable interrupts, if the ISR isn't already running
        ld a, (_udata + U_DATA__U_ININTERRUPT)
	ld (_int_disabled),a
        or a
        ret nz ; in ISR, leave interrupts off
        ei
        ret ; return with interrupts on

switchinfail:
        ; something went wrong and we didn't switch in what we asked for
        call outhl
        ld hl, #badswitchmsg
        call outstring
        jp _plt_monitor

map_kernel_restore:
map_buffers:
map_kernel_di:
map_kernel: ; map the kernel into the low 60K, leaves common memory unchanged
        push af
.if DEBUGBANK
        ld a, #'K'
        call outchar
.endif
        ld a, #(OS_BANK + FIRST_RAM_BANK)
        out0 (MMU_BBR), a
        pop af
        ret

map_proc_always_di:
map_proc_always: ; map the process into the low 60K based on current common mem (which is unchanged)
        push af
.if DEBUGBANK
        ld a, #'='
        call outchar
.endif
        ld a, (_udata + U_DATA__U_PAGE)
        out0 (MMU_BBR), a
.if DEBUGBANK
        call outcharhex
.endif
        ; MMU_CBR is left unchanged
        pop af
        ret

map_for_swap:
.if DEBUGBANK
	push af
        ld a, #'s'
        call outchar
.endif
	out0 (MMU_BBR),a	; the page is passed in A, so we just do an out0
.if DEBUGBANK
        call outcharhex
	pop af
.endif
	ret

map_save_kernel:   ; save the current process/kernel mapping
        push af
        in0 a, (MMU_BBR)
        ld (map_store), a
.if DEBUGBANK
        ld a, #'S'
        call outchar
.endif
        ld a, #(OS_BANK + FIRST_RAM_BANK)
        out0 (MMU_BBR), a
        pop af
        ret

map_restore: ; restore the saved process/kernel mapping
        push af
.if DEBUGBANK
        ld a, #'-'
        call outchar
.endif
        ld a, (map_store)
        out0 (MMU_BBR), a
.if DEBUGBANK
        call outcharhex
.endif
        pop af
        ret

map_store:  ; storage for map_save/map_restore
        .db 0

.ifne CONFIG_SWAP
	.area _COMMONMEM

	; Combining these is tricky so for the sake of 128 bytes or so we
	; don't try at this point.
	.ds 128
swapstack:
	.ds 128
swapinstack:
.endif

;
;	Stub helpers for code compactness. Note that
;	sdcc_enter_ix is in the standard compiler support already
;
	.area _DISCARD

;
;	The first two use an rst as a jump. In the reload sp case we don't
;	have to care. In the pop ix case for the function end we need to
;	drop the spare frame first, but we know that af contents don't
;	matter
;

rstblock:
	jp	___sdcc_enter_ix
	.ds	5
___spixret:
	ld	sp,ix
	pop	ix
	ret
	.ds	3
___ixret:
	pop	af
	pop	ix
	ret
	.ds	4
___ldhlhl:
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	ret
