; 2013-12-18 William R Sowerbutts
; socz80 hardware specific code (no doubt there's lots more of it elsewhere!)

            .module socz80

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl page17in
            .globl page17out
            .globl interrupt_handler
            .globl _program_vectors
            .globl _tty_putc
            .globl _tty_outproc
	    .globl map_kernel
	    .globl map_process
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore
	    .globl platform_interrupt_all
	    .globl _kernel_flag

            ; exported debugging tools
            .globl _trap_monitor
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl _inint
            .globl _tty_inproc
            .globl istack_top
            .globl istack_switched_sp
            .globl dispatch_process_signal
            .globl unix_syscall_entry
            .globl trap_illegal
	    .globl null_handler
            .globl _timer_interrupt
	    .globl nmi_handler
            .globl outnibble
            .globl outcharhex
            .globl outhl, outde, outbc
            .globl outnewline
            .globl outstring
            .globl outstringhex

            .include "socz80.def"
            .include "kernel.def"
            .include "../kernel.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM
_kernel_flag:
	    .db 1	; We start in kernel mode

trapmsg:    .ascii "Trapdoor: SP="
            .db 0
trapmsg2:   .ascii ", PC="
            .db 0
tm_user_sp: .dw 0

tm_stack:
            .db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
tm_stack_top:

_trap_monitor:
            ; stash SP
            ld (tm_user_sp), sp
            ; switch to temporary stack
            ld sp, #tm_stack_top
            call outnewline
            ld hl, #trapmsg
            call outstring
            ld hl, (tm_user_sp)
            call outhl
            ld hl, #trapmsg2
            call outstring
            ld hl, (tm_user_sp)
            ld e, (hl)
            inc hl
            ld d, (hl)
            call outde
            call outnewline
            ; dump MMU
            call mmu_state_dump
            ; map ROM
            xor a
            out (MMU_SELECT), a
            ld a, #0x20
            out (MMU_FRAMEHI), a
            xor a
            out (MMU_FRAMELO), a
            ; jump into to ROM, lovely ROM. come and give us a cuddle, ROM.
            jp 0x0000
            ; it's never a dull day with ROM around!

platform_interrupt_all:
	    ret

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xF000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
            ; use MMU to map fast SRAM (memory page 0x2001) into frame at 0xf000
            ld a, #0x0F
            out (MMU_SELECT), a
            ld a, #0x20
            out (MMU_FRAMEHI), a
            ld a, #0x01
            out (MMU_FRAMELO), a
            ; otherwise we assume the MMU is set up with DRAM from 0x0000 to 0xEFFF.

            ; we need to recover the contents of 0xF000, especially as it holds the stack 
            ; (with our return address on it!) the interrupt handler code, etc.

            ; map RAM that was at 0xF000 into 0xE000
            ld a, #0x0E
            out (MMU_SELECT), a
            xor a
            out (MMU_FRAMEHI), a
            ld a, #0x0F
            out (MMU_FRAMELO), a

            ; copy 0xE000 -> 0xF000 for 0x1000 bytes
            ld hl, #0xE000
            ld de, #0xF000
            ld bc, #0x1000
            ldir

            ; map page 001F (which will become the top page for our first process, init) into frame at 0xf000
            ld a, #0x0F
            out (MMU_SELECT), a
            xor a
            out (MMU_FRAMEHI), a
            ld a, #0x1f
            out (MMU_FRAMELO), a

            ; copy 0xE000 -> 0xF000 for 0x1000 bytes
            ld hl, #0xE000
            ld de, #0xF000
            ld bc, #0x1000
            ldir
            
            ; put 0xE000 back as it was
            ld a, #0x0E
            out (MMU_SELECT), a
            out (MMU_FRAMELO), a

            ret

init_hardware:
            ; set system RAM size
            ld hl, #8192
            ld (_ramsize), hl
            ; 6MB is reserved for the RAM disks, and 64K for the kernel, so ...
            ld hl, #1984		; (8192-4096-2048-64)
            ld (_procmem), hl

            ; set up timer hardware to interrupt us at 100Hz
            ld a, #TIMCMD_SEL_DOWNRESET
            out (TIMER_COMMAND), a

            ; (1000000 / 50) - 1 = 0x00004e1f
            ; (1000000 / 100) - 1 = 0x0000270f
            ; (1000000 / 250) - 1 = 0x0000049f
            ld a, #0x0F
            out (TIMER_VAL0), a
            ld a, #0x27
            out (TIMER_VAL1), a
            xor a
            out (TIMER_VAL2), a
            out (TIMER_VAL3), a

            ; reset downcounter
            ld a, #TIMCMD_DOWNRESET
            out (TIMER_COMMAND), a

            ; program timer control register
            in a, (TIMER_STATUS)
            set 6, a ; enable interrupt generation
            res 7, a ; clear any outstanding interrupt
            out (TIMER_STATUS), a

            ; program UART0 for interrupts on RX (not TX ... yet)
            in a, (UART0_STATUS)
            and #0xF0 ; clear bottom four bits only
            or  #0x0c ; enable TX & RX ints
            out (UART0_STATUS), a

            ; program UART1 similarly
            in a, (UART1_STATUS)
            and #0xF0 ; clear bottom four bits only
            or  #0x0c ; enable TX & RX ints
            out (UART1_STATUS), a

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

            im 1 ; set CPU interrupt mode
            ret


; tty_putc(uint8_t minor, uint8_t char)
_tty_putc:
            ; stack has: return address, minor, character
            ;                   0 1      2      3
            ; set HL to point to minor on stack
            ld hl, #2
            add hl,sp
            ld a, (hl)
            inc hl ; advance HL to point to character
            cp #1
            jr z, uart0putc
            cp #2
            jr z, uart1putc
            ret ; not a console we recognise
uart0putc:
tpc0loop:   ; wait for transmitter to be idle
            in a, (UART0_STATUS)
            bit 6, a
            jr nz, tpc0loop
            ; output character
            ld a, (hl)
            out (UART0_DATA), a
            ret
uart1putc:
tpc1loop:   ; wait for transmitter to be idle
            in a, (UART1_STATUS)
            bit 6, a
            jr nz, tpc1loop
            ; output character
            ld a, (hl)
            out (UART1_DATA), a
            ret

uart0_input: ; on arrival we know UART0_STATUS bit 7 is set (RX ready); we must preserve BC
            push bc
            in a, (UART0_DATA)
            ld b, a
            ld c, #1
indocall:   push bc
            call _tty_inproc
            pop bc
            pop bc
            ret

uart1_input: ; on arrival we know UART1_STATUS bit 7 is set (RX ready); we must preserve BC
            push bc
            in a, (UART1_DATA)
            ld b, a
            ld c, #2
            jr indocall

uart0_output: ; on arrival we know UART0_STATUS bit 6 is clear (TX idle); we must preserve BC
            push bc
            ld c, #1
outdocall:  push bc
            call _tty_outproc
            pop bc
            pop bc
            ret

uart1_output: ; on arrival we know UART1_STATUS bit 6 is clear (TX idle); we must preserve BC
            push bc
            ld c, #2
            jr outdocall

; write out DE bytes to MMU page17 register from (HL)
page17out:
            ld bc, #MMU_PAGE17 ; also loads B=0
            jr next256write
dowrite:
            ; register B contains number of bytes to write
            otir    ; writes B bytes (B=0 means 256)
            ; setup for next write
next256write:
            ld a, d
            or a
            jr z, writelastbytes
            ; D>0 so we do another 256 bytes
            dec d
            jr dowrite
writelastbytes:
            ; now D=0, just write E bytes
            ld a, e
            or a
            ret z
            ld b, e
            otir
            ret

; read in DE bytes to MMU page17 register to (HL)
page17in:
            ld bc, #MMU_PAGE17 ; also loads B=0
            ; read in DE bytes from MMU page17 register (in C) to (HL); B contains 0
            jr next256read
doread:
            ; register B contains number of bytes to read
            inir    ; reads B bytes (B=0 means 256)
            ; setup for next read
next256read:
            ld a, d
            or a
            jr z, readlastbytes
            ; D>0 so we do another 256 bytes
            dec d
            jr doread
readlastbytes:
            ; now D=0, just read E bytes
            ld a, e
            or a
            ret z
            ld b, e
            inir
            ret

_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
            di ; just to be sure
            pop de ; temporarily store return address
            pop hl ; function argument -- page table of this process
            push hl ; put stack back as it was
            push de

	    call map_process

            ; write zeroes across all vectors
            ld hl, #0x0
            ld de, #0x1
            ld bc, #0x7f ; program first 0x80 bytes only
            ld (hl), #0x00
            ldir

            ; now install the interrupt vector at 0x0038
            ld a, #0xC3 ; JP instruction
            ld (0x38), a
            ld hl, #interrupt_handler
            ld (0x39), hl

            ; set restart vector for UZI system calls
            ld (0x30), a   ;  (rst 30h is unix function call vector)
            ld hl, #unix_syscall_entry
            ld (0x31), hl

            ; Set vector for Illegal Instructions (this is presuambly a Z180 CPU feature? Z80 doesn't do this)
            ld (0x0), a
            ld hl, #trap_illegal   ;   to Our Trap Handler
            ld (0x1), hl

            ld (0x0000), a
            ld hl, #null_handler   ;   to Our Trap Handler
            ld (0x0001), hl

            ld (0x66), a  ; Set vector for NMI
            ld hl, #nmi_handler
            ld (0x0067), hl

	    call map_kernel
	    ret

mmumsg:     .ascii "MMU page "
            .db 0

mmu_state_dump:
            push bc
            push hl
            ld c, #0
            ld b, #16
dumpnextframe:
            ld hl, #mmumsg
            call outstring
            ld a, c
            out (MMU_SELECT), a
            call outnibble
            ld a, #':'
            call outchar
            ld a, #' '
            call outchar
            in a, (MMU_FRAMEHI)
            ld h, a
            in a, (MMU_FRAMELO)
            ld l, a
            call outhl
            call outnewline
            inc c
            djnz dumpnextframe
            pop hl
            pop bc
            ret



;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

;
; Mapping routines. Not yet all fixed up for the new style memory management
;
; Note: we don't flip the common space here, it's handled on task switch and
; in some cases in the swap driver
;
; HL points to the page array for this task, or NULL for kernel
; Preserve all but a, hl
map_process:
	    ld a, h
 	    or l
	    jr nz, map_process_always
	    ld hl, #os_bank
	    jr map_loadhl

;
; map the current process, preserves all registers
;
map_process_always:
	    push hl
	    ld hl, #U_DATA__U_PAGE
	    call map_loadhl
	    pop hl
	    ret
;
; map the kernel. preserves all registers
;
map_kernel:
	    push hl
	    ld hl, #os_bank
	    call map_loadhl
	    pop hl
	    ret
os_bank:    .db 0, 1, 2, 3	; first 64K

map_restore:
	    push hl
	    ld hl, #saved_map
	    call map_loadhl
	    pop hl
            ret

map_save:
	    ; FIXME: Save the current mapping registers. Preserves all
	    ; registers
	    push hl
	    ld hl, (map_current)
	    ld (saved_map), hl
	    ld hl, (map_current + 2)
	    ld (saved_map + 2), hl
	    pop hl
	    ret

map_loadhl:
	    ;
	    ;	Switch all but the top page frame according to the passed
	    ;	page set. We have to do some work as we've got a rather
	    ;	fancy MMU while Fuzix is expecting simple banks
	    ;
	    push af
	    push bc
	    push de
	    ld de, #map_current
	    push hl
	    ldi
	    ldi
	    ldi
	    ldi
	    pop hl
	    ld bc, #MMU_SELECT	; 	page 0, MMU select for port
	    xor a
	    call map4
	    call map4
	    call map4
	    call map3		;	top 4K is common memory
	    pop de
	    pop bc
	    pop af
	    ret

;
;	Map the 4 pages indicated by the 16K page number at (hl)
;	into the banks starting at B. The caller must point C at
;	MMU_SELECT (where it will remain)
;
;	Returns hl pointing to the next page number, B indexing the next bank
;

map3:	    push hl
	    ld a, #3
	    jr map3or4

map4:	    push hl
	    ld a, #4
map3or4:
	    ; Load the frame pointer first to free up a register
	    ld e, (hl)		;	pageframe (in 16K terms)

	    add b		;	stop point
	    ld l, a		; 	save stop marker

	    ; Convert the frame pointer to something useful
	    sla e		;	into 8K
	    rla
	    sla e		;	into 4K pages
	    rla			;
	    and #0x03		;	clean top bits
	    ld d, a
map4loop:
	    out (c), b		;	page to update
	    ld a, d
	    out (MMU_FRAMEHI), a
	    ld a, e
	    out (MMU_FRAMELO), a
	    inc b		;	next frame in the MMU
	    inc e		;	next 4K page
				;	A 16K block will never span
				;	a higher power of 2, so d never
				;	needs to change
	    ld a, b
	    cp l
	    jr nz, map4loop
	    pop hl
	    inc hl
	    ret

saved_map:  .db 0, 0, 0, 0
map_current: .db 0, 0, 0, 0

; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
            push bc
            ld b, a
            ; wait for transmitter to be idle
ocloop:     in a, (UART0_STATUS)
            bit 6, a
            jr nz, ocloop   ; loop while busy
            ; now output the char to serial port
            ld a, b
            out (UART0_DATA), a
            pop bc
            ret
