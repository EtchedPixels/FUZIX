;
;	MTX512 hardware support
;


            .module mtx

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl _program_vectors
	    .globl platform_interrupt_all

	    .globl map_kernel
	    .globl map_kernel_di
	    .globl map_process
	    .globl map_process_di
	    .globl map_process_a
	    .globl map_process_always
	    .globl map_process_always_di
	    .globl map_save_kernel
	    .globl map_restore

	    .globl _int_disabled

	    .globl _sil_memcpy

	    .globl _irqvector

            ; exported debugging tools
            .globl _platform_monitor
            .globl _platform_reboot
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem
	    .globl _membanks

	    .globl unix_syscall_entry
            .globl null_handler
	    .globl nmi_handler
            .globl interrupt_handler

	    .globl vdpinit
	    .globl vdpload
	    .globl _vtinit

            .globl outcharhex
            .globl outhl, outde, outbc
            .globl outnewline
            .globl outstring
            .globl outstringhex

            .include "kernel.def"
            .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xC000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM
;
;	This is linked straight after the udata block so is 256 byte
; aligned. Put the im2 table first
;
	     .globl intvectors

intvectors:
	    .dw ctc0_int	; VDP counting timer
	    .dw bogus_int	; Baud generator
	    .dw bogus_int	; Baud generator
	    .dw trace_int	; can be used for CPU tracing etc
	    .dw serial_int	; we don't use the vector mod functions

_int_disabled:
	    .db 1

_platform_monitor:
	    di
	    halt

_platform_reboot:
	    di
	    xor a
	    out (0),a		; ROM mode, we are in common so survive
	    rst 0		; kaboom

_irqvector:
	    .db 0		; used to identify the vector in question

ctc0_int:   push af
	    xor a
	    ld (_irqvector), a
	    pop af
	    jp interrupt_handler
trace_int:
bogus_int:  reti
serial_int:
	    push af
	    ld a, #1
	    ld (_irqvector), a
	    pop af
	    jp interrupt_handler

; Kernel is in 0x80 so we just need to count the banks that differ
size_ram:
	    ld hl, #0x1000	; good as anywhere
	    ld bc, #0x8100	; port 0 in c, b = 0x81
size_next:
	    out (c), b
	    ld a, #0xA5
	    ld (hl), a
	    cp (hl)
	    jr nz, size_nonram
	    cpl
	    ld (hl), a
	    cp (hl)
	    jr nz, size_nonram
	    inc b
	    ld a, #0x8B
	    cp b
	    jr nz, size_next	; All banks done
size_nonram:
	    ld a, #0x80
	    out (c), a		; Return to kernel
	    res 7,b		; Clear the flag so we just have banks
	    dec b		; Last valid bank
	    ld a, b
	    ld (_membanks), a
	    ld hl, #0
	    ld de, #48
sizer:	    add hl, de
	    djnz sizer
	    ret

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xC000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------

	    .area _CONST
crtcmap:
	    .db	   0x77, 0x50, 0x5c, 0x09, 0x1e, 0x03, 0x18, 0x1b
	    .db    0x00, 0x09, 0x60, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00

            .area _CODE

init_early:
;
;	Bring up the 80 column card early so it can be used for debug
;
	    ld hl, #crtcmap
	    xor a
	    ld bc, #0x1139		; 17 commands to port 0x38/39
init6845:
	    dec c
	    out (c), a
	    inc c
	    inc a
	    outi
	    jr nz, init6845
            ret

init_hardware:
	    call size_ram
	    ; FIXME: do proper size checker
            ; set system RAM size (hardcoded for now)
            ld (_ramsize), hl
	    and a
	    sbc hl, de			; DE will hold 48 at this point
            ld (_procmem), hl

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

	    ; Program the video engine
	    call vdpinit
	    call vdpload

	    ; 08 is channel 0, which is input from VDP
            ; 09 is channel 1, output for DART ser 0 } fed 4MHz/13
            ; 0A is channel 2, output for DATA ser 1 }
	    ; 0B is channel 3, counting CSTTE edges (cpu clocks) at 4MHz

	    xor a
	    out (0x08), a		; vector 0
	    ld a, #0xC5
	    out (0x08), a		; CTC 0 as our IRQ source
	    ld a, #0x01
	    out (0x08), a		; Timer constant

	    ld hl, #intvectors		; Work around SDCC crappiness
	    ld a, h
	    ld i, a
            im 2 ; set CPU interrupt mode

	    call _vtinit		; init the console video

            ret

_sil_memcpy:
	    push ix
	    ld ix, #0
	    add ix, sp
	    ld l, 6(ix)	; dptr
	    ld h, 7(ix)
	    ld a, 5(ix)	; map
	    ld e, 8(ix)	; block
	    ld d, 9(ix)
	    call map_process_a	; map in the user space we want
	    ld c, 11(ix) ; port base
	    out (c), e	 ; block low
	    inc c
	    out (c), d	 ; block high
	    inc c
	    inc c
	    ld b, #0
	    bit 0, 4(ix) ; read ? 
	    jr z, sil_mwrite
	    inir	 ; load 256 bytes
	    inir	 ; load 256 bytes
sil_copydone:
	    call map_kernel
	    pop ix
	    ret
sil_mwrite:
	    otir	; write 256 bytes
	    otir	; write 256 bytes
	    jr sil_copydone

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM


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

            ; Set vector for jump to NULL
            ld (0x0000), a   
            ld hl, #null_handler  ;   to Our Trap Handler
            ld (0x0001), hl

            ld (0x0066), a  ; Set vector for NMI
            ld hl, #nmi_handler
            ld (0x0067), hl

	    ; our platform has a "true" common area, if it did not we would
	    ; need to copy the "common" code into the common area of the new
	    ; process.

	    ; falls through

            ; put the paging back as it was -- we're in kernel mode so this is predictable
map_kernel:
map_kernel_di:
	    push af
	    ld a, #0x80		; ROM off bank 0
	    ; the map port is write only, so keep a local stash
	    ld (map_copy), a
	    out (0), a
	    pop af
            ret
map_process:
map_process_di:
	    ld a, h
	    or l
	    jr z, map_kernel
	    ld a, (hl)
map_process_a:
	    ld (map_copy), a
	    out (0), a
	    ret
map_process_always:
map_process_always_di:
	    push af
	    ld a, (U_DATA__U_PAGE)
map_set_a:
	    ld (map_copy), a
	    out (0), a
	    pop af
	    ret
map_save_kernel:
	    push af
	    ld a, (map_copy)
	    ld (map_store), a
	    ld a, #0x80		; ROM off bank 0
	    ; the map port is write only, so keep a local stash
	    ld (map_copy), a
	    out (0), a
	    pop af
            ret
map_restore:
	    push af
	    ld a, (map_store)
	    jr map_set_a

map_store:
	    .db 0
map_copy:
	    .db 0

; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
	    out (0x0c), a
            ret
