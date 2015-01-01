; 2014-12-19 William R Sowerbutts
; An attempt at generic Z180 support code, based on N8VEM Mark IV and DX-Design P112 targets

        .module z180
        .z180

        ; exported symbols
        .globl z180_init_hardware
        .globl z180_init_early
        .globl _program_vectors
        .globl _copy_and_map_process
        .globl interrupt_table ; not used elsewhere but useful to check correct alignment
        .globl _irqvector
        .globl _kernel_flag

        ; imported symbols
        .globl _ramsize
        .globl _procmem
        .globl _newproc
        .globl _runticks
        .globl _chksigs
        .globl _inint
        .globl _getproc
        .globl _trap_monitor
        .globl _switchin
        .globl _switchout
        .globl _dofork
        .globl map_kernel
        .globl map_process_always
        .globl map_save
        .globl map_restore
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

        .include "kernel.def"
        .include "../cpu-z180/z180.def"
        .include "../kernel.def"

; -----------------------------------------------------------------------------
; Constant used for timer configuration
; -----------------------------------------------------------------------------
TIMER_TICK_RATE = 100 ; Hz

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

        ; (this code used to be in _program_vectors but now we do it once and copy)
        ; write HALT across all vectors
        ld hl, #0
        ld de, #1
        ld bc, #0x007f ; program first 0x80 bytes only
        ld (hl), #0x76 ; HALT instruction
        ldir

        ; now install the interrupt vector at 0x0038
        ld a, #0xC3 ; JP instruction
        ld (0x0038), a
        ld hl, #z80_irq
        ld (0x0039), hl

        ; set restart vector for UZI system calls
        ld (0x0030), a   ;  (rst 30h is unix function call vector)
        ld hl, #unix_syscall_entry
        ld (0x0031), hl

        ; Set vector for jump to NULL
        ld (0x0000), a   
        ld hl, #null_handler  ;   to Our Trap Handler
        ld (0x0001), hl

        ld (0x0066), a  ; Set vector for NMI
        ld hl, #nmi_handler
        ld (0x0067), hl

        ; program Z180 interrupt table registers
        ld hl, #interrupt_table ; note table MUST be 32-byte aligned!
        out0 (INT_IL), l
        ld a, h
        ld i, a
        im 1 ; set CPU interrupt mode for INT0

        ; set up system tick timer
        xor a
        out0 (TIME_TCR), a
        ld hl, #(CPU_CLOCK_KHZ * (1000/20) / TIMER_TICK_RATE) ; timer ticks at PHI/20
        out0 (TIME_RLDR0L), l
        out0 (TIME_RLDR0H), h
        ld a, #0x11         ; enable downcounting and interrupts for timer 0 only
        out0 (TIME_TCR), a

        ; Enable illegal instruction trap (vector at 0x0000)
        ; Enable external interrupts (INT0/INT1/INT2) 
        ld a, #0x87
        out0 (INT_ITC), a
        ret

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xC000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
        .area _CODE

_kernel_flag:
        .db 1

_copy_and_map_process:
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
        ld de, #(0x10000 - U_DATA__TOTALSIZE - U_DATA) ; copy to end of memory
        out0 (DMA_BCR0H), d     ; set byte count
        out0 (DMA_BCR0L), e
        ld de, #(U_DATA+U_DATA__TOTALSIZE)
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
        ld de, #(U_DATA + U_DATA__TOTALSIZE - 0x100) ; byte count
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
        ; copy_and_map_process has all the fun now
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
        ld (U_DATA__U_SP), sp

        ; now we're in a safe state for _switchin to return in the parent
        ; process.

        ; --------- copy process ---------
        ld hl, (fork_proc_ptr)
        ld de, #P_TAB__P_PAGE_OFFSET
        add hl, de
        push hl
        call _copy_and_map_process
        pop hl

        ; now the copy operation is complete we can get rid of the stuff
        ; _switchin will be expecting from our copy of the stack.
        pop bc
        pop bc
        pop bc

        ; Make a new process table entry, etc.
        ld hl, (fork_proc_ptr)
        push hl
        call _newproc
        pop bc 

        ; runticks = 0;
        ld hl, #0
        ld (_runticks), hl
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
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
        .area _COMMONMEM

        ; MUST arrange for this table to be 32-byte aligned
        ; linked immediately after commonmem.s
interrupt_table:
        .dw z180_irq_unused         ;     1    INT1 external interrupt - ? disconnected
        .dw z180_irq_unused         ;     2    INT2 external interrupt - SD card socket event
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

_irqvector: .db 0

z80_irq:
        push af
        xor a
        jr z180_irqgo

; z180_irq1:
;         push af
;         ld a, #1
;         jr z180_irqgo
; 
; z180_irq2:
;         push af
;         ld a, #2
;         jr z180_irqgo

z180_irq3:
        push af
        ld a, #3
        ; fall through -- timer is likely to be the most common, we'll save it the jr
z180_irqgo:
        ld (_irqvector), a
        ; quick and dirty way to debug which interrupt is jamming us up ...
        ;    add #0x30
        ;    .globl outchar
        ;    call outchar
        pop af
        jp interrupt_handler
        ; this isn't perfect -- interrupt_handler always ends with RETI while only
        ; INT0 should end with RETI, the others ending with a normal RET. unless
        ; there are Z80 interrupt peripherals on the bus we'll be OK.

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

; Switchout switches out the current process, finds another that is READY,
; possibly the same process, and switches it in.  When a process is
; restarted after calling switchout, it thinks it has just returned
; from switchout().
; 
; This function can have no arguments or auto variables.
_switchout:
        di
        call _chksigs
        ; save machine state

        ld hl, #0 ; return code set here is ignored, but _switchin can 
        ; return from either _switchout OR _dofork, so they must both write 
        ; U_DATA__U_SP with the following on the stack:
        push hl ; return code
        push ix
        push iy
        ld (U_DATA__U_SP), sp ; this is where the SP is restored in _switchin

        ; set inint to false
        xor a
        ld (_inint), a

        ; no need to stash udata on this platform since common memory is dedicated
        ; to each process.

        ; find another process to run (may select this one again)
        call _getproc

        push hl
        call _switchin

        ; we should never get here
        jp _trap_monitor

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

        ; when we add support for swap we should check here that (hl) is non-zero

        ; map in the common memory for the new process -- this swaps common
        ; memory and the stack under our feet so let's hope that common memory
        ; contains a copy of this code, eh?
        ld a, (hl)
        ; out0 (MMU_BBR), a -- WRS: leave the kernel mapped in
        out0 (MMU_CBR), a

        ; sanity check: u_data->u_ptab matches what we wanted?
        ld hl, (U_DATA__U_PTAB) ; u_data->u_ptab
        or a                    ; clear carry flag
        sbc hl, de              ; subtract, result will be zero if DE==IX
        jr nz, switchinfail

        ; wants optimising up a bit
        ld ix, (U_DATA__U_PTAB)
        ; next_process->p_status = P_RUNNING
        ld P_TAB__P_STATUS_OFFSET(ix), #P_RUNNING

        ; WRS -- we can skip this for now until we start re-arranging processes in
        ; memory and/or support swapping to disk?
        ;; ; Fix the moved page pointers
        ;; ; Just do one byte as that is all we use on this platform
        ;; ld a, P_TAB__P_PAGE_OFFSET(ix)
        ;; ld (U_DATA__U_PAGE), a

        ; runticks = 0
        ld hl, #0
        ld (_runticks), hl

        ; restore machine state -- note we may be returning from either
        ; _switchout or _dofork
        ld sp, (U_DATA__U_SP)

        pop iy
        pop ix
        pop hl ; return code

        ; enable interrupts, if the ISR isn't already running
        ld a, (_inint)
        or a
        ret z ; in ISR, leave interrupts off
        ei
        ret ; return with interrupts on

switchinfail:
        ; something went wrong and we didn't switch in what we asked for
        call outhl
        ld hl, #badswitchmsg
        call outstring
        jp _trap_monitor

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

map_process_always: ; map the process into the low 60K based on current common mem (which is unchanged)
        push af
.if DEBUGBANK
        ld a, #'='
        call outchar
.endif
        ld a, (U_DATA__U_PAGE)
        out0 (MMU_BBR), a
.if DEBUGBANK
        call outcharhex
.endif
        ; MMU_CBR is left unchanged
        pop af
        ret

map_save:   ; save the current process/kernel mapping
        push af
        in0 a, (MMU_BBR)
        ld (map_store), a
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
