;
;	PX4 Plus hardware support
;

            .module px4plus

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl _program_vectors

	    .globl map_kernel
	    .globl map_process
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore
	    .globl map_process_save
	    .globl map_kernel_restore
	    .globl _sidecar
	    .globl _carttype
	    .globl _need_resched

	    .globl plt_interrupt_all

            ; exported debugging tools
            .globl _plt_monitor
            .globl _plt_reboot
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem
	    .globl _vtinit
	    .globl _cartridge_size

	    .globl unix_syscall_entry
            .globl null_handler
	    .globl nmi_handler
            .globl interrupt_handler

	    ; ROM interfaces
	    .globl _rom_sidecar_read
	    .globl _rom_cartridge_read
	    .globl _romd_off
	    .globl _romd_size
	    .globl _romd_addr
	    .globl _romd_mode


	    .globl __bank_0_1
	    .globl __bank_0_3
	    .globl __bank_1_3
	    .globl __bank_3_1

	    .globl __stub_0_1
	    .globl __stub_0_3
	    .globl __stub_1_3
	    .globl __stub_3_1


            .include "kernel.def"
            .include "../kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (not unmapped when we flip to ROM)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

; FIXME: figure out how to reboot into CP/M
_plt_monitor:
_plt_reboot:
	    di
	    halt
plt_interrupt_all:
	    ret

;
;	FIXME: this probably needs to be a new "commondata" area so we can
;	ROM this correctly
;
_need_resched:
	    .db 0
kernel_map:			; Last kernel map we were using
	    .db 0xA2
saved_map:			; Saved mapping for IRQ entry/exit
	    .db 0
_sidecar:
	    .db 0
_carttype:
	    .db 0
_romd_off:
	    .dw 0
_romd_size:
	    .dw 0
_romd_addr:
	    .dw 0
_romd_mode:
	    .db 0

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
	    ld a, (0xF53F)		; BIOS cartridge type
	    ld (_carttype), a
            ret

init_hardware:
	    ; FIXME: set video base and display properties first
	    ld a, #VIDBASE
	    out (0x08), a		; Video at 0xF800
	    ; Reset hardware scrolling
	    ld a, #0x80			; On, Y offset 0
	    out (0x09), a

            ; set system RAM size
	    ; We have 64K RAM + 64K ROM to immediate hand, but we should
	    ; try and include the "swap" like RAM here to give a sensible
	    ; number, as we won't treat it as disk
            ld hl, #64
	    in a, (0x94)
	    add a
	    jr nc, notplus
	    ld hl, #192			; Sidecar always has 128K on the PX4
					; PX8 version is 120K if we ever do
					; PX8
	    ld a, #1
	    ld (_sidecar), a		; Sidecar present
notplus:    ; Not a PX4plus or PX4 with sidecar
	    ; See if we have a RAM cartridge fitted
	    ld a, (_carttype)
	    cp #2
	    jr nz, noramcart
	    push hl
	    push af
	    call _cartridge_size
	    pop af
	    pop de
	    add hl, de
noramcart:
            ld (_ramsize), hl
	    or a
            ld de, #32			; 32K for kernel
	    sbc hl, de
            ld (_procmem), hl

	    ; We don't bank 0x00-0xFF so we can do the vectors once at boot
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

	    ; IRQ enables
	    ld a, #0xB		; OVF (timer), RXRDY (gapnio), 7508
	    out (0x04), a

	    push af
	    call _vtinit
	    pop af
            im 1 ; set CPU interrupt mode
            ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM


_program_vectors:
	    ret


map_kernel: 
map_kernel_restore:
	    push af
	    ld a, (kernel_map)
	    out (0x05), a
	    pop af
	    ret
map_process:
	    ld a, h
	    or l
	    jr z, map_kernel
map_process_always:
map_process_save:
	    push af
	    in a,(0x05)
	    cp #0x42			; Already in user map
	    jr z, reto
	    ld (kernel_map), a
            ld a, #0x42			; user map (ROMs out)
	    out (0x05), a
reto:
	    pop af
	    ret

map_save:
	    in a, (0x05)
	    ld (saved_map), a
	    ret
map_restore:
	    ld a, (saved_map)
	    and #0xF0
	    or #0x02
	    out (0x05), a
	    ret

; outchar: Wait for UART TX idle, then print the char in A
outchar:
	    push af
outcharw:
	    in a, (0x15)
	    bit 0, a
	    jr z, outcharw
	    pop af
	    out (0x14), a	 
            ret


;
;
;	Banking helpers
;
;	Logical		Physical
;	0		COMMON
;	1		ROM1
;	2		*unused*
;	3		ROM2
;
;
__bank_0_1:
	ld a, #0xA2	   ; switch to 32K ROM 1
bankina0:
	;
	;	Get the target address first, otherwise we will change
	;	bank and read it from the wrong spot!
	;
	pop hl		   ; Return address (points to true function address)
	ld e, (hl)	   ; DE = function to call
	inc hl
	ld d, (hl)
	inc hl
	push hl		   ; Restore corrected return pointer
	ld c, #0x05
	in b, (c)
	out (c), a
	; figure out which bank to map on the return path
	ld a, #0xA2
	cp b
	jr z, __retmap1
	jr __retmap3

callhl:	jp (hl)

__bank_0_3:
	ld a, #0xE2	   ; 32K ROM 2
	jr bankina0

__bank_1_3:
	ld a, #0xE2	   ; 32K ROM 2
	pop hl		   ; Return address (points to true function address)
	ld e, (hl)	   ; DE = function to call
	inc hl
	ld d, (hl)
	inc hl
	push hl		   ; Restore corrected return pointer
	out (0x05), a
__retmap1:
	ex de, hl
	call callhl	   ; call the function
	ld a,#0xA2	   ; return to ROM1
	out (0x05), a
	ret

__bank_3_1:
	ld a,#0xA2	   ; 32K ROM 1
	pop hl		   ; Return address (points to true function address)
	ld e, (hl)	   ; DE = function to call
	inc hl
	ld d, (hl)
	inc hl
	push hl		   ; Restore corrected return pointer
	out (0x05), a
__retmap3:
	ex de, hl
	call callhl	   ; call the function
	ld a, #0xE2	   ; return to ROM 2
	out (0x05), a
	ret

;
;	Stubs need some stack munging and use DE
;
__stub_0_1:
	ld a, #0xA2
__stub_0_a:
	pop hl		; the return
	ex (sp), hl	; write it over the discard
	ld c, #0x05
	in b, (c)
	out (c), a
	ld a, #0xA2
	cp b
	jr z, __stub_1_ret
	jr __stub_3_ret
__stub_0_3:
	ld a, #0xE2
	jr __stub_0_a

__stub_1_3:
	ld a, #0xE2
	pop hl		; the return
	ex (sp), hl	; write it over the discad
	out (0x05), a
__stub_1_ret:
	ex de, hl
	call callhl
	ld a, #0xA2
	out (0x05), a
	pop de
	push de		; dummy the caller will discard
	push de
	ret

__stub_3_1:
	ld a, #0xA2
	pop hl		; the return
	ex (sp), hl	; write it over the discad
	out (0x05), a
__stub_3_ret:
	ex de, hl
	call callhl
	ld a,#0xE2
	out (0x05), a
	pop de
	push de		; dummy the caller will discard
	push de
	ret
