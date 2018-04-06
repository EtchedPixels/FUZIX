;
;	Cromemco hardware support
;

            .module cromemco

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl _program_vectors
	    .globl platform_interrupt_all

	    .globl map_kernel
	    .globl map_process
	    .globl map_process_always
	    .globl map_process_a
	    .globl map_save
	    .globl map_restore

	    .globl _platform_reboot

            ; exported debugging tools
            .globl _platform_monitor
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem

	    .globl unix_syscall_entry
            .globl null_handler
	    .globl nmi_handler
            .globl interrupt_handler
	    .globl _doexit
	    .globl _inint
	    .globl kstack_top
	    .globl _panic
	    .globl _need_resched
	    .globl _ssig

            .globl outcharhex
            .globl outhl, outde, outbc
            .globl outnewline
            .globl outstring
            .globl outstringhex

            .include "kernel.def"
            .include "../kernel.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_platform_reboot:
_platform_monitor:
	    jr _platform_monitor
platform_interrupt_all:
	    ret

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xF000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
	    ld a,#0x81			; Every memory is in bank 7
	    out (0x40),a		; MMU set
	    ld hl,#0xF000		; Copy 4K to itself loading
	    ld d,h			; into into the other banks
	    ld e,l
	    ld b,#0x10
	    ld c,l
	    ldir
	    ld a,#0x01			; bank to the kernel bank
	    out (0x40),a
            ret

init_hardware:
            ; set system RAM size
            ld hl, #448
            ld (_ramsize), hl
            ld hl, #(448-64)		; 64K for kernel
            ld (_procmem), hl

	    ld a, #156			; ticks for 10Hz (9984uS per tick)
	    out (8), a			; 10Hz timer on

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

;	    ld a, #0xfe			; Use FEFF (currently free)
;	    ld i, a
;            im 2 ; set CPU interrupt mode
	    im 1			; really should use a page and im2?
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

            ; now install the interrupt vector at 0x38
            ld hl, #interrupt_handler
            ld (0x39), hl

	    ld a,#0xC3		; JP
            ; set restart vector for UZI system calls
            ld (0x0030), a   ;  (rst 30h is unix function call vector)
	    ld (0x0038), a   ;  (rst 38h)
            ld hl, #unix_syscall_entry
            ld (0x0031), hl

            ; now install the interrupt vector at 0x38
            ld hl, #interrupt_handler
            ld (0x39), hl

            ; Set vector for jump to NULL
            ld (0x0000), a   
            ld hl, #null_handler  ;   to Our Trap Handler
            ld (0x0001), hl

            ld (0x0066), a  ; Set vector for NMI
            ld hl, #nmi_handler
            ld (0x0067), hl

	    ; falls through

            ; put the paging back as it was -- we're in kernel mode so this is predictable
map_kernel:
	    push af
	    ld a,#1
	    out (0x40), a
	    ld (map_page), a	; map_page lives in kernel so be careful
	    pop af		; our common is r/o common so writes won't
            ret			; cross a bank
map_process:
	    ld a, h
	    or l
	    jr z, map_kernel
	    ld a, (hl)
map_process_a:
	    ld (map_page),a	; save before we map out kernel
	    out (0x40), a
            ret
map_process_always:
	    push af
	    ld a, (U_DATA__U_PAGE)
	    ld (map_page),a	; save before we map out kernel
	    out (0x40), a
	    pop af
	    ret
map_save:			; this is a bit naughty, but we know
	    push af		; map_save will always be followed by
	    ld a, #1		; map_kernel so we do the map_kernel
	    out (0x40), a	; a shade early.
	    ld a, (map_page)	; then we can get the vars
	    ld (map_store), a
	    pop af
	    ret	    
map_restore:			; called in kernel map
	    push af
	    ld a, (map_store)
	    ld (map_page),a
	    out (0x40), a
	    pop af
	    ret	    
map_store:
	    .db 0
map_page:
	    .db 0

; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
	    push af
outcharl:
	    in a, (0x00)
	    bit 7,a
	    jr z, outcharl
	    pop af
	    out (0x01), a
            ret

;
;	Low level pieces for floppy driver
;
		.globl _fd_reset
		.globl _fd_operation
		.globl _fd_map
		.globl _fd_cmd

_fd_reset:
		ret
_fd_operation:
		ret
_fd_map:
		.byte 0
_fd_cmd:
		.byte 0, 0, 0, 0, 0, 0, 0

