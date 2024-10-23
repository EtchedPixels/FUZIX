;
;    Spectrum +3 support

        .module cpc6128

        ; exported symbols
        .globl init_early
        .globl init_hardware
        .globl _program_vectors
        .globl plt_interrupt_all
	.globl interrupt_handler
	.globl unix_syscall_entry
	.globl null_handler
	.globl nmi_handler

        .globl map_kernel
        .globl map_proc_always
        .globl map_proc
        .globl map_kernel_di
        .globl map_proc_always_di
        .globl map_save_kernel
        .globl map_restore
	.globl map_kernel_restore
	.globl map_for_swap
	.globl map_video
	.globl current_map

        .globl _need_resched
	.globl _int_disabled
	.globl _vtborder
	.globl diskmotor

        ; exported debugging tools
        .globl _plt_monitor
	.globl _plt_reboot
        .globl outchar

        ; imported symbols
        .globl _ramsize
        .globl _procmem

	.globl _vtoutput
	.globl _vtinit

	.globl _do_beep

        .globl outcharhex
        .globl outhl, outde, outbc
        .globl outnewline
        .globl outstring
        .globl outstringhex

	.globl ___sdcc_enter_ix

        .include "kernel.def"
        .include "../../cpu-z80/kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (above 0xF000)
; -----------------------------------------------------------------------------
        .area _COMMONMEM

_plt_monitor:
	;
	;	Not so much a monitor as wait for space
	;	Part of this code is borrowed from https://github.com/lronaldo/cpctelera

	ld bc,#0x7fc2
	out (c),c		; keep us mapped
	ld  bc,  #0xF782         ;; [3] Configure PPI 8255: Set Both Port A and Port C as Output. 
	out (c), c               ;; [4] 82 = 1000 0010 : (B7=1)=> I/O Mode,       (B6-5=00)=> Mode 1,          
								;;                       (B4=0)=> Port A=Output,  (B3=0)=> Port Cu=Output, 
								;;                       (B2=0)=> Group B, Mode 0,(B1=1)=> Port B=Input, (B0=0)=> Port Cl=Output
	ld  bc,  #0xF40E         ;; [3] Write (0Eh = 14) on PPI 8255 Port A (F4h): the register we want to select on AY-3-8912
	ld  e, b                 ;; [1] Save F4h into E to use it later in the loop
	out (c), c               ;; [4]

	ld  bc,  #0xF6C0         ;; [3] Write (C0h = 11 000000b) on PPI Port C (F6h): operation > select register 
	ld  d, b                 ;; [1] Save F6h into D to use it later in the loop
	out (c), c               ;; [4]
	.dw #0x71ED ; out (c), 0 ;; [4] out (C), 0 => Write 0 on PPI's Port C to put PSG's in inactive mode 
								;; .... (required in between different operations)
	ld  bc,  #0xF792         ;; [3] Configure PPI 8255: Set Port A = Input, Port C = Output. 
	out (c), c               ;; [4] 92h= 1001 0010 : (B7=1)=> I/O Mode,        (B6-5=00)=> Mode 1,          
								;;                       (B4=1)=> Port A=Input,    (B3=0)=> Port Cu=Output, 
								;;                       (B2=0)=> Group B, Mode 0, (B1=1)=> Port B=Input, (B0=0)=> Port Cl=Output
	ld a, #0x45		
	ld    b, d               ;; [1] B = F6h => Write the value of A to PPI's Port C to select next Matrix Line
	out (c), a               ;; [4] 
	ld    b, e               ;; [1] B = F4h => Read from PPI's Port A: Pressed/Not Pressed Values from PSG
	in a,(c)                 
	rla
	jr c, _plt_monitor

_plt_reboot:
	di
	;halt ;we are debugging why we end here
	ld bc, #0x7f89 ;this would set the firmware ready for boot into firmware with (out (c),c ; rst0)
	out (c), c
	    rst 0		; back into our booter

plt_interrupt_all:
        ret

	.area _COMMONMEM

_int_disabled:
	.db 1

_vtborder:		; needs to be common
	.db 0


; -----------------------------------------------------------------------------
; KERNEL CODE BANK (below 0xC000)
; -----------------------------------------------------------------------------
        .area _CODE

init_early:

	call _program_early_vectors
        ret

init_hardware:
        ; set system RAM size
        ld hl, #128
        ld (_ramsize), hl
        ld hl, #64	      ; 64K for kernel/screen/etc (FIXME)
        ld (_procmem), hl

	; Install rst shorteners
	ld hl,#rstblock
	ld de,#8
	ld bc,#32
	ldir

	ld bc,#0x7fc3
				; bank 7 (common) in high in either mapping
				; video bank 3 at &4000
	out (c),c
			; and we should have special mapping
				; already by now	
	ld bc,#0x7f10
	out (c),c
	ld a,#0x54		;
	ld (_vtborder), a
	out (c),a		; black border
	ld bc,#0x7f00
	out (c),c
	ld a,#0x44
	out (c),a		; blue paper
	ld bc,#0x7f01
	out (c),c
	ld a,#0x4b
	out (c),a		; white ink


	;we set the crtc for a screen with 64x32 colsxrows
	;pros: max number of characters on screen and easy hardware scroll
	;cons: 80x25 is more standard => TODO list (with mode change)
	ld bc,#0xbc01
	out (c),c
	ld bc,#0xbd20
	out (c),c
	ld bc,#0xbc02
	out (c),c
	ld bc,#0xbd2A
	out (c),c
	ld bc,#0xbc06
	out (c),c
	ld bc,#0xbd20
	out (c),c
	ld bc,#0xbc07
	out (c),c
	ld bc,#0xbd22
	out (c),c

	call _do_beep

        ; screen initialization
	call _vtinit

        ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

        .area _COMMONMEM

_program_early_vectors:
	call map_proc_always
	call set_vectors
	call map_kernel
set_vectors:
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

_program_vectors:
	ret

	; Swap helper. Map the page in A into the address space such
	; that swap_map() gave the correct pointer to use. Undone by
	; a map_kernel_{restore}
map_proc:
        ld a, h
        or l
        jr z, map_kernel
map_for_swap:
map_proc_always:
map_proc_always_di:
	push af
	ld a,#0xc2			; 4 5 6 7
	jr map_a_pop
;
;	Save and switch to kernel
;
map_save_kernel:
	push af
        ld a, (current_map)
        ld (map_store), a
	pop af
map_kernel_di:
map_kernel:
map_kernel_restore:
	push af
	ld a,#0xc1			; 0 1 2 7
map_a_pop:
	push bc
	ld (current_map),a
	ld bc,#0x7f00
	out (c),a
	pop bc
	pop af
	ret

map_video:
	push af
	ld a,#0xc3			; 0 3 2 7
	jr map_a_pop

map_restore:
	push af
        ld a, (map_store)
	jr map_a_pop

;
;	We have no easy serial debug output instead just breakpoint this
;	address when debugging.
;
outchar:
	ld (_tmpout), a
	push bc
	push de
	push hl
	push ix
	ld hl, #1
	push hl
	ld hl, #_tmpout
	push hl
	call _vtoutput
	pop af
	pop af
	pop ix
	pop hl
	pop de
	pop bc
        ret

	.area _COMMONMEM
_tmpout:
	.db 1

current_map:                ; place to store current page number. Is needed
        .db 0               ; because we have no ability to read 0xF4 port
                            ; to detect what page is mapped currently 
map_store:
        .db 0

_need_resched:
        .db 0

diskmotor:
	.db 0

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

;
;	Helpers for the CH375
;
	.area _CODE

	.globl _nap20

_nap20: ;modified, in the cpc 1 nop = 1us. the call-ret add some us', it can be optimized (FIXME)
	
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop

	ret

