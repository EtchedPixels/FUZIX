; This is a mix of code taken from FUZIX's z80pak.s and sbcv2.s -jcw
; For use with the EZ-Retro v2 board, see https://docs.jeelabs.org/projects/ezr/
;
; Original & modified code follows, in messy / not-cleaned-up state for now.
;
;	EZ Retro v2 support
;


            .module ezretro

	    .ez80
	    .adl 0

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl _program_vectors
	    .globl plt_interrupt_all

	    .globl map_kernel
	    .globl map_kernel_restore
	    .globl map_kernel_di
	    .globl map_proc
	    .globl map_proc_di
	    .globl map_proc_always
	    .globl map_proc_always_di
	    .globl map_proc_a
	    .globl map_save_kernel
	    .globl map_restore

	    .globl _fd_cmd

	    .globl _int_disabled

	    .globl _plt_reboot

            ; exported debugging tools
            .globl _plt_monitor
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem

	    .globl unix_syscall_entry
            .globl null_handler
	    .globl nmi_handler
            .globl interrupt_handler

            .include "kernel.def"
            .include "../../cpu-z80/kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xE000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_plt_monitor:
_plt_reboot: ; FIXME    out (0), a ; exit emulator
	    di
            jr _plt_reboot

plt_interrupt_all:
	    ret

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xC000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
        ;xor a
        ;; out0 (RAM_BANK),a		(set SRAM to bank 0)
        ;.db 0xED,0x39,0xB5
            ret

init_hardware:
            ; initialise UART0 to 9600 baud
            ld hl,#0x0380
            ld de,#0x1A06

	    out0 (0xA5),h
	    out0 (0xC3),l
	    out0 (0xC0),d
	    out0 (0xC3),h
	    out0 (0xC2),e
;            .db 0xED,0x21,0xA5        ; out0 (0A5h),h = 03h
;            .db 0xED,0x29,0xC3        ; out0 (0C3h),l = 80h
;            .db 0xED,0x11,0xC0        ; out0 (0C0h),d = 1Ah
;            .db 0xED,0x21,0xC3        ; out0 (0C3h),h = 03h
;            .db 0xED,0x19,0xC2        ; out0 (0C2h),e = 06h

            ; set system RAM size
            ld hl, #512
            ld (_ramsize), hl
            ld hl, #(512-64)		; 64K for kernel
            ld (_procmem), hl

            ; set up interrupt vectors for the kernel
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

            ret

; this code runs in 24-bit ADL mode and must be placed outside SRAM
; FIXME this doesn't handle interrupts in ADL mode, avoid interrupts for now
seladl:     ; out0 (RAM_BANK),a		(move SRAM to selected bank)
            .db 0xED,0x39,0xB5
    	    ; ld  mb,a			(set MBASE to selected bank)
            .db 0xED,0x6D	
            ; jp.sis selret             (exit ADL mode)
            .db 0x40,0xC3 
            .dw selret

; FLOPPY DISK emulation, using a single 1536 KB RAM disk at 0x080000..0x1FFFFF
; this can handle any bank, thanks to eZ80's 24-bit "ldir.l" instruction
; there's no bank switching (and no need to disable interrupts, I think)
;
_fd_cmd:
        pop de		; return
        pop hl		; descriptor address
        push hl
        push de		; fix stack

        ld a,(hl)              ; dma l
        ld (dmaadr),a
        inc hl
        ld a,(hl)              ; dma h
        ld (dmaadr+1),a
        inc hl
        ld a,(hl)              ; bank
        ld (dmaadr+2),a
        inc hl
        ;ld a,(hl)              ; drive, ignored
        ;inc hl
        ld c,(hl)              ; cmd
        inc hl
        ld e,(hl)              ; sector l
        inc hl
        ld d,(hl)              ; sector h
        ld h,d
        ld l,e
        add hl,de ; now it's a 512-byte multiple (top 2 bytes)
        ld de,#0x0800
        add hl,de ; now it's an offset in the ram disk (top 2 bytes)
        ld (dskadr+1),hl

    ;ld a,#'@'
    ;add a,h
    ;call outchar
    ;ld a,#'@'
    ;add a,l
    ;call outchar

        ld a,c
        and #0x80
        jr nz,dwrite

dread:  .db 0x5B,0x2A                   ; ld.lil hl,({0,dskadr})
	.dw dskadr
	.db 0
	.db 0x5B,0xED,0x5B              ; ld.lil de,({0,dmaadr})
	.dw dmaadr
	.db 0
    ;ld a,#'r'
    ;call outchar
	jr drwop

dwrite: .db 0x5B,0x2A                   ; ld.lil hl,({0,dmaadr})
	.dw dmaadr
	.db 0
	.db 0x5B,0xED,0x5B              ; ld.lil de,({0,dskadr})
	.dw dskadr
	.db 0
    ;ld a,#'w'
    ;call outchar

drwop:	.db 0x5B,0x01,0x00,0x02,0x00    ; ld.lil bc,000200h
	.db 0x49,0xED,0xB0              ; ldir.l

	xor a ; TODO this driver never reports any errors
        ld l,a
        ld h,a
	ret

dmaadr: .db 0,0,0   ; last dma address + bank
dskadr: .db 0,0,0   ; disk address + bank

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

_int_disabled:
	    .byte 1

_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
            di ; just to be sure
            pop de ; temporarily store return address
            pop hl ; function argument -- base page number
            push hl ; put stack back as it was
            push de

	    call map_proc

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

            ; set restart vector for FUZIX system calls
            ld (0x0030), a   ;  (rst 30h is unix function call vector)
            ld hl, #unix_syscall_entry
            ld (0x0031), hl

            ld (0x0000), a   
            ld hl, #null_handler   ;   to Our Trap Handler
            ld (0x0001), hl

            ld (0x0066), a  ; Set vector for NMI
            ld hl, #nmi_handler
            ld (0x0067), hl

	    ; and fall into map_kernel

map_kernel:
map_kernel_di:
map_kernel_restore:
	    push af
	    xor a
	    call selmem
	    pop af
            ret

map_proc:
map_proc_di:
	    ld a, h
	    or l
	    jr z, map_kernel
	    ld a, (hl)
map_proc_a:
; the actual bank-switching code is in low memory, since SRAM is being moved
selmem:     ; jp.lil {0,seladl}		(enter ADL mode)
            .db 0x5B,0xC3
            .dw seladl
            .db 0x00
selret:     ret ; control returns here once SRAM and MBASE have been adjusted

map_proc_always:
map_proc_always_di:
	    push af
	    ld a, (_udata + U_DATA__U_PAGE)
	    call selmem
	    pop af
	    ret

map_save_kernel:
	    push af
            .db 0xED,0x38,0xB5 	; in0 a,(RAM_BANK)
	    ld (mapsave), a
	    xor a
	    call selmem
	    pop af
	    ret	    

map_restore:
	    push af
	    ld a, (mapsave)
	    call selmem
	    pop af
	    ret	    

mapsave:    .db 0

; outchar: Wait for UART TX idle, then print the char in A
outchar:    push af
out1:       .db 0xED,0x38,0xC5 	; in0 a,(0C5h)
            and #0x20
            jr z,out1
            pop af
            .db 0xED,0x39,0xC0 	; out0 (0C0h),a
            ret
