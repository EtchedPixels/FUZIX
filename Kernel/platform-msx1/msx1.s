;
;	    MSX2 hardware support
;

            .module msx2

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl map_kernel
	    .globl map_process
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore

	    ; video driver
	    .globl _vtinit

            ; exported debugging tools
            .globl _trap_monitor
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem
	    .globl _msxmaps

            .globl _tty_inproc
            .globl unix_syscall_entry
            .globl trap_illegal
	    .globl nmi_handler
	    .globl null_handler

	     ; debug symbols
            .globl outcharhex
            .globl outhl, outde, outbc
            .globl outnewline
            .globl outstring
            .globl outstringhex

	    ; stuff to save
	    .globl _vdpport
	    .globl _infobits

	    ;
	    ; vdp - we must initialize this bit early for the vt
	    ;
	    .globl vdpinit

            .include "kernel.def"
            .include "../kernel.def"

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

; Ideally return to any debugger/monitor
_trap_monitor:
	    di
	    halt


_trap_reboot:
;FIXME: TODO
	    di
	    halt

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xF000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
	    ld a, #'F'
	    out (0x2F), a
            ret

init_hardware:
	    ld a, #'U'
	    out (0x2F), a
	    ; save the useful bits of low memory first
	    ld hl, (0x2B)
	    ld (_infobits), a
;	    ld a, (0x07)
;	    ld (_vdpport), a

	    ; Size RAM
	    call size_memory

            ; set up interrupt vectors for the kernel mapped low page and
            ; data area
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

	    ld a, #'Z'
	    out (0x2F), a

	    ; Program the video engine
	    call vdpinit

	    ld a, #'I'
	    out (0x2F), a

            im 1 			; set CPU interrupt mode
	    call _vtinit		; init the console video

	    ld a, #'X'
	    out (0x2F), a
            ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

;
;	Called with interrupts off. See if we can work out how much RAM
;	there is
;
size_memory:
	    ; set system RAM size in KB
	    ld hl, #64
	    ld (_ramsize), hl
	    ld de, #0xFFD0
	    add hl, de		; subtract 48K for the kernel
	    ld (_procmem), hl
	    ret

_program_vectors:
            ; we are called, with interrupts disabled, by both newproc() and crt0
	    ; will exit with interrupts off
            di ; just to be sure
            pop de ; temporarily store return address
            pop hl ; function argument -- base page number
            push hl ; put stack back as it was
            push de

	    ; At this point the common block has already been copied
	    call map_process

            ; write zeroes across all vectors
	    ; on MSX this is probably the wrong thing to do!!! FIXME
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

            ; set restart vector for Fuzix system calls
            ld (0x0030), a   ;  (rst 30h is unix function call vector)
            ld hl, #unix_syscall_entry
            ld (0x0031), hl

            ld (0x0000), a   
            ld hl, #null_handler   ;   to Our Trap Handler
            ld (0x0001), hl

            ld (0x0066), a  ; Set vector for NMI
            ld hl, #nmi_handler
            ld (0x0067), hl
	    jr map_kernel

;
;	All registers preserved
;
map_process_always:
	    ret
;
;	HL is the page table to use, A is eaten, HL is eaten
;
map_process:
	    ld a, h
	    or l
	    jr nz, map_process_2
;
;	Map in the kernel below the current common, go via the helper
;	so our cached copy is correct.
;
map_kernel:
	    ret
map_process_2:
            ret
;
;	Restore a saved mapping. We are guaranteed that we won't switch
;	common copy between save and restore. Preserve all registers
;
map_restore:
	    ret
;
;	Save the current mapping.
;
map_save:
	    ret

; emulator debug port for now
outchar:
	    push af
	    out (0x2F), a
	    pop af
            ret

;
;	On entry D is repeating slot pattern, E repeating SSLOT pattern
;
;
;	We play with 0x0000-0x7FFF (for user mappings) and with
;	0xC000-0xFFFF (for the stupid sslot stuff), so this wants to live
;	in 0x8000-0xBFFF. We must also be very careful with our sslot
;	read/writes as while we do the sslot jiggery-pokery we have unmapped
;	our stack.
;
;
	    .area _HIGHCODE
setslot0:
	    push bc
	    in a, (0xA8)
	    and #0xFC
	    ld b, a
	    ld a, d
	    and #0x03
	    or b
	    ld b, a		; Final mapping
	    in a, (0xA8)
	    and #0x3C
	    ld c, a
	    ld a, d
	    and #0xC3
	    or c
	    ld c, a		; Temporary mapping for SSLOT crap
	    out (0xA8), a
	    ld a, e
	    and #0x03
	    ld e, a
	    ld a, (0xFFFF)
	    cpl
	    and #0xFC
	    or e
	    ld (0xFFFF), a	; Set SSLOT
	    ld a, b		; Final PSLOT (unmap the SSLOT)
	    out (0xA8), a
	    pop bc
	    ret

;
;	This can't live low as its used by _HIGHCODE stuff
;

setslot1:
	    push bc
	    in a, (0xA8)
	    and #0xF3
	    ld b, a
	    ld a, d
	    and #0x0C
	    or b
	    ld b, a		; Final mapping
	    in a, (0xA8)
	    and #0x33
	    ld c, a
	    ld a, d
	    and #0xCC
	    or c
	    ld c, a		; Temporary mapping for SSLOT crap
	    out (0xA8), a
	    ld a, e
	    and #0x0C
	    ld e, a
	    ld a, (0xFFFF)
	    cpl
	    and #0xF3
	    or e
	    ld (0xFFFF), a	; Set SSLOT
	    ld a, b		; Final PSLOT (unmap the SSLOT)
	    out (0xA8), a
	    pop bc
	    ret

;
;	Sufficient ?? FIXME - maybe wrong approach anyway
;
saveslotmap:
	    in a, (0xA8)
	    ld (slot), a	; Slots
	    ld a, (0xFFFF)
	    cpl
	    ld (slots), a
	    ret

restoreslotmap:
	    ld a, (slot)
	    out (0xA8), a
	    ld a, (slots)
	    ld (0xFFFF), a
	    ret

;
;	Can't be in common as we bugger about with 0xC000+
;
	    .area _HIGHCODE

systemslotmap:
	    in a, (0xA8)
	    ld (system_pslot), a
	    ld d, a		; need this in a register
	    and #0x3F		; all but top 16K
	    ld e, a
	    and #3		; Low 16K
	    rrca		; Into the top 16K space
	    rrca
	    or e
	    ld (system_pslot0), a
	    out (0xA8), a
	    ld a, (0xFFFF)
	    ld a, d
	    out (0xA8), a	; Put things back to sanity
	    cpl
	    ld (system_sslot0), a
	    ld a, d
	    and #0x3F
	    ld e, a
	    and #0x0C
	    rlca
	    rlca
	    rlca
	    rlca
	    or e
	    ld (system_pslot1), a
	    out (0xA8), a
	    ld a, (0xFFFF)
	    ld a, d
	    out (0xA8), a
	    cpl
	    ld (system_sslot1), a
	    ret

system_pslot0:	.db 0		; Keep paired..
system_sslot0:	.db 0
system_pslot1:	.db 0
system_sslot1:	.db 0
system_pslot:	.db 0

;
;	Map the system as we recorded it in systemslotmap
;
;	Must reside below 0xC000
;
map_system:
	    ld c, #0xA8
	    in b, (c)
	    ld de, (system_pslot0)	; e=pslot0, d=sslot0
	    out (c), e
	    ld a, d
	    ld (0xFFFF), a
	    out (c), b
	    ld de, (system_pslot1)	; e=pslot1, d=sslot1
	    out (c), e
	    ld a, d
	    ld (0xFFFF), a
	    out (c), b
	    ld a, (system_pslot)
	    out (c), a
	    ret

slotexpanded:
	    push hl
	    ld hl, #0xFFFF
	    ld a, (hl)		; Read cpl value
	    cpl			; A is now the write value
	    ld (hl), a		; Write it
	    cpl
	    cp (hl)
	    pop hl
	    ret			; Z = Expanded

slot:	    .db 0
slots:	    .db 0

;
;	Scan slot 1 for all the slots and subslots. For each slot/sub
;	call hl' with the registers exchanged for the scanner routines
;	use
;
slotscan1:
	    call saveslotmap

	    ld d, #0
nextslot:
	    ld e, #0
nextexpanded:
	    call setslot1
	    exx
	    call callhl		; Scan
	    exx
	    call slotexpanded
	    jr nz, noexpanded
	    ld a, #0x55
	    add e
	    ld e, a
	    jr nc, nextexpanded
noexpanded:
	    inc e
	    bit 2, e
	    jr z, nextslot
	    call restoreslotmap
	    ret

callhl:	    jp (hl)

megaset0:   xor a
megaset:    out (c), a		; ROM mode
	    ld (0x5fff), a	; page A
	    in a, (c)
	    ret
;
;	Hunt for a MegaRAM (assumes c set correctly by caller)
;
megaram_p:   call megaset0
	    ld a, b		; Just using d for check code
	    ld (0x5FFF), a	; Now should store this either way
	    call megaset0
	    ld a, (0x5FFF)
	    cp b		; Z = MegaRAM (probably)
	    ret

megaram_chk:
	    ld bc, #0xA58E
	    call megaram_p
	    ret nz		; Not MegaRAM
	    ld c, #0x5A
	    call megaram_p	; Paranoia check
	    ret

megaram_scan_f:
	    call megaram_chk
	    ret nz
	    exx
	    ld (megaram_i), de	; save de' (slot/subslot info)
	    exx
	    ret
megaram_i:  .dw 0

megaram_scan:
	    exx
	    ld hl, #megaram_scan_f
	    exx
	    call slotscan1
	    ld de, (megaram_i)
	    ld a, d
	    or e
	    jr z, megaram_no
	    call setslot1	; select the megaram
	    ld bc, #0x8E
	    ld hl, #0x5fff
	    call megaset0
	    ld (hl), #0x55
megaram_size:
	    ld a, b
	    call megaset
	    ld a, (hl)
	    cp #0x55
	    jr z, megawrap_maybe
megaram_size2:
	    inc b
	    jr nz, megaram_size
megaram_sized:
	    call restoreslotmap
	    ; b is the size in 8K pages
            ld a, b
	    rra
	    rra
	    and #0x3F
	    ret nz
	    add #0x40
	    ret
megaram_no:
	    xor a
	    ret

megawrap_maybe:
	    call megaset0
	    ld (hl), #0xAA
	    ld a, b
	    call megaset
	    ld a, (hl)
	    cp #0xAA		; both patterns worked
	    jr z, megaram_sized
	    jr megaram_size2	; false alarm, carry on

map_megaram:
	    ld de, (megaram_i)
	    push de
	    call setslot0
	    pop de
	    call setslot1
	    ret
