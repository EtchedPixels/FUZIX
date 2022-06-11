; This is a mix of code taken from FUZIX's z80pak.s and sbcv2.s -jcw
; For use with µC-based emulator, see https://jeelabs.org/2018/z80-zexall-f407/
;
; All IN instructions are treated as "system calls" and can modify all CPU
; state, i.e. reading as well as writing any of its registers (beware!).
; For details, see the switch statement of systemCall() in this source file:
;  https://git.jeelabs.org/jcw/retro/src/branch/master/native-fuzix/src/main.cpp
; Also, any OUT instruction is treated as a request to exit the emulator.
;
; Original & modified code follows, in messy / not-cleaned-up state for now.
;
;	JeeLabs Retro emulator support
;


            .module jeeretro

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl _program_vectors
	    .globl plt_interrupt_all

	    .globl map_kernel
	    .globl map_kernel_di
	    .globl map_kernel_restore
	    .globl map_process
	    .globl map_process_di
	    .globl map_process_always
	    .globl map_process_always_di
	    .globl map_process_a
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
            .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_plt_monitor:
_plt_reboot:
	    out (0), a ; exit emulator
plt_interrupt_all:
	    ret

;
;	We need the right bank present when we perform the transfer
;
_fd_cmd:    pop de		; return
	    pop hl		; descriptor address
	    push hl
	    push de		; fix stack
	    ld a, (hl)              ; bank
            inc hl
            ld c, (hl)              ; drive
            inc hl
            ld b, (hl)              ; cmd
            inc hl
            ld e, (hl)              ; dma l
            inc hl
            ld d, (hl)              ; dma h
            push de
            inc hl
            ld e, (hl)              ; sector l
            inc hl
            ld d, (hl)              ; sector h
            pop hl
            ; all of the above is with kernel bank still active

	    di                  ; no interrupts during bank switch
	    in a, (7)           ; bank now switched
            push af             ; save previous bank
            ld a, c                 ; drive
	    in a, (4)           ; perform disk transfer
	    pop af             	; restore bank
	    in a, (7)           ; bank now restored
	    ld a, (_int_disabled) ; restore interrupt enable/disable
	    or a
	    ret nz
	    ei
	    ret

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xC000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
	    ld a, #240			; 240 * 256 bytes (60K)
	    in a, (6)			; set up memory banking
            ret

init_hardware:
            ; set system RAM size
            ld hl, #184
            ld (_ramsize), hl
            ld hl, #(184-64)		; 64K for kernel
            ld (_procmem), hl

	    ;ld a, #1
	    ;out (27), a			; 100Hz timer on XXX

            ; set up interrupt vectors for the kernel
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

            im 1 ; set CPU interrupt mode

            ret


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
	    in a, (7)
	    pop af
            ret
map_process:
map_process_di:
	    ld a, h
	    or l
	    jr z, map_kernel
	    ld a, (hl)
map_process_a:
            push af
	    in a, (7)
            pop af
            ret
map_process_always:
map_process_always_di:
	    push af
	    ld a, (_udata + U_DATA__U_PAGE)
	    in a, (7)
	    pop af
	    ret
map_save_kernel:
	    push af
	    xor a
	    in a, (7)
	    ld (mapsave), a
	    pop af
	    ret	    
map_restore:
	    push af
	    ld a, (mapsave)
	    in a, (7)
	    pop af
	    ret	    
mapsave:
	    .db 0

; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
            push bc
            ld c,a
	    in a, (2)
            pop bc
            ret
