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
	    .globl map_buffers
	    .globl map_kernel_di
	    .globl map_process
	    .globl map_process_di
	    .globl map_process_a
	    .globl map_process_always
	    .globl map_process_always_di
	    .globl map_save_kernel
	    .globl map_restore
	    .globl map_for_swap
	    .globl _kernel_map

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
	    .globl _udata

	    .globl unix_syscall_entry
            .globl null_handler
	    .globl nmi_handler
            .globl interrupt_handler

	    .globl vdpinit
	    .globl vdpload
	    .globl _vtinit
	    .globl _probe_prop
	    .globl _has_6845
	    .globl _has_rememo
	    .globl _curtty
	    .globl _vdptype
	    .globl _vdpport

            .globl outcharhex
            .globl outhl, outde, outbc
            .globl outnewline
            .globl outstring
            .globl outstringhex

            .include "kernel.def"
            .include "../kernel-z80.def"

;
;	Disk buffers
;
	    .globl _bufpool
	    .area _BUFFERS

_bufpool:
	    .ds BUFSIZE * NBUFS


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
bogus_int:  ei
	    reti
serial_int:
	    push af
	    ld a, #1
	    ld (_irqvector), a
	    pop af
	    jp interrupt_handler

; We always have 0x80 so we just need to count the banks that differ
; If we have partial banks they appear from 0 but we can't use them.
; Check in the 8000-BFFF range therefore.
;
; We use BFFF as this is harmlessly in kernel data space for the kernel
; bank.
size_ram:
	    ld hl, #0xBFFF	; clear of anything we need
	    ld a,(hl)
	    push af		; save kernel BFFF
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
	    ld a, #0x90
	    cp b
	    jr nz, size_next	; All banks done
size_nonram:
	    ld a, (_kernel_map)
	    out (c), a		; Return to kernel
	    res 7,b		; Clear the flag so we just have banks
	    ld a, b
	    ld (_membanks), a	; Number of banks (0 - membanks - 1)
	    or a
	    jr z,nobanks
	    ld hl, #0
	    ld de, #48
	    dec b
	    jr z, nobanks
sizer:	    add hl, de
	    djnz sizer
nobanks:
	    pop af
	    ld (0xBFFF),a
	    ret

find_my_ram:
	    ld a,#0x80
test_bank:
	    out (0),a
	    ld hl,#crtcmap
	    ex af,af
	    ld a,(hl)
	    cp #0x77
	    jr nz,nope
	    inc hl
	    ld a,(hl)
            cp #0x50
	    jr nz,nope
            inc hl
	    ld a,(hl)
	    cp #0x5C
            jr z,found_self
nope:	    ex af,af
	    inc a
	    cp #0x90
	    jr nz, test_bank
            jp _platform_reboot

found_self:
	    ex af,af
	    out (0),a
	    ld (_kernel_map),a
	    ret

find_rememorizer:
	    ld a,#0x8F
	    out (0),a
	    xor a
	    out (0xD0),a	; 0x4000 should now be 0xC000 on a
				; rememorizer only
	    ld hl,(intvectors-0x8000)
	    ld de,(intvectors)
	    or a
	    sbc hl,de
	    ld a,#0
	    jr nz,not_rememo
	    inc a
not_rememo:
	    ld c,a		; save the rememorizer state
	    ld a,(_kernel_map)
	    out (0),a
	    ; memory back so we can now write the state
	    ld a,c
	    out (0xD0),a
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
;	Will effectively be a no-op if we then turn out to have a prop
;	based board
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
	    ; Task 1. Find my bank
	    call find_my_ram
	    ; Count banks
	    call size_ram
            ld (_procmem), hl
	    ld de,#64			; common memory and kernel
	    add hl,de
	    ld (_ramsize),hl

	    ; We can now check for a Rememorizer. If we have one then we can
	    ; do extra games with the memory banking
	    call find_rememorizer
            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

	    ; Program the video engine

	    ld bc,(_vdpport)
	    ; Play with status register 2
	    ld a,#0x8F
	    out (c),a
	    nop
	    nop
	    in a,(c)
	    ld a,#2
	    out (c),a
	    ld a,#0x8F
	    out (c),a
	    nop
	    nop
vdpwait:    in a,(c)
	    and #0xE0
	    add a
	    jr nz, not9918a	; bit 5 or 6
	    jr nc, vdpwait	; not bit 7
	    ; we vblanked out - TMS9918A
	    xor a
	    ld (_vdptype),a
	    jr vdp_setup
not9918a:   ; Use the version register
	    ld a,#1
	    out (c),a
	    ld a,#0x8F
	    out (c),a
	    nop
	    nop
	    in a,(c)
	    rrca
	    and #0x1F
	    inc a
	    ld (_vdptype),a

vdp_setup:
	    call vdpinit
	    call vdpload

	    ; 08 is channel 0, which is input from VDP
            ; 09 is channel 1, output for DART ser 0 } fed 4MHz/13
            ; 0A is channel 2, output for DATA ser 1 }
	    ; 0B is channel 3, counting CSTTE edges (cpu clocks) at 4MHz

	    ld a,#3
	    out (0x08),a
	    out (0x09),a
	    out (0x0A),a
	    out (0x0B),a
	    xor a
	    out (0x08), a		; vector 0
	    ld a, #0xA5
	    out (0x08), a		; CTC 0 as our IRQ source
	    ld a, #0xFA			; 250 - 62.5Hz
	    out (0x08), a		; Timer constant

	    ld hl, #intvectors		; Work around SDCC crappiness
	    ld a, h
	    ld i, a
            im 2 ; set CPU interrupt mode

	    call _probe_6845		; look for 80 column video
	    call _probe_prop		; see if we have CFII prop video
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

;
; Helper for the SN76489. We need to keep thee 32 clocks apart and that's
; hard to do in C but easy if it has to call a tiny fastcall helper
;
; From the strobe we have a minimum of
; ret  			10
; ld l,r		4
; call xxxx		17
; ld a,l		4
;
; so we are safe.
;
;
	    .globl _sn76489_write

_sn76489_write:
	    ld a,l
	    out (0x06),a
	    in a,(0x03)
	    ret

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
map_buffers:
map_kernel_di:
	    push af
	    ld a, (_kernel_map)	; ROM off bank for kernel
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
map_for_swap:
map_process_a:
	    ld (map_copy), a
	    out (0), a
	    ret
map_process_always:
map_process_always_di:
	    push af
	    ld a, (_udata + U_DATA__U_PAGE)
map_set_a:
	    ld (map_copy), a
	    out (0), a
	    pop af
	    ret
map_save_kernel:
	    push af
	    ld a, (map_copy)
	    ld (map_store), a
	    ld a, (_kernel_map)	; ROM off bank for kernel
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
_kernel_map:
	    .db 0

; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
	    out (0x60), a
	    out (0x0c), a
            ret

	    .area _DISCARD
_probe_6845:
	    ld a,#' '
	    out (0x32),a
	    ld a,#0x07
	    out (0x33),a
	    ld a,#0xE0
	    out (0x31),a
	    xor a
	    out (0x30),a
	    ; Now read back
	    out (0x31),a
	    ld a,#0x20
	    out (0x30),a
	    in a,(0x32)
	    cp #0x20
	    ret nz
	    ld (_has_6845),a
	    xor a
	    ld (_curtty),a
	    ret
