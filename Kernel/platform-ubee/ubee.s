;
;	    Microbee 128K and 256TC  hardware support
;

            .module ubee

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl map_kernel
	    .globl map_process
	    .globl map_process_a
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore
	    .globl platform_interrupt_all
	    .globl _kernel_flag

	    ; hard disk helpers
	    .globl _hd_xfer_in
	    .globl _hd_xfer_out
	    ; and the page from the C code
	    .globl _hd_page

            ; exported debugging tools
            .globl _trap_monitor
            .globl _trap_reboot
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl istack_top
            .globl istack_switched_sp
            .globl unix_syscall_entry
            .globl trap_illegal
            .globl outcharhex
	    .globl fd_nmi_handler
	    .globl null_handler

	    .globl _ubee_model

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

            .include "kernel.def"
            .include "../kernel.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xE800 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_trap_monitor:
	    di
	    call map_kernel
	    jp to_monitor

platform_interrupt_all:
	    in a,(0xef)
	    ret

_trap_reboot:
	    di
	    call map_kernel
	    jp to_reboot

;
;	Sit in common and play with th banks to see what we have
;
size_ram:
	    ; We could have < 128, 128 or various extensions up to 512K or
	    ; so.
	    ld ix,#0x80			; safe scribble
	    ld c,#0x01
	    ld de,#page_codes
	    ld (ix),#0			; clear in bank 0 low
	    ld hl,#32
scan_ram:
	    add hl,hl
	    ld a,(de)			; try entry in table
	    or a
	    jr z,scan_done		; finished
	    inc de
	    out (0x50),a		; select proposed bank
	    ld (ix),c			; write to it
	    ld a,#4
	    out (0x50),a		; back to bank 0
	    ld a,(ix)			; read it back
	    cp c			; did it mess with bank 0L ?
	    jr nz, scan_ram

;	We found our first mismatch HL is our memory size
;	info if at least 128K is present (or 64 if not)
scan_done:
	    ret

page_codes:
	    ; Detect a standard or premium 128K system
	    .byte 0x06  ;	write 1 to bank 1L 0x80
	    ; Detect a system with a 256K expansion mod
	    .byte 0x44	;	write 1 to bank 2L 0x80
	    ; Detect a system with a 512K expansion mod
	    .byte 0x84	;	write 1 to bank 4L 0x80
	    ; We don't handle the modern ubee premium plus emulated thing
	    .byte 0x00  ;	and done

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xE800, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

; FIXME: most of this belongs in discard

; These two must be below 32K and not use the stack until they hit ROM
; space.
;
; Not sure we can do this for most cases because not everyone has all that
; ROM
;
to_monitor:
	    xor a			; 0 or 1 to keep low 32K right ? */
	    out (0x50), a		; ROMS please
	    jp 0xE003			; Monitor

to_reboot:
	    xor a
	    out (0x50), a
	    jp 0xE000
	
;
;	This setting list comes from the Microbee 256TC documentation
;	and is the quoted table for 80x25 mode
;
_ctc6545:				; registers in reverse order
	    .db 0x00, 0x00, 0x00, 0x20, 0x0A, 0x09, 0x0A, 0x48
	    .db 0x1a, 0x19, 0x05, 0x1B, 0x37, 0x58, 0x50, 0x6B

init_early:
            ; load the 6545 parameters
	    ld hl, #_ctc6545
	    ld bc, #0x0F0C
ctcloop:    out (c), b			; register
	    ld a, (hl)
	    out (0x0D), a		; data
	    inc hl
	    dec b
	    jp p, ctcloop
	    ; ensure the CTC clock is right
	    ld a, #0
	    in a, (9)			; manual says in but double check
	    in a, (0x1C)
	    and #0x7F
	    out (0x1C), a		; ensure we are in simple mode for now
   	    ; clear screen
	    ld hl, #0xF000
	    ld (hl), #'*'		; debugging aid in top left
	    inc hl
	    ld de, #0xF002
	    ld bc, #1998
	    ld (hl), #' '
	    ldir
            ret

init_hardware:
	    ld a,(_ubee_model)
	    cp #2			; 256TC
	    ld hl,#256			; 256TC has 256K
	    call nz, size_ram
is_tc:
            ld (_ramsize), hl
	    ld de,#64			; 64K for kernel
	    or a
	    sbc hl,de
            ld (_procmem), hl

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

	    ld a,(_ubee_model)
	    cp #2
	    ld hl,#pio_setup
	    jr nz, not_t256
	    ld hl,#pio_setup_t256
not_t256:
	    call init_ports
	    call init_ports
	    ;
	    ; set up the RTC driven periodic timer. The PIA should already
	    ; have been configured for us
	    ;
	    ; we don't necessarily have one in which case we have a
	    ; (arguably far more useful) vblank timer. In which case this
	    ; routine will pee into the void and do no harm whatsoever
	    ;
	    ld bc,#0x0A04
	    out (c), b			; select register A
	    ld a,#0x70			; reset divier
	    out (0x06), a
	    ld a,#0x2D			; and select 32KHz operation
	    out (0x06), a		; with an 8Hz interrupt
	    ld a,#0x40
	    inc b
	    out (c),b
	    ld a,#0x46			; PIE, binary, 24 hour
	    out (0x06), a
	    inc b
	    out (c),b
	    in a,(0x07)			; Clear pending interrupt

            im 1 ; set CPU interrupt mode

            ret

;
;	This nifty routine is c/o Stewart Kay's CP/M 3 for the Microbee
;	- slightly tweaked to make it re-callable for series of tables
;
init_ports:
	    ld b,(hl)
	    inc hl
	    ld c,(hl)
	    inc hl
	    otir
	    ld a,(hl)
	    or a
	    jr nz,init_ports
	    inc hl
	    ret

pio_setup:
	    .byte	0x04
	    .byte	0x01
	    ; vector, mode 0 (output), int control off, on
	    .byte	0x00,0x0F,0x03,0x83
	    .byte	0
	    ; and port B
	    .byte	0x06
	    .byte	0x03
	    ; vector 0, mode 3, 7/4/3/0 are input
	    ; interrupt enable, or, high, mask follows
	    ; interrupt on 7/4
	    ; int control on
	    .byte	0x00, 0xCF, 0x99, 0xB7, 0x6F, 0x83
	    .byte	0x01
	    .byte	0x02
	    ; and set the data lines so the rs232 looks sane (not that
	    ; we care right now).
	    .byte	0x24
	    .byte	0x00

pio_setup_t256:
	    .byte	0x04
	    .byte	0x01
	    ; vector, mode 0 (output), int control off, on
	    .byte	0x00,0x0F,0x03,0x83
	    .byte	0
	    ; and port B
	    .byte	0x06
	    .byte	0x03
	    ; vector 0, mode 3, 7/4/3/1/0 are input
	    ; interrupt enable, or, high, mask follows
	    ; interrupt on 7/4/1
	    ; int control on
	    .byte	0x00, 0xCF, 0x9B, 0xB7, 0x6D, 0x83
	    .byte	0x01
	    .byte	0x02
	    ; and set the data lines so the rs232 looks sane (not that
	    ; we care right now).
	    .byte	0x24
	    .byte	0x00

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

mapreg:    .db 0
mapsave:   .db 0

_kernel_flag:
	    .db 1	; We start in kernel mode

_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
            di ; just to be sure
            pop de ; temporarily store return address
            pop hl ; function argument -- base page number
            push hl ; put stack back as it was
            push de

	    call map_process

            ; write zeroes across all vectors
            ld hl, #0
            ld de, #1
            ld bc, #0x007f ; program first 0x80 bytes only
            ld (hl), #0x00
            ldir

            ; now install the interrupt vector at 0x0038
            ld a, #0xC3 ; JP instruction
            ld (0x0038), a
            ld hl, #interrupt_handler
            ld (0x0039), hl

            ; set restart vector for UZI system calls
            ld (0x0030), a   ;  (rst 30h is unix function call vector)
            ld hl, #unix_syscall_entry
            ld (0x0031), hl

            ld (0x0000), a   
            ld hl, #null_handler   ;   to Our Trap Handler
            ld (0x0001), hl

            ld (0x0066), a  ; Set vector for NMI
            ld hl, #fd_nmi_handler
            ld (0x0067), hl

;
;	Mapping set up for the Microbee
;
;	The top 32K bank holds kernel code and pieces of common memory
;	The lower 32K is switched between the various user banks.
;
map_kernel:
	    push af
	    ld a, #0x04		; bank 0, 1 no ROM - FIXME: map video over kernel
	    ld (mapreg), a
	    out (0x50), a
	    pop af
	    ret
map_process:
	    ld a, h
	    or l
	    jr z, map_kernel
map_process_hl:
	    ld a, (hl)
map_process_a:			; used by bankfork
	    ld (mapreg), a
	    out (0x50), a
            ret

map_process_always:
	    push af
	    push hl
	    ld hl, #U_DATA__U_PAGE
	    call map_process_hl
	    pop hl
	    pop af
	    ret

map_save:   push af
	    ld a, (mapreg)
	    ld (mapsave), a
	    pop af
	    ret

map_restore:
	    push af
	    ld a, (mapsave)
	    ld (mapreg), a
	    out (0x50), a
	    pop af
	    ret
	    
; No UART (could use printer port ?)
outchar:
            ret

;
;	Swap helpers
;
_hd_xfer_in:
	   pop de
	   pop hl
	   push hl
	   push de
	   ld a, (_hd_page)
	   or a
	   call nz, map_process_a
	   ld bc, #0x40			; 512 bytes from 0x40
	   inir
	   inir
	   call map_kernel
	   ret

_hd_xfer_out:
	   pop de
	   pop hl
	   push hl
	   push de
	   ld a, (_hd_page)
	   or a
	   call nz, map_process_a
	   ld bc, #0x40			; 512 bytes to 0x40
	   otir
	   otir
	   call map_kernel
	   ret
