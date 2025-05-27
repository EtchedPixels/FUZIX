;
;	Amstrad CPC with standard memory expansion hardware support
;

	.module cpcsme

	; exported symbols
	.globl init_early
	.globl init_hardware
	.globl _program_vectors
	.globl plt_interrupt_all

	.globl map_kernel_low
	.globl map_save_low
	.globl map_restore_low
	.globl map_user_low
	.globl map_page_low

	.globl map_kernel
	.globl map_video


	.globl _plt_reboot

	.globl _plt_doexec

	.globl _int_disabled

	.globl MMR_for_this_bank

	.globl _vtborder


	; exported debugging tools
	.globl _plt_monitor
	.globl outchar

	; imported symbols
	.globl _ramsize
	.globl _procmem

	.globl s__COMMONMEM
	.globl l__COMMONMEM

	.globl syscall_platform
	.globl unix_syscall_entry
	.globl my_nmi_handler
	.globl interrupt_handler
	.globl _doexit
	.globl kstack_top
	.globl istack_top
	.globl istack_switched_sp
	.globl _panic
	.globl _need_resched
	.globl _ssig
	.globl _udata

	.globl _vtoutput
	.globl _vtinit

	.globl _do_beep

	.globl interrupt_high

	.globl outcharhex
	.globl outhl, outde, outbc
	.globl outnewline
	.globl outstring
	.globl outstringhex

	.globl nmi_handler

	.include "kernel.def"
	.include "../../cpu-z80/kernel-z80.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF200 upwards)
; -----------------------------------------------------------------------------
        .area _COMMONMEM

_plt_monitor:
	;
	;	Not so much a monitor, waits for space key pressed - could change border to warn
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
	ld a, #0x45					;; SPACE
	ld    b, d               ;; [1] B = F6h => Write the value of A to PPI's Port C to select next Matrix Line
	out (c), a               ;; [4] 
	ld    b, e               ;; [1] B = F4h => Read from PPI's Port A: Pressed/Not Pressed Values from PSG
	in a,(c)                 
	rla
	jr c, _plt_monitor

_plt_reboot:
	di
	ld bc, #0x7f89 	;this would set the firmware ready for boot into firmware with (out (c),c ; rst0)
					;work with the 6128 firmware, fails with the 464 & the 664 firmware, need to investigate.
	out (c), c
	rst 0		; back into our booter
plt_interrupt_all:
	ret

_int_disabled:
	.db 1

_vtborder:		; needs to be common
	.db 0

MMR_for_this_bank:
	.db 0
; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (only accessible when the kernel is mapped)
; In this port, as DISCARD area is in the  upper 16K we can use C1 map to see the
; first 48K, copy a routine to populate the common area of all banks to 0x8000
; and that routine uses cf,d7,df.. maps to copy the common area and 
; to mark all the 64K banks with its MMR value
; see https://www.cpcwiki.eu/index.php/Standard_Memory_Expansions
; and https://www.cpcwiki.eu/index.php/Gate_Array#Register_MMR_.28RAM_memory_mapping.29
; -----------------------------------------------------------------------------
	.area _DISCARD

init_early: 
		ld bc, #0x7fc1 			;convenient map to copy common to a visible area for copy routine
		out (c),c				;also used for video output
		ld hl, #s__COMMONMEM
		ld de, #0x8000			;temporary buffer to hold common copy
		ld bc, #l__COMMONMEM
		ldir
		ld hl, #copy_common
		ld de, #0x100
		ld bc, #copy_common_end-#copy_common
		ldir
		jp 0x100
early_ret:
		ld bc, #0x7fc2 			;Kernel map
		out (c),c
		ret
copy_common:		;Copy common to each user bank top 16K mapped at 0x4000
		ld d,#0
		ld e,#0
cc_loop:			; copy the common memory to all banks
		ld hl,#0x100+#MMR_array_for_copy_common-#copy_common
		add hl,de
		ld a,(hl)
		ld bc,#0x7fff
		out(c),a
		exx
		ld hl, #0x8000
		ld de, #s__COMMONMEM - #0x8000
		ld bc, #l__COMMONMEM
		ldir
		exx
		ld hl,#0x100+#MMR_array_for_user_bank-#copy_common
		add hl,de
		ld a,(hl)
		ld (#MMR_for_this_bank - #0x8000),a ;mark each user bank with the corresponding MMR value
		ld a,(#0x100+#nmaps-#copy_common)
		inc e
		cp e
		jr nz,cc_loop
		ld bc, #0x7fc1
		out (c),c
		ld hl, #0x8000		;clear VRAM for tty2
		ld de, #0x8000 + 1
		ld bc, #0x3fff
		ld (hl), #0
		ldir
			; Write the stubs for video map
		ld hl,#stubs_low
		ld de,#0x0000
		ld bc,#stubs_low_end-stubs_low
		ldir
;		ld a,#0xc9				;temp solution, could we mark c1 map and let it be interrupted by kernel as common is maped in?
;		ld (0x38),a				;for now ignore interrupts in this map
		
		jp early_ret
nmaps:
        .db 7
MMR_array_for_copy_common:
        .db 0xcf
        .db 0xd7
        .db 0xdf
        .db 0xe7
        .db 0xef
        .db 0xf7
		.db 0xff
MMR_array_for_user_bank:
        .db 0xca
        .db 0xd2
        .db 0xda
        .db 0xe2
        .db 0xea
        .db 0xf2
		.db 0xfa
copy_common_end:

init_hardware:
        ; set system RAM size
        ld hl, #512		; Assuming 576K cpc for now. First 64K video, second kernel, rest user
        ld (_ramsize), hl
        ld hl, #(512-64)	; 64K for kernel
        ld (_procmem), hl

	; Write the stubs for our bank
	ld hl,#stubs_low
	ld de,#0x0000
	ld bc,#stubs_low_end-stubs_low
	ldir

	;init usifac serial

	ld bc,#0xfbd1
	ld a,#16
	out (c),a ; try to set usifac baudrate 115200
	ld c,#0xdd
	in a,(c)
	cp #16
	jr nz, end_usifac ;usifac is not present
	ld c,#0xd1
flush_usifac:	
	in a,(c)
	dec a
	jr z,end_usifac
	dec c
	in a,(c)
	inc c
	jr flush_usifac
end_usifac:
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


	;we set the crtc for a screen with 64x32 colsxrows, video page at 0x4000
	;https://www.cpcwiki.eu/index.php/CRTC
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
	ld bc,#0xbc0c           ;
	out (c),c
	ld bc,#0xbd10
	out (c),c

	call _do_beep

        ; screen initialization
	call _vtinit

        ret

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

	.area _COMMONMEM

;
;	We switch in one go so we don't have these helpers. This means
;	we need custom I/O wrappers and custom usercopy functions.MMR_array_for_user_bank:


map_save_low:
map_kernel_low:
map_restore_low:
map_user_low:
map_page_low:
	ret

_program_vectors:
	pop de
	pop hl
	push hl
	push de
	push ix
	; From kernel to user bank
	ld d,#0xc2
	ld e,(hl)
	; Write the stubs for our bank
	ld hl,#stubs_low
	ld ix,#0x0000
	ld bc,#stubs_low_end-stubs_low
	call ldir_far
	pop ix
	ret


; outchar: Print the char in A
outchar:
	push bc
	ld bc,#0xfbd0
	out(c),a
	pop bc
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

_tmpout:
	.db 1

;
; Don't be tempted to put the symbol in the code below ..it's relocated
; to zero. Instead define where it ends up.
;

_plt_doexec	.equ	0x28

        .area _COMMONMEM

	.globl rst38
	.globl stubs_low
	.globl ___sdcc_enter_ix
;
;	This exists at the bottom of each bank. We move these into place
;	from discard.
;

stubs_low:
	.byte 0xC9	;changed from c3 to keep going!!
	.word 0		; cp/m emu changes this
	.byte 0		; cp/m emu I/O byte
	.byte 0		; cp/m emu drive and user
	jp 0		; cp/m emu bdos entry point

;
;	Stub helpers for code compactness. Note that
;	sdcc_enter_ix is in the standard compiler support already
;
;	The first two use an rst as a jump. In the reload sp case we don't
;	have to care. In the pop ix case for the function end we need to
;	drop the spare frame first, but we know that af contents don't
;	matter
;

rstblock:
	jp	___sdcc_enter_ix
	nop
	nop
	nop
	nop
	nop
___spixret:
	ld	sp,ix
	pop	ix
	ret
	nop
	nop
	nop
___ixret:
	pop	af
	pop	ix
	ret
	nop
	nop
	nop
	nop
___ldhlhl:
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	ret
	nop
	nop
	nop
rst28:	jp plt_doexec_high
	nop
	nop
	nop
	nop
	nop
rst30:	jp syscall_high
	nop
	nop
	nop
	nop
	nop
;
;	We only have 38-4F available for this in low space
;
rst38:	jp interrupt_high		; Interrupt handling stub
	nop
	nop
	nop
	nop
	nop
	.ds 0x26
my_nmi_handler:	jp nmi_handler
stubs_low_end:


;MMR_for_this_bank:	
;	.ds 1 ;; Should be at 0x68
;
;	This stuff needs to live somewhere, anywhere out of the way (so we
;	use common). We need to copy it to the same address on both banks
;	so place it in common as we will put common into both banks
;

	.area _COMMONMEM

	.globl ldir_to_user
	.globl ldir_from_user
	.globl ldir_far
;
;	This needs some properly optimized versions!
;
ldir_to_user:
	ld a,(_udata + U_DATA__U_PAGE)	;
	ld e,a
	ld d,#0xc2			; Kernel is in 0xc2
ldir_far:				;hl->source address, d->source bank, ix->destination address, e->destination bank
	push bc				;bc->byte count
	ld bc,#0x7f10
	out (c),c
	ld c,#0x4b   ;Bright white
	out (c),c
	ld bc,#0x7fff
	exx
	pop bc			; get BC into alt bank
far_ldir_1:
	exx
	out (c),d			; Select source
	ld a,(hl)
	inc hl
	out (c),e			; Select target
	ld (ix),a
	inc ix
	exx
	dec bc
	ld a,b
	or c
	jr nz, far_ldir_1
	ld bc,#0x7fc2
	out (c),c		; Select kernel
	ld bc,#0x7f10
	out (c),c                                    
	ld a,(_vtborder)
	out (c),a
	ret
ldir_from_user:
	ld a,(_udata + U_DATA__U_PAGE)
	ld e,#0xc2
	ld d,a
	jr ldir_far
;
;	High stubs. Present in each bank in the top 256 bytes
;	of the available space (remembering F000-FFFF is not available
;	for general usage but is ok for reading)
;
interrupt_high:
	push af
	push de
	push hl
	ex af,af'
	push af
	push bc
	exx
	push bc
	push de
	push hl
	push ix
	push iy
	ld a,(#MMR_for_this_bank)		; bank MMR is stored in each bank early
	ld d,a
	ld bc,#0x7fc2
	out (c),c		; Kernel map
	ld (istack_switched_sp),sp	; istack is positioned to be valid
	ld sp,#istack_top		; in both banks. We just have to
	;
	;	interrupt_handler may come back on a different stack in
	;	which case de is junk. Fortuntely we never pre-empt in
	;	kernel so the case we care about de is always safe. This is
	;	not a good way to write code and should be fixed! FIXME
	;
	push de
	call interrupt_handler	; switch on the right SP
	pop de
	; Restore stack pointer to user. This leaves us with an invalid
	; stack pointer if called from user but interrupts are off anyway
	ld sp,(istack_switched_sp)
	; On return HL = signal vector E= signal (if any) A = page for
	; high
	or a 
	jr z, kernout
	; Returning to user space
	ld bc,#0x7fff
	out (c),a		; page passed back
	; User stack is now valid
	; back on user stack
	xor a
	cp e
	call nz, sigpath
pops:
	ex af,af'
	exx
	pop iy
	pop ix
	pop hl
	pop de
	pop bc
	exx
	pop bc
	pop af
	ex af,af'
	pop hl
	pop de
	pop af
	ei
	ret
kernout:
	; restore bank - if we interrupt mid user copy or similar we
	; have to put the right bank back
	ld a,d
	ld bc,#0x7fff
	out (c),a
	jr pops

sigpath:
	push de		; signal number
	ld de,#irqsigret
	push de		; clean up
	ex de,hl
	ld hl,(PROGLOAD+16)	; helper vector
	jp (hl)
irqsigret:
	inc sp		; drop signal number
	inc sp
	ret

syscall_platform:
syscall_high:
	push ix
	ld ix,#0
	add ix,sp
	push iy
	push bc
	ld c,6(ix)
	ld b,7(ix)
	ld e,8(ix)
	ld d,9(ix)
	ld l,10(ix)
	ld h,11(ix)
	push hl
	ld l,12(ix)
	ld h,13(ix)
	pop ix
	di
	exx
	push bc
	ld bc,#0x7fc2
	out (c),c
	exx
	; Stack now invalid
	ld (_udata + U_DATA__U_SYSCALL_SP),sp
	ld sp,#kstack_top
	call unix_syscall_entry
	; FIXME check di rules
	; stack now invalid. Grab the new sp before we unbank the
	; memory holding it
	ld sp,(_udata + U_DATA__U_SYSCALL_SP)
	ld a, (_udata + U_DATA__U_PAGE)	; back to the user page
	exx
	ld c,#0xff
	out (c),a
	pop bc
	exx
	xor a
	cp h
	call nz, syscall_sigret
	; FIXME for now do the grungy C flag HL DE stuff from
	; lowlevel-z80 until we fix the ABI
	pop bc
	pop iy
	ld a,h
	or l
	jr nz, error
	ex de,hl
	pop ix
	ei
	ret
error:	scf
	pop ix
	ei
	ret
syscall_sigret:
	ld a,l		; DEBUG
	push hl		; save errno
	push de		; save retval
	ld l,h
	ld h,#0
	push hl		; signal
	ld hl,#syscall_sighelp
	push hl		; vector
	push bc
	ret
syscall_sighelp:
	pop de		; discard signal
	pop de		; recover error info
	pop hl
	ld h,#0		; clear signal bit
	ret
plt_doexec_high:
	; Activate the user bank (which also holds these bytes)
	ld a,(_udata + U_DATA__U_PAGE)
	ld bc,#0x7fff
	out (c),a
	ei
	; and leap into user space
	jp (hl)

map_kernel:
	push bc
	ld bc,#0x7fc2
	out (c),c
;	push af
;	ld a,c
;	ld (#MMR_for_this_bank),a ; mark kernel bank with its MMR value
;	pop af
	pop bc
	ret

map_video:
	push bc	
	ld bc,#0x7fc1
	out (c),c	
;	push af
;	ld a,c
;	ld (#MMR_for_this_bank),a ; mark kernel bank with video map MMR value
;	pop af
	pop bc
	ret

;
;	Helpers for the CH375
;
	.area _CODE

	.globl _nap20

_nap20: ;modified, in the cpc 1 nop = 1us, call = 5 nops and ret = 3 nops => do 12 nops to wait 20us
	
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

