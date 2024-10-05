;
;	    TRS 80  hardware support
;

            .module trs80

            ; exported symbols
            .globl init_early
            .globl interrupt_handler
            .globl _program_vectors
	    .globl plt_interrupt_all
	    .globl map_kernel
	    .globl map_proc
	    .globl map_proc_a
	    .globl map_proc_always
	    .globl map_save_kernel
	    .globl map_restore
	    .globl map_kernel_restore

	    .globl go_fast
	    .globl go_slow

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

	    .globl _trs80_model
	    .globl _int_disabled

	    ; hard disk helpers
	    .globl _hd_xfer_in
	    .globl _hd_xfer_out
	    ; and the page from the C code
	    .globl _hd_page

            ; exported debugging tools
            .globl _plt_monitor
            .globl _plt_reboot
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl istack_top
            .globl istack_switched_sp
            .globl unix_syscall_entry
            .globl outcharhex
	    .globl null_handler
	    .globl fd_nmi_handler
	    .globl _vtflush
	    .globl mapper_init

            .include "kernel.def"
            .include "../../cpu-z80/kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0x4000 upwards after the udata etc)
; -----------------------------------------------------------------------------
            .area _COMMONDATA
;
;	This is linked first after udata and boot, and we turn the boot area
;	into the istack. Don't screw around with the link order!
;
istack_top:
istack_switched_sp: .dw 0

	    .area _COMMONMEM

_plt_monitor:
	    push af
	    call _vtflush		; get any panic onscreen
	    pop af
monitor_spin:
	    di
	    jr monitor_spin

plt_interrupt_all:
	    ret

_plt_reboot:
	    di
	    ld sp,#0xffff
	    xor a
	    out (0x43),a
	    rst 0

_int_disabled:
	    .db 1

; -----------------------------------------------------------------------------
; BOOT MEMORY BANK (below 0x8000)
; -----------------------------------------------------------------------------

            .area _BOOT


init_early:
	    call mapper_init	; A is the mapper type passed from crt0.s
	    ld a,(4)
	    cp #0x30
	    jr nz, not_m3
	    ld a,#1
	    ld (_trs80_model),a
	    ld a,#0x74
	    out (0xE0),a	; Mask iobus, cassette
	    xor a
	    out (0xE4),a	; and NMI sources
	    jr not_vg
not_m3:
	    ; Detect machine type (Model 1 or LNW80 or VideoGenie ?)
	    ld a,#8
	    out (0xFE),a	; turn off ROM on the LNW80
	    ld hl,#0
	    ld a,(hl)
	    inc (hl)
	    cp (hl)
	    jr z, not_lnw	; if it's RAM it's an LNW80
	    dec (hl)
	    xor a
	    out (0xFE),a	; ROM back on, normal video mode for now
	    ld a,#2		; LNW80
	    ld (_trs80_model), a
	    jr not_vg
not_lnw:
	    ld hl,(0x18F5)
	    ld de,#0x4E53	; 'SN' for VG, 'L3' for TRS80 Model 1
	    or a
	    sbc hl,de
	    jr nz, not_vg
	    ld a,#3
	    ld (_trs80_model),a	; Video Genie
not_vg:
	    jp _rom_vectors

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM
;
;	TRS80 speed control. Save anything used except AF
;	The LNW80 does automatic slowing on floppy disk access
;	The Model 3 sprinter type card uses port 95 the same way but
;	also does automatic slow down when needed.
;
;	Only allowed to mess with AF
;
go_slow:
	    ld a,(_trs80_model)
	    or a
	    jr nz, snot_model_1
	    ; A = 0
	    out (254),a
	    ret
snot_model_1:
	    cp #1		; model III ?
	    ret nz
	    xor a
	    out (95),a
	    ret
go_fast:
	    ld a,(_trs80_model)
	    or a
	    jr nz,fnot_model_1
	    ; A = 0
	    inc a
	    out (254),a
	    ret
fnot_model_1:
	    cp #1
	    ret nz
	    ; A = 1
	    out (95),a
	    ret

_program_vectors:
	    ret

_rom_vectors:
	    ld a,#0xC3
	    ld (0x4012), a
            ld hl, #interrupt_handler
            ld (0x4013), hl

            ; set restart vector for UZI system calls
            ld (0x400F), a   ;  (rst 30h is unix function call vector)
            ld hl, #unix_syscall_entry
            ld (0x4010), hl

	    ; Model III only but just writing it does no harm
	    ld hl,#fd_nmi_handler
	    ld (0x404A), hl
	    jp map_kernel
	    
; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
; FIXME: need to do different things for Video Genie and Model III
outchar:
            ld (0x37E8), a
            ret

;
;	Swap helpers.
;	We have our buffers mapped in Bank 2 but we don't need to do
;	anything here as we are in common memory and we've carefully
;	arranged that the device driver callers are in BANK2 thus we'll
;	have BANK2 mapped by default, although we may map a user bank
;	in temporarily if going direct to user.
;
_hd_xfer_in:
	   pop de
	   pop bc
	   pop hl
	   push hl
	   push bc
	   push de
	   ld a, (_hd_page)
	   or a
	   push af
	   call nz, map_proc_a
	   ld bc, #0xC8			; 256 bytes from 0xC8
	   inir
	   pop af
	   call nz, map_kernel_restore
	   ret

_hd_xfer_out:
	   pop de
	   pop bc
	   pop hl
	   push hl
	   push bc
	   push de
	   ld a, (_hd_page)
	   or a
	   push af
	   call nz, map_proc_a
	   ld bc, #0xC8			; 256 bytes to 0xC8
	   otir
	   pop af
	   call nz, map_kernel_restore
	   ret


;
;	Storage for buffers. Must be banked with CODE2
;
	    .area _BUFFERS2
	    .globl _bufdata
	    .globl _bufdata_end
_bufdata:
	    .ds 512 * 5
_bufdata_end:

