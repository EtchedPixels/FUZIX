;
;	    MSX2 hardware support
;

            .module msx1

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
	    .globl _slot_table
	    .globl _kernel_flag

	    ; video driver
	    .globl _vtinit

            ; exported debugging tools
            .globl _trap_monitor
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem

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

_kernel_flag:
	   .db 1


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
	    call megaram_scan
	    ld l, a
	    ld h, #0
	    add hl, hl
	    add hl, hl
	    add hl, hl			; megaram for the user space
	    ld (_procmem), hl
	    ld de, #0x40		; main memory for kernel
	    add hl, de
	    ld (_ramsize), hl
	    call rom_scan
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
	    push hl
	    ld hl, #U_DATA__U_PAGE
	    call map_process_2
	    pop hl
	    ret
;
;	HL is the page table to use, A is eaten, HL is eaten
;
map_process:
	    ld a, h
	    or l
	    jr nz, map_process_2
;
;	Map in the kernel low area (0x0000-0x7FFF)
;
map_kernel:
	    push af
	    ld a, i
	    push af
	    di
	    ld a, #'K'
	    out (0x2f), a
	    ld a, (map_cache)	; Already mapped
	    or a
	    jr z, map_kernel_out
	    push bc
	    push de
	    push hl
	    call restoreslotmap
	    xor a
	    ld (map_cache), a
	    pop hl
	    pop de
	    pop bc
map_kernel_out:
	    ld a, #'k'
	    out (0x2f), a
	    pop af
	    jp po, map_kernel_di
	    ei
map_kernel_di:
	    pop af
	    ret
map_process_2:
	    push af
	    ld a, i
	    push af
	    di
	    push de
	    ld a, #'U'
	    out (0x2f), a
	    ld a, (map_cache)
	    or a
	    jr nz, inter_mega
	    ld a, #'M'
	    out (0x2f), a
	    call map_megaram
	    ld a, #'m'
	    out (0x2f), a
inter_mega:
	    ld a, (hl)
	    ld (map_cache), a
	    dec a		; turn the bank number into 0 offset
	    add a, a
	    add a, a		; 4 pages per process
	    out (0x8E), a
	    ld (0), a
	    inc a
	    ld (1), a
	    inc a
	    ld (0x4000), a
	    inc a
	    ld (0x4001), a
	    in a, (0x8E)
	    pop de
	    ld a, #'u'
	    out (0x2f), a
	    pop af
	    jp po, map_proc_di
	    ei
map_proc_di:pop af
            ret
;
;	Restore a saved mapping. We are guaranteed that we won't switch
;	common copy between save and restore. Preserve all registers
;
map_restore:
	    push hl
	    push af
	    ld hl, #map_saved
	    ld a, (hl)		; Kernel ?
	    or a
	    jr nz, map_ruser
	    ld hl, #0		; Do a kernel remap not a user one
map_ruser:
	    call map_process	; Map it
	    pop af
	    pop hl
	    ret
;
;	Save the current mapping (trivial)
;
map_save:
	    push af
	    ld a, (map_cache)
	    ld (map_saved), a
	    pop af
	    ret

map_cache:  .db 0
map_saved:  .db 0

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
	    push de
	    ld a, #'S'
	    out (0x2f), a
	    ld a, #'0'
	    out (0x2f), a
	    ld a, d
	    call outcharhex
	    ld a, e
	    call outcharhex
	    in a, (0xA8)
	    and #0xFC		; For final setting
	    ld b, a
	    and #0x3C		; For setting sslot
	    ld c, a
	    ld a, d
	    and #0xC3
	    or c
	    out (0xA8), a	; Map the right slot in bank 0 and 3
	    ld a, e
	    and #0x03
	    ld e, a
	    ld a, (0xFFFF)
	    cpl
	    and #0xFC
	    or e
	    ld (0xFFFF), a	; Set SSLOT
	    ld a, d
	    and #0x03
	    or b
	    out (0xA8), a
	    call outcharhex
	    ld a, #'/'
	    out (0x2f), a
	    pop de
	    pop bc
	    ret

;
;	This can't live low as its used by _HIGHCODE stuff
;

setslot1:
	    push bc
	    push de
	    ld a, #'S'
	    out (0x2f), a
	    ld a, #'1'
	    out (0x2f), a
	    ld a, d
	    call outcharhex
	    ld a, e
	    call outcharhex
	    in a, (0xA8)
	    and #0xF3		; For final setting
	    ld b, a
	    and #0x33		; For setting sslot
	    ld c, a
	    ld a, d
	    and #0xCC
	    or c
	    out (0xA8), a	; Map the right slot in bank 1 and 3
	    ld a, e
	    and #0x0C
	    ld e, a
	    ld a, (0xFFFF)
	    cpl
	    and #0xF3
	    or e
	    ld (0xFFFF), a	; Set SSLOT
	    ld a, d
	    and #0x0C
	    or b
	    out (0xA8), a
	    call outcharhex
	    ld a, #'/'
	    out (0x2f), a
	    pop de
	    pop bc
	    ret

;
;	Save the slot/subslot map. This requires much mucking about
;	remapping the top bank
;
saveslotmap:
	    ld hl, #slotsave
	    ld bc, #0x05A8	; count for slots (+ 1 for ini), port
	    ini
	    in d, (c)		; Load again for working
	    ld e, #0		; Slot number
saveslotmap1:
	    ld a, d
	    and #0x3f		; Mask off top bank
	    or e
	    out (c), a
	    ld a, (0xFFFF)
	    cpl
	    ld (hl), a
	    inc hl
	    ld a, #0x40
	    add e
	    ld e, a
	    djnz saveslotmap1
	    out (c), d
	    ret

restoreslotmap:
	    ld hl, #slotsave
	    ld bc, #0x05A8
	    outi
	    in d, (c)
	    ld e, #0		; Slot number
resslotmap1:
	    ld a, d
	    and #0x3f		; Mask off top bank
	    or e
	    out (c), a
	    ld a, (hl)
	    ld (0xFFFF), a	; Load the subslot into each
	    inc hl
	    ld a, #0x40
	    add e
	    ld e, a
	    djnz resslotmap1
	    out (c), d
	    ret

slotsave:   .db 0
	    .db 0, 0, 0, 0

slotexpanded:
	    push hl
	    push de
	    in a, (0xA8)
	    call outcharhex
	    in a, (0xA8)
	    ld e, a
	    and #0x3F
	    ld d, a
	    and #0x0C
	    rlca
	    rlca
	    rlca
	    rlca
	    or d
	    push af
	    call outcharhex
	    pop af
	    out (0xA8), a
	    ld hl, #0xFFFF
	    ld a, (hl)		; Read cpl value
	    cpl			; A is now the write value
	    ld (hl), a		; Write it
	    cpl
	    cp (hl)
	    jr z, isexp
	    ld a, #'N'
	    out (0x2f), a
isexp:
	    ld a,e
	    out (0xA8), a
	    pop de
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
	    call setslot1
	    exx
	    call callhl		; Scan
	    exx
	    call slotexpanded
	    jr nz, noexpanded
	    jr expanders
nextexpanded:
	    call setslot1
	    exx
;	    call callhl		; Scan
	    exx
expanders:
	    ld a, #0x55
	    add e
	    ld e, a
	    ld a, #'.'
	    out (0x2f), a
	    jr nc, nextexpanded
noexpanded:
	    ld a, #'|'
	    out (0x2f), a
	    ld a, d
	    add #0x55
	    ld d, a
	    jr nc, nextslot
	    call restoreslotmap
	    ret

callhl:	    jp (hl)

megaset0:   xor a
megaset:    out (c), a		; ROM mode
	    ld (hl), a		; page A
	    in a, (c)		; RAM mode
	    ret
;
;	Hunt for a MegaRAM (assumes c set correctly by caller)
;
;	The basic idea is
;		Stick it in paging mode
;		Page it to 0
;		Stick it in RAM mode
;		Write a value
;		Stick it back in paging mode
;		Write 0
;
;	If it's a MegaRAM the memory will remain as the value, if it's real RAM
;	it will be zeroed by the paging write back
;
megaram_p:
	    ld d, (hl)
	    call megaset0
	    ld (hl), b		; Now should store this either way
	    call megaset0	; Should not change if megaram
	    ld a, (hl)
	    ld (hl), d		; Restore old bits
	    cp b		; Z = MegaRAM (probably)
	    ret

megaram_chk:
	    ld hl, #0x5FFF
	    ld bc, #0xA58E	; A5 is the pattern we use
	    call megaram_p
	    ret nz		; Not MegaRAM
	    ld b, #0x5A
	    call megaram_p	; Paranoia check
	    ret

megaram_scan_f:
	    push hl
	    call megaram_chk
	    pop hl
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
	    ld a, #'M'
	    out (0x2f), a
	    call setslot1	; select the megaram
	    ld bc, #0x8E
	    ld hl, #0x4000
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

;
;	Find each rom and insert it by hash code into the slot table. We
;	can then use this to find out where things like floppy drives have
;	been hidden. Right now we are only scanning 0x4000-0x7FFF, but we
;	will probably need to scan 0x8000-0xBFFF as well eventually (which
;	will be a right PITA as we'll need slotscan2 and all the supporting
;	logic to be in 0x0000-0x3FFF!)
;
rom_scan:
	    ld hl, #rom_scan_f
	    exx
	    call slotscan1
	    ret

rom_scan_f:
	    ld a, #'*'
	    out (0x2f), a
	    ld a, (0x4000)
	    cp #'A'
	    ret nz
	    ld a, (0x4001)
	    cp #'B'
	    ret nz
	    ;
	    ; ROM found. Preserve HL as its the call vector
	    ;
	    ld a, #'R'
	    out (0x2f), a
	    push hl
	    ld hl, #0x4002
	    ld bc, #2048
	    ld de, #0
	    ; Use the low 2K as a checksum identifier
rom_scan_1:
	    ld a, (hl)
	    add e
	    ld e, a
	    ld a, d
	    adc #0
	    ld d, a
	    inc hl
	    djnz rom_scan_1
	    dec c
	    jr nz,rom_scan_1
	    exx
	    push de	;slot info
	    exx
	    pop bc
	    ld a, b
	    and #0x0C
	    ld b, a
	    ld a, c
	    and #0x03
            or b
	    add a
	    ld c, a
	    ld b, #0
	    ld hl, #_slot_table
	    add hl, bc
	    ld (hl), e
	    inc hl
	    ld (hl), d
	    pop hl
	    ret

; Needs to be outside of 0x4000-0x7FFF
_slot_table:
	    .dw 0,0,0,0
	    .dw 0,0,0,0
	    .dw 0,0,0,0
	    .dw 0,0,0,0
