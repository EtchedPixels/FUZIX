;
;	    MSX2 hardware support
;

            .module msx2

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
	    .globl plt_interrupt_all
            .globl _program_vectors
	    .globl map_kernel
	    .globl map_kernel_restore
	    .globl map_proc
	    .globl _map_kernel
	    .globl map_proc_always
	    .globl map_kernel_di
	    .globl map_proc_di
	    .globl map_proc_always_di
	    .globl map_save_kernel
	    .globl map_restore
	    .globl enaslt
	    .globl _mapslot_bank1
	    .globl _mapslot_bank2
	    .globl _need_resched
            .globl _bufpool
	    .globl _int_disabled
            .globl _udata
            .globl ___sdcc_enter_ix

	    ; video driver
	    .globl _vtinit

            ; exported debugging tools
            .globl _plt_monitor
            .globl outchar

            .globl _tty_inproc
            .globl unix_syscall_entry
            .globl _plt_reboot
	    .globl nmi_handler
	    .globl null_handler

	     ; debug symbols
            .globl outcharhex
            .globl outhl, outde, outbc
            .globl outnewline
            .globl outstring
            .globl outstringhex

	    ; machine bits
	    .globl _slotrom
	    .globl _slotram
	    .globl _vdpport
	    .globl _infobits
	    .globl _machine_type

            ; IDE driver
	    .globl _ide_slot
	    .globl _ide_error
	    .globl _ide_base
	    .globl _ide_addr
	    .globl _ide_lba
	    .globl _ide_is_read

	    ;
	    ; vdp - we must initialize this bit early for the vt
	    ;
	    .globl _vdpinit

            .include "kernel.def"
            .include "../kernel-z80.def"

	    .area _BUFFERS

_bufpool:
	    .ds BUFSIZE * NBUFS
; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

; Ideally return to any debugger/monitor
_plt_monitor:
	    di
	    halt


_plt_reboot:
;FIXME: TODO
	    di
	    halt

_need_resched:
	    .db 0
_int_disabled:
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

            ; RST peepholes support (only in kernel maps)
	    ld hl,#rstblock
	    ld de,#8
	    ld bc,#32
	    ldir

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

plt_interrupt_all:
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
	    call map_proc

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
map_proc_always:
map_proc_always_di:
	    push hl
	    ld hl, #_udata + U_DATA__U_PAGE
	    call map_proc_2
	    pop hl
	    ret
;
;	HL is the page table to use, A is eaten, HL is eaten
;
map_proc:
map_proc_di:
	    ld a, h
	    or l
	    jr nz, map_proc_2
;
;	Map in the kernel below the current common, go via the helper
;	so our cached copy is correct.
;
_map_kernel:
map_kernel_restore:
map_kernel_di:
map_kernel:
	    push hl
	    ld hl, #map_kernel_data
	    call map_proc_2
	    pop hl
	    ret

map_proc_2:
	    push de
            push bc
	    push af
	    ld de, #map_table	; Write only so cache in RAM
            ld (de), a
            ld bc, #4
            ldir
            dec hl
            dec hl
            dec hl
            dec hl
            ld c, #0xFC
            ld b, #4
            outi
            inc c
            outi
            inc c
            outi
	    pop af
            pop bc
	    pop de
            ret
;
;	Restore a saved mapping. We are guaranteed that we won't switch
;	common copy between save and restore. Preserve all registers
;
map_restore:
	    push hl
	    ld hl,#map_savearea
	    call map_proc_2	; Put the mapper back right
	    pop hl
	    ret
;
;	Save the current mapping.
;
map_save_kernel:
            push hl
	    ld hl, (map_table)
	    ld (map_savearea), hl
	    ld hl, (map_table + 2)
	    ld (map_savearea + 2), hl
	    ld hl, #map_kernel_data
	    call map_proc_2
	    pop hl
	    ret

		.area _CODE

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
_ide_slot:
	    .db 0
_ide_error:
	    .dw 0
_ide_base:
	    .dw 0
_ide_addr:
	    .dw 0
_ide_lba:
	    .dw 0
	    .dw 0
_ide_is_read:
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

;
;	Stub helpers for code compactness. Note that
;	sdcc_enter_ix is in the standard compiler support already
;
	.area _DISCARD

;
;	The first two use an rst as a jump. In the reload sp case we don't
;	have to care. In the pop ix case for the function end we need to
;	drop the spare frame first, but we know that af contents don't
;	matter
;

rstblock:
	jp	___sdcc_enter_ix
	.ds	5
___spixret:
	ld	sp,ix
	pop	ix
	ret
	.ds	3
___ixret:
	pop	af
	pop	ix
	ret
	.ds	4
___ldhlhl:
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	ret
