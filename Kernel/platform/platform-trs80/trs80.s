;
;	    TRS 80  hardware support
;

            .module trs80

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl plt_interrupt_all

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
	    .globl fd_nmi_handler
	    .globl null_handler
	    .globl map_kernel
	    .globl map_proc
	    .globl map_proc_a
	    .globl _opreg
	    .globl _modout
	    .globl _int_disabled

	    .globl map_proc_always
	    .globl map_kernel

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

	    .globl _bufpool

            .include "kernel.def"
            .include "../../cpu-z80/kernel-z80.def"

	    .area _BUFFERS

_bufpool:
	    .ds BUFSIZE * NBUFS
; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xE800 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_int_disabled:
	    .db 1

_plt_monitor:
	    di
	    halt

plt_interrupt_all:
	    ret

;
;	We write the following into low memory
;	out (0x84),a
;	inc a
;	out (0x9c),a
;	.. then resets into ROM (see the technical reference manual)
;
_plt_reboot:
	   di
	   ld hl,#0x84D3		; out (0x84),a
	   ld (0),hl
	   ld hl,#0xD33C		; inc a, out (
	   ld (2),hl
	   ld a,#0x9C			; 9c),a
	   ld (4),a
	   xor a
	   rst 0

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xE800, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

_ctc6845:				; registers in order
	    .db 99, 80, 85, 10, 25, 4, 24, 24, 0, 9, 101, 9, 0, 0, 0, 0
init_early:
	    ld a, #0x24			; uart rx, timer
	    out (0xE0), a
	    ld a, (_opreg)
	    out (0x84), a
	    ld a, (_modout)
	    out (0xEC), a

            ; load the 6845 parameters
	    ld hl, #_ctc6845
	    ld bc, #0x88
ctcloop:    out (c), b			; register
	    ld a, (hl)
	    out (0x89), a		; data
	    inc hl
	    inc b
	    bit 4,b
	    jr z, ctcloop

   	    ; clear screen
	    ld hl, #0xF800
	    ld (hl), #'*'		; debugging aid in top left
	    inc hl
	    ld de, #0xF802
	    ld bc, #1998
	    ld (hl), #' '
	    ldir
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

            ; set restart vector for UZI system calls
            ld (0x0030), a   ;  (rst 30h is unix function call vector)
            ld hl, #unix_syscall_entry
            ld (0x0031), hl

            ld (0x0000), a   
            ld hl, #null_handler   ;   to Our Trap Handler
            ld (0x0001), hl

            ld (0x0066), a  ; Set vector for NMI
            ld hl, #fd_nmi_handler
            ld (0x0067), hl
	    jp map_kernel
	    
; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
            out (0xEB), a
            ret

;
;	Swap helpers
;
_hd_xfer_in:
	   pop de
	   pop hl
	   push hl
	   push de
	   ld a, (_hd_page)
	   or a
	   call nz, map_proc_a
	   ld bc, #0xC8			; 256 bytes from 0xC8
	   inir
	   call map_kernel
	   ret

_hd_xfer_out:
	   pop de
	   pop hl
	   push hl
	   push de
	   ld a, (_hd_page)
	   or a
	   call nz, map_proc_a
	   ld bc, #0xC8			; 256 bytes to 0xC8
	   otir
xfer_out_wait:				; busy wait after OTIR
	   ex (sp), hl
	   ex (sp), hl
	   in a,(0xCF)
	   rlca
	   jr c, xfer_out_wait
	   call map_kernel
	   ret

;
;	Graphics
;
	.globl _gfx_write
	.globl _gfx_read
	.globl _gfx_draw
	.globl _gfx_exg
	.globl _gfx_blit

	.globl _ctrl_cache

_gfx_write:
	call map_proc_always
	push ix
	push hl
	pop ix
	ld de,#8
	add hl,de

	ld e,6(ix)			; Lines to do
	ld d,2(ix)			; Y

	ld a,(_ctrl_cache)
	or #0x40			; clock X on write
	out (0x83),a
	ld c,#0x82			; data

wnextline:
	ld a,d
	out (0x81),a			; set Y
	ld b,4(ix)			; width
	ld a,0(ix)			; X left
	out (0x80),a
	otir				; write bytes
	inc d				; move down
	dec e				; count one off
	jr nz, wnextline
	pop ix
	ld hl,#0
	jp map_kernel


_gfx_read:
	call map_proc_always
	push ix
	push hl
	pop ix
	ld de,#8
	add hl,de

	ld e,4(ix)			; Lines to do
	ld d,0(ix)			; Y

	ld a,(_ctrl_cache)
	or #0x10			; clock X on read
	out (0x83),a
	ld c,#0x82			; data

rnextline:
	ld a,d
	out (0x81),a			; set Y
	ld b,6(ix)			; width
	ld a,2(ix)			; X left
	out (0x80),a
	inir				; write bytes
	inc d				; move down
	dec e				; count one off
	jr nz, rnextline
	pop ix
	ld hl,#0
	jp map_kernel


_gfx_exg:
	call map_proc_always
	push ix
	push hl
	pop ix
	ld de,#8
	add hl,de

	ld e,4(ix)			; Lines to do
	ld d,0(ix)			; Y

	ld a,(_ctrl_cache)
	or #0x20			; clock X on write
	out (0x83),a
	ld c,#0x82			; data

enextline:
	ld a,d
	out (0x81),a			; set Y
	ld b,6(ix)			; width
	ld a,2(ix)			; X left
	out (0x80),a
ebyte:
	ld a,(hl)			; get new
	ini				; read byte to memory
	out (0x82),a			; write new and inc
	djnz ebyte
	inc d				; move down
	dec e				; count one off
	jr nz, enextline
	pop ix
	ld hl,#0
	jp map_kernel

_gfx_draw:
	call map_proc_always		; Length, Y, X
	ld de,#6			
	add hl,de
	push ix
	push hl
	pop ix
	ld a,(_ctrl_cache)
	or #0x40			; clock X on write
	out (0x83),a
	ld d,4(ix)			; X
	ld e,2(ix)			; Y
nextopline:
	; Set position
	ld a,d
	out (0x80),a
	ld a,e
	out (0x81),a
nextop:
	xor a
	ld b,(hl)
	cp b
	jr z,line_done
	inc hl
	ld c,(hl)
	inc hl
oploop:
	in a,(0x82)			; read no inc
	and c
	xor (hl)
	out (0x82),a			; write, inc
	djnz oploop
	inc hl
	jr nextop
line_done:
	inc e				; down a line
	xor a
	cp (hl)
	jr  nz, nextopline
	pop ix
	ld hl,#0
	jp map_kernel

_gfx_blit:
	push ix
	push hl
	pop ix

	ld d,10(ix)			; width
	res 7,d

	exx
	ld a,(_ctrl_cache)
	or #0x10			; alt bc are the config sets
	ld b,a
	xor #0x30
	ld c,a
	exx

	ld a,0(ix)			; Compare Y
	cp 4(ix)
	jr c, blitrd
	jr nz, blitlu
	ld a,2(ix)			; and X
	cp 6(ix)
	jr c, blitrd

blitlu:
	ld e,8(ix)			; start on bottom line and work up
	dec e
blitlu_n:
	exx
	ld a,b
	out (0x83),a
	exx
	ld a,2(ix)			; source X
	out (0x80),a
	ld a,e
	add 0(ix)			; add Y source
	out (0x81),a
	ld b,d				; width of line
	ld hl,#scratch
	inir
	exx
	ld a,6(ix)
	out (0x80),a
	ld a,e
	add 4(ix)			; add Y dest
	out (0x81),a
	ld a,c
	out (0x83),a
	exx	
	ld b,d
	ld hl,#scratch
	otir
	dec e
	jr nz, blitlu_n
	pop ix
	ld hl,#0
	ret
	
blitrd:
	ld e,#0				; line counter
blitrd_n:
	exx
	ld a,b
	out (0x83),a
	exx
	ld a,d				; source X
	out (0x80),a
	ld a,e
	add 0(ix)			; add Y source
	out (0x81),a
	ld b,8(ix)			; width of line
	ld hl,#scratch
	inir
	exx
	ld a,d
	out (0x80),a
	ld a,e
	add 4(ix)			; add Y dest
	out (0x81),a
	ld a,c
	out (0x83),a
	exx	
	ld b,8(ix)
	ld hl,#scratch
	otir
	inc e
	ld a, 8(ix)			; height match ?
	cp e
	jr nz, blitrd_n
	pop ix
	ld hl,#0
	ret

scratch:
	.ds 128
