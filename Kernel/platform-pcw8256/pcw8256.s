;
;	PCW8256-9512+ support
;

            .module pcw8256

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl _program_vectors
	    .globl map_kernel
	    .globl map_process
	    .globl map_process_always
	    .globl _kernel_flag
	    .globl map_save
	    .globl map_restore
	    .globl platform_interrupt_all

            ; exported debugging tools
            .globl _trap_monitor
            .globl _trap_reboot
            .globl outchar
	    .globl _bugout

	    ; exported video symbols
	    .globl _scroll_up
	    .globl _scroll_down
	    .globl _cursor_on
	    .globl _cursor_off
	    .globl _plot_char
	    .globl _do_beep
	    .globl _clear_lines
	    .globl _clear_across

            ; imported symbols
            .globl _ramsize
            .globl _procmem
	    .globl nmi_handler
	    .globl null_handler
            .globl interrupt_handler
            .globl unix_syscall_entry
	    .globl _vtinit

	    ; debug symbols
            .globl outcharhex
            .globl outhl, outde, outbc
            .globl outnewline
            .globl outstring
            .globl outstringhex

            .include "kernel.def"
            .include "../kernel.def"

; -----------------------------------------------------------------------------
; VIDEO MEMORY BANK (0x4000-0xBFFF during video work)
; -----------------------------------------------------------------------------

framebuffer .equ	0x4000
font8x8	    .equ	0x9C00		; font loaded after framebuffer

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

trapmsg:    .ascii "Trapdoor: SP="
            .db 0
trapmsg2:   .ascii ", PC="
            .db 0
tm_user_sp: .dw 0

tm_stack:
            .db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
tm_stack_top:

;
;	Ask the controller to reboot
;
_trap_reboot:
	    ld a, #0x01
	    out (0xF8), a
            ; should never get here
_trap_monitor:
	    di
	    halt
	    jr _trap_monitor

platform_interrupt_all:
	    ret

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xF000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
	    ld b, #'U'
	    call _bugoutv
	    call _vtinit
            ret

init_hardware:
            ; set system RAM size
	    ld b, #'Z'
	    call _bugoutv
            ld hl, #256
            ld (_ramsize), hl
            ld hl, #(256-64)		; 64K for kernel
            ld (_procmem), hl

	    ; FIXME 100Hz timer on

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

            im 1 ; set CPU interrupt mode
	    ld b, #'I'
	    call _bugoutv
            ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

_kernel_flag:
	    .db 1

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
            ld hl, #nmi_handler
            ld (0x0067), hl

            ; put the MMU back as it was -- we're in kernel mode so this is predictable
	    call map_kernel
	    ret
;
;	We must provide
;
;	map_kernel		-	map in the kernel, trashes nothing
;	map_process_always	-	map in the current process, ditto
;	map_process		-	map the pages pointed to by hl, eats
;					a, hl
;
kmap:	    .db 0x80, 0x81, 0x82, 0x83

map_kernel:
	    push af
	    push hl
	    ld hl, #kmap
	    call map_process_1
            pop hl
	    pop af
	    ret

map_process_always:
	    push af
	    push hl
	    ld hl, #U_DATA__U_PAGE
	    call map_process_1
	    pop hl
	    pop af
	    ret

map_process:
	    ld a, h
	    or l
	    jr z, map_kernel
map_process_1:
	    ld a, i
	    push af
	    di			; ensure we don't take an irq mid update
	    push de
	    push bc
	    ld de, #map_current
	    ld bc, #0x3F0	; 3 loops starting 0xf0
				; we don't touch common in these functions
map_loop:
	    ld a, (hl)
	    ld (de), a
	    out (c), a
	    inc hl
	    inc de
	    inc c
	    djnz map_loop
	    pop bc
	    pop de
	    pop af
	    ret po
	    ei
	    ret

map_save:   push hl
	    push de
	    push bc
	    ld hl, #map_current
	    ld de, #map_save_area
	    ldi
	    ldi
	    ldi
	    pop bc
	    pop de
	    pop hl
	    ret

map_restore:push hl
	    push af
	    ld hl, #map_save_area
            call map_process_1
	    pop af
            pop hl
            ret
;
;	These are in common, that means that on a system that switches
; common by task there are multiple copies of this information.
;
; Safe IFF we always reload the *full* map when task switching (we do)
;
map_current:
	    .db 0		; need this tracked
	    .db 0		; hardware ports are write only
	    .db 0
; Safe because we never task switch from an IRQ while in kernel mode.
; In user mode we won't restore the saved area anyway
map_save_area:
	    .db 0
	    .db 0
	    .db 0


_bugout:    pop hl
	    pop bc
	    push bc
	    push hl
	    ld b, c
_bugoutv:
	    ld a, #0x20
	    .dw 0xfeed
	    ret
; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
;
outchar:    push bc
	    ld b, a
	    ld a, #0x20
	    .dw 0xfeed
	    pop bc
	    ret


; FIXME: serial version below
	    push af
outcharl:
	    xor a
	    out (0xE1), a
	    in a, (0xE1)
	    bit 2, a
	    jr z, outcharl
	    pop af
	    out (0xE0), a
	    ret

	    .area _CODE

;
; Video helpers. Video is in banks 4/5 for now
;

_scroll_up:
	    ld a, (roller)
	    add a, #8
	    ld (roller), a
            out (0xf6), a
	    ret
_scroll_down:
	    ld a, (roller)
	    sub a, #8
	    ld (roller), a
            out (0xf6), a
	    ret

	    .area _COMMONMEM

addr_de:    ld a, i
	    push af
	    ld a, (roller)
	    rra			; in text lines
	    rra
	    rra
	    and #0x1F
	    ld l, a
	    ld a, e		; Y
            add l		; plus the roller
	    and #31		; wrap
	    ; Y * 720
	    add a		; x 2
	    add a		; x 4
	    add a		; x 8 ( 31 x 8 fits in 8bits)
	    push bc
	    ld l, a
	    ld h, #0
	    add hl, hl		; x 16
	    push hl
	    add hl, hl		; x 32
	    add hl, hl		; x 64
	    push hl
	    add hl, hl		; x 128
	    push hl
	    add hl, hl		; x 256
	    add hl, hl		; x 512
	    pop bc
	    add hl, bc		; x 640
	    pop bc
	    add hl, bc		; x 704
	    pop bc
	    add hl, bc		; x 720
	    ex de, hl
	    ld l, h
	    ld h, #0
	    add hl, hl
	    add hl, hl
	    add hl, hl		; X * 8
	    add hl, de
	    ld de, #framebuffer	; the bank base we are using
	    add hl, de
	    ex de, hl
	    ;
	    ; We don't want to take an interrupt midway through
	    ; this lot, or the restore will be of the wrong values
	    ;
	    di
	    ld a, #0x84		; Map the video memory
	    ld (map_current + 1), a
	    inc a
	    ld (map_current + 2), a
	    out (0xf2), a
	    dec a
	    out (0xf1), a
	    pop bc
	    pop af
	    ret po
	    ei
	    ret
_plot_char:
	    pop hl
	    pop de	; d, e = co-ords
	    pop bc      ; c = char
	    push bc
	    push de
	    push hl
	    call addr_de	;  returns an address in DE and maps the vram
	    ld l, c
	    ld h, #0
	    add hl, hl
	    add hl, hl
	    add hl, hl
 	    ld bc, #font8x8	;  where crt0.s stuck the font
	    add hl, bc
	    ld bc, #8
	    ldir
	    call map_kernel
	    ret
_cursor_on:
	    pop hl
	    pop de
	    push de
	    push hl
cursordo:
	    ld (cursorpos), de
	    call addr_de
	    ex de, hl
            ld b, #8
cursorl:    ld a, (hl)
	    cpl
	    ld (hl), a
	    inc hl
	    djnz cursorl
	    call map_kernel
	    ret
_cursor_off:
	    ld de, (cursorpos)
	    jr cursordo

_do_beep:
	    ret

_clear_lines:
	    pop hl
	    pop de	; E = y, D = count
	    push de
	    push hl
	    ld b, d
	    ld d, #0
clloop:     push de
	    push bc
	    call addr_de
	    ld h, d
	    ld l, e
	    ld (hl), #0
	    inc de
	    ld bc, #719
	    ldir
	    pop bc
	    pop de
	    inc e
	    call map_kernel
	    djnz clloop
	    ret

_clear_across:
	    pop hl
	    pop de	; co-ordinates
	    pop bc	; count in C
	    push bc
	    push de
	    push hl
	    call addr_de
	    ld b, #8
	    xor a
	    ex de, hl
clearal:
	    ld (hl), a
	    inc hl
	    djnz clearal
	    dec c
	    jr nz, clearal
	    call map_kernel
	    ret

;
;	Need to live outside of common/code. Take care not to access them
;	with the video bank mapped
;
	    .area _DATA

roller:	    .db 0
cursorpos:  .dw 0
