;
;	    MSX2 hardware support
;

            .module msx2

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
	    .globl platform_interrupt_all
            .globl _program_vectors
	    .globl map_kernel
	    .globl map_process
	    .globl _map_kernel
	    .globl map_process_always
	    .globl map_save
	    .globl map_restore
	    .globl enaslt
	    .globl _mapslot_bank1
	    .globl _mapslot_bank2
	    .globl _kernel_flag

	    ; video driver
	    .globl _vtinit

            ; exported debugging tools
            .globl _trap_monitor
            .globl outchar

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


	    .globl _slotrom
	    .globl _slotram
	    .globl _vdpport
	    .globl _infobits
	    .globl _machine_type

	    ;
	    ; vdp - we must initialize this bit early for the vt
	    ;
	    .globl _vdpinit

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
	    ret

init_hardware:
            ; set up interrupt vectors for the kernel mapped low page and
            ; data area
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

	    ld a, #'Z'
	    out (0x2F), a

	    call _vdpinit

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

platform_interrupt_all:
	    ld bc,(_vdpport)
	    in a, (c)
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
;	Map in the kernel below the current common, go via the helper
;	so our cached copy is correct.
;
_map_kernel:
map_kernel:
	    push hl
	    ld hl, #map_kernel_data
	    call map_process_2
	    pop hl
	    ret

map_process_2:
	    push de
	    push af
	    ld de, #map_table	; Write only so cache in RAM
	    ld a, (hl)
	    ld (de), a
	    out (0xFC), a	; Low 16K
	    inc hl
	    inc de
	    ld a, (hl)	
	    out (0xFD), a	; Next 16K
	    ld (de), a
	    inc hl
	    inc de
	    ld a, (hl)		; Next 16K. Leave the common for the task
	    out (0xFE), a	; switcher
	    ld (de), a
	    pop af
	    pop de
            ret
;
;	Restore a saved mapping. We are guaranteed that we won't switch
;	common copy between save and restore. Preserve all registers
;
map_restore:
	    push hl
	    ld hl,#map_savearea
	    call map_process_2	; Put the mapper back right
	    pop hl
	    ret
;
;	Save the current mapping.
;
map_save:   push hl
	    ld hl, (map_table)
	    ld (map_savearea), hl
	    ld hl, (map_table + 2)
	    ld (map_savearea + 2), hl
	    pop hl
	    ret

;
;	Slot mapping functions.
;
;   necessary to access memory mapped io ports used by certain devices
;   (e.g ide, sd devices)
;
;   These need to go in bank0; cannot be in the common area because
;   they do switch bank3 to access the subslot register. And neither
;   can be in bank1 or 2 because those are the ones usually used to
;   map the io ports.
;
		.area _CODE

_mapslot_bank1:
		ld hl,#0x4000
		jr enaslt
_mapslot_bank2:
		ld hl,#0x8000

enaslt:
		call setprm         ; calculate bit pattern and mask code
		jp m, mapsec        ; if expanded set secondary first
		in a,(0xa8)
		and c
		or b
		out (0xa8),a        ; set primary slot
		ret
mapsec:
		push hl
		; here need to store the slot that is being set....
		call setexp         ; set secondary slot
		pop hl
		jr enaslt

		; calculate bit pattern and mask
setprm:
		di
		push af
		ld a,h
		rlca
		rlca
		and #3
		ld e,a              ; bank number
		ld a,#0xC0
setprm1:
		rlca
		rlca
		dec e
		jp p, setprm1
		ld e,a              ; mask pattern
		cpl
		ld c,a              ; inverted mask pattern
		pop af
		push af
		and #3              ; extract xxxxxxPP
		inc a
		ld b,a
		ld a,#0xAB
setprm2:
		add a,#0x55
		djnz setprm2
		ld d,a              ; primary slot bit pattern
		and e
		ld b,a
		pop af
		and a               ; if expanded slot set sign flag
		ret

		; set secondary slot
setexp:
		push af
		ld a,d
		and #0xC0          ; get slot number for bank 3
		ld c,a
		pop af
		push af
		ld d,a
		in a,(0xa8)
		ld b,a
		and #0x3F
		or c
		out (0xa8),a        ; set bank 3 to target slot
		ld a,d
		rrca
		rrca
		and #3
		ld d,a
		ld a,#0xAB          ; secondary slot to bit pattern
setexp1:
		add a,#0x55
		dec d
		jp p,setexp1
		and e
		ld d,a
		ld a,e
		cpl
		ld h,a
		ld a,(0xffff)       ; read and update secondary slot register
		cpl
		ld l,a
		and h               ; strip off old bits
		or d                ; add new bits
		ld (0xffff),a
		ld a,b
		out (0xa8),a        ; restore status
		pop af
		and #3
		ret

		.area _COMMONMEM


map_table:
	    .db 0,0,0,0	
map_savearea:
	    .db 0,0,0,0
map_kernel_data:
	    .db 3,2,1,4
_slotrom:
	    .db 0
_slotram:
	    .db 0
_vdpport:
	    .dw 0
_infobits:
	    .dw 0
_machine_type:
	    .db 0

; emulator debug port for now
outchar:
	    push af
	    out (0x2F), a
	    pop af
	    ret

