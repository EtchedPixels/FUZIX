;
;	MTX512 hardware support
;


            .module mtx

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl _program_vectors
            .globl _system_tick_counter
	    .globl platform_interrupt_all

	    .globl map_kernel
	    .globl map_process
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore

	    .globl _sil_memcpy

	    .globl _kernel_flag

            ; exported debugging tools
            .globl _trap_monitor
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem

	    .globl unix_syscall_entry
            .globl null_handler
	    .globl nmi_handler
            .globl interrupt_handler

	    .globl vdpinit
	    .globl _vtinit

            .globl outcharhex
            .globl outhl, outde, outbc
            .globl outnewline
            .globl outstring
            .globl outstringhex

            .include "kernel.def"
            .include "../kernel.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xC000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_trap_monitor:
	    di
	    halt

_trap_reboot:
	    di
	    xor a
	    out (0),a		; ROM mode, we are in common so survive
	    rst 0		; kaboom

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
	    ; FIXME: do proper size checker
            ; set system RAM size (hardcoded for now)
            ld hl, #480
            ld (_ramsize), hl
            ld hl, #(480-48)		; 64K for kernel
            ld (_procmem), hl

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

	    ; Program the video engine
	    call vdpinit

	    ; FIXME: set up interrupt timer on the CTC
	    ; 08 is channel 0, which is input for vdp
            ; 09 is channel 1, output for DART ser 0 } fed 4MHz/13
            ; 0A is channel 2, output for DATA ser 1 }
	    ; 0B is channel 3, counting CSTTE edges (cpu clocks) at 4MHz

	    xor a
	    out (0x08), a		; vector 0
	    ld a, #0xA5
	    out (0x08), a		; CTC 0 as our IRQ source
	    ld a, #0xFC
	    out (0x08), a		; Timer constant

            im 1 ; set CPU interrupt mode

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
	    push af
	    ld a, #0x80		; ROM off bank 0
	    ; the map port is write only, so keep a local stash
	    ld (map_copy), a
	    out (0), a
	    pop af
            ret
map_process:
	    ld a, h
	    or l
	    jr z, map_kernel
	    ld a, (hl)
map_process_a:
	    ld (map_copy), a
	    out (0), a
	    ret
map_process_always:
	    push af
	    ld a, (U_DATA__U_PAGE)
	    ld (map_copy), a
	    out (0), a
	    pop af
	    ret
map_save:
	    push af
	    ld a, (map_copy)
	    ld (map_store), a
	    pop af
	    ret	    
map_restore:
	    push af
	    ld a, (map_store)
	    out (0), a
	    pop af
	    ret	    
map_store:
	    .db 0
map_copy:
	    .db 0
_kernel_flag:
	    .db 1

; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
	    out (0x0c), a
            ret
