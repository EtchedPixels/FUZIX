;
;	    NC200 hardware support
;

            .module nc200

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl map_kernel
	    .globl map_kernel_restore
	    .globl map_proc
	    .globl map_proc_always
	    .globl map_kernel_di
	    .globl map_proc_di
	    .globl map_proc_always_di
	    .globl map_save_kernel
	    .globl map_restore
	    .globl _int_disabled
	    .globl top_bank

	    ; for the PCMCIA disc driver
	    .globl _rd_memcpy

	    ; video driver
	    .globl _scroll_up
	    .globl _scroll_down
	    .globl _plot_char
	    .globl _clear_lines
	    .globl _clear_across
	    .globl _cursor_on
	    .globl _cursor_off
	    .globl _cursor_disable
	    .globl _cursorpos
	    ; need the font
	    .globl _fontdata_6x8
	    .globl _vtinit
	    .globl plt_interrupt_all
	    .globl _video_cmd
	    .globl _vtattr_notify

            ; exported debugging tools
            .globl _plt_monitor
            .globl _plt_reboot
            .globl outchar

            ; imported symbols
            .globl _ramsize
            .globl _procmem
            .globl _tty_inproc
            .globl unix_syscall_entry
	    .globl nmi_handler
	    .globl null_handler
	    .globl _udata
	    .globl ___sdcc_enter_ix

	     ; debug symbols
            .globl outcharhex
            .globl outhl, outde, outbc
            .globl outnewline
            .globl outstring
            .globl outstringhex

            .include "kernel.def"
            .include "../../cpu-z80/kernel-z80.def" ; Kernel
	    .include "nc200.def"

; -----------------------------------------------------------------------------
; COMMON MEMORY BANK (0xF000 upwards)
; -----------------------------------------------------------------------------
            .area _COMMONMEM

_int_disabled:
	    .db 1

_plt_monitor:
	    di
	    halt
	    jr _plt_monitor

_plt_reboot:
	    xor a
	    out (0x70), a

plt_interrupt_all:
            ret

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xF000, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

init_early:
            ret

init_hardware:

	    call probe_pcmcia
	    jr z, pcmciabootable

	    ld hl,#304
	    ld (_ramsize),hl
	    ld hl,#304-64
	    ld (_procmem),hl
	    jr to_setup
pcmciabootable:
            ; set system RAM size
            ld hl, #256
            ld (_ramsize), hl
            ld hl, #(256-64)		; 64K for kernel
            ld (_procmem), hl
to_setup:
	    ; 100Hz timer on

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

	    ld hl,#rstblock		; install rst hooks
            ld de,#8
            ld bc,#32
            ldir

	    ld a, #0x08			; keyboard IRQ only
	    out (0x60), a		; set up
	    xor a
	    out (0x90), a
            im 1 ; set CPU interrupt mode
            in a, (0xB9)
	    call _vtinit		; init the console video
            ret


;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

probe_pcmcia:
	    ld bc,#0x8011
	    in a,(c)
	    out (c),b
	    ld hl,(0x4200)
	    out (c),a
	    ld de,#0x434E		; should be "NC"
	    or a
	    sbc hl,de
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

            ld (0x0000), a   
            ld hl, #null_handler   ;   to Our Trap Handler
            ld (0x0001), hl

            ld (0x0066), a  ; Set vector for NMI
            ld hl, #my_nmi_handler
            ld (0x0067), hl
	    jp map_kernel

;
;	These are shared and agreed with the bootstrap code in the
;	bootstrap low page
;
entry_sp     .equ 0x1BE
kernel_sp   .equ 0x1C0
resume_vector .equ 0x1C2
suspend_map .equ 0x1C4
entry_banks .equ 0x1C8
suspend_stack .equ 0x1FE

suspend_r:   .db 0

my_nmi_handler:
	    push af
	    xor a
	    jr suspend_1
suspend:
	    push af
	    ld a,#1
suspend_1:
	    ld (suspend_r),a
	    ld a, (_int_disabled)
	    push af
	    push bc
	    di
	    in a,(0x10)
	    ld c,a
	    ld a,#0x80
	    out (0x10),a
	    ; Save our SP
	    ld (kernel_sp), sp
	    ld sp,#suspend_stack
	    push de
	    push hl
	    push ix
	    push iy
	    exx
	    push bc
	    push de
	    push hl
	    ex af,af'
	    push af
	    exx
	    ; Register map saved (first one is wrong)
	    ld hl,#suspend_map
	    ld (hl),c
	    inc hl
	    in a,(0x11)
	    ld (hl),a
	    inc hl
	    in a,(0x12)
	    ld (hl),a
	    inc hl
	    in a,(0x13)
	    ld (hl),a
	    ld hl,#resume
	    ld (resume_vector),hl
	    ld de,(suspend_r)
	    ; Now switch the low 48K (OS) banks back as they were
	    ; and run the "official" OS NMI handler
	    ld hl,(entry_banks)
	    ld sp,(entry_sp);
	    ld a,l
	    out (0x10),a
	    ld a,h
	    out (0x11),a
	    ld a,(entry_banks+2)
	    out (0x12),a
	    ld a,e
	    or a
	    jr z, nmi_and_resume
	    ; Hand triggered suspend
	    ld a,(entry_sp)
	    ; To NC100 OS
	    ret
nmi_and_resume:
	    call 0x66
;
;	Called by the booter via our resume vector. The bootstrap or ROM has
;	already set the high 16K correctly before calling us there
;
resume:
	    di
	    ld hl,#0
	    ld (resume_vector),hl
	    ld hl,#suspend_map+1
	    ; Restore hardware state
	    ld a,#0x08
	    out (0x60),a		; keyboard IRQ only
	    xor a
	    out (0x90),a
	    im 1
	    in a,(0xB9)			; clear pending
	    ; Begin by restoring the mapping for 0x4000-0xBFFF
	    ld a,(hl)
	    out (0x11),a
	    inc hl
	    ld a,(hl)
	    out (0x12),a
	    ; Map the bootstrap bank back in at 0x0000-0x3FFF so we can get
	    ; our data back
            ld a,#0x80
	    out (0x10),a
	    ; Restore the registers
	    ld sp,#suspend_stack-16
	    pop af
	    ex af,af
	    pop hl
	    pop de
	    pop bc
	    exx
	    pop iy
	    pop ix
	    pop hl
	    pop de

	    ; Switch back to the kernel stack pointer. This could be in any
	    ; bank so we must not dereference it until we fix the low 16K
	    ld sp,(kernel_sp)
	    ; Grab the low 16K mapping we had before
	    ld a,(suspend_map)
	    out (0x10),a	; booter page in 0x0000-0x3FFF vanishes here
				; and our vectors re-appear
	    pop bc		; BC was saved on the other stack so we had
				; work room
	    pop af		; IRQ state
	    or a
	    jr nz,no_irq_on
	    ei
no_irq_on:  pop af
	    ret
;
;	Userspace mapping pages 7+  kernel mapping pages 3-5, first common 6
;
;
;	All registers preserved
;
map_proc_always:
map_proc_always_di:
	    push hl
	    push af
	    ld hl, #_udata + U_DATA__U_PAGE
	    call map_proc_2
	    pop af
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
;	Map in the kernel below the current common, all registers preserved
;
map_kernel:
map_kernel_restore:
map_kernel_di:
	    push af
	    ; kernel is in banks 3/4/5, common starts at 6 but then gets
	    ; copied into each task
	    ld a, #0x83
	    out (0x10), a
	    inc a
	    out (0x11), a
	    inc a
	    out (0x12), a
	    pop af
            ret
map_proc_2:
	    ld a, (hl)
	    out (0x10), a
	    inc hl
	    ld a, (hl)
	    out (0x11), a
	    inc hl
	    ld a, (hl)
	    out (0x12), a
            ret
;
;	Restore a saved mapping. We are guaranteed that we won't switch
;	common copy between save and restore. Preserve all registers
;
map_restore:
	    push hl
	    push af
	    ld hl,#map_savearea
	    call map_proc_2
	    pop af
	    pop hl
	    ret
;
;	Save the current mapping.
;
map_save_kernel:
	    push hl
	    push af
	    ld hl, #map_savearea
	    call save_maps
	    ; kernel is in banks 3/4/5, common starts at 6 but then gets
	    ; copied into each task
	    ld a, #0x83
	    out (0x10), a
	    inc a
	    out (0x11), a
	    inc a
	    out (0x12), a
	    pop af
	    pop hl
	    ret

save_maps:
	    in a, (0x10)
	    ld (hl), a
	    inc hl
	    in a, (0x11)
	    ld (hl), a
	    inc hl
	    in a, (0x12)
	    ld (hl), a
	    inc hl
	    in a, (0x13)
	    ld (hl), a
	    ret

map_savearea:
	    .db 0,0,0,0
top_bank:			; actually a dummy to keep the lib code
				; happy
	    .db 0

; outchar: Wait for UART TX idle, then print the char in A
; destroys: AF
outchar:
	    push af
outcharw:
            in a, (0xC1)
	    bit 0, a
	    jr z, outcharw
	    pop af
	    out (0xC0), a
            ret

;
; Disk helper - FIXME - do we need the di/ei still
;
_rd_memcpy:  push ix
	    ld ix, #0
	    add ix, sp
	    ; 4(ix) = is_read, 5(ix) = dptr page 6-7(ix) = dptr, 8-9(ix) = block
	    ; 10-11(ix) = len, 12(ix) = map2, 13-14(ix) = len2
            ld l, 6(ix)
	    ld h, 7(ix)
	    ld c, 5(ix)
            ld e, 8(ix)
            ld d, 9(ix)
            ld a, h
            and #0x3F		; Remove bank bits from H
            or #0x80            ; Will be at 0x8000
            ld h, a		; HL is now ready
	    ; DE is the block, but we need to work out where that block
	    ; lives in terms of 16K chunks
	    sla e               ; e = e * 2 (will be 512 in a bit)
	    rl d		; this is OK we won't overflow on a 1MB device
            ld a, e		; save a copy
	    sla e
            rl d
	    sla e
            rl d		; D now holds the bank
            push af
            ld a, d
	    add #0x80
            di
            out (0x11), a	; 0x4000 is now the ramdisc bank
	    pop af
	    and #0x3F		; Mask bank
	    or #0x40		; bank is at 0x4000
	    ld d, a		; e = e * 256 (so now in byte terms)
            ld e, #0		; always aligned

	    ld a, c
            out (0x12), a	; bank 0x8000 is now the user/kernel buffer
	    bit 0, 4(ix)	; read or write ?
	    jr z, rd_write

	    ex de, hl
	    ;
	    ;	All mapped, and then its simple
	    ;
rd_write:   ld c, 10(ix)
	    ld b, 11(ix)
	    ldir

	    ld a, 12(ix)		; second page map or 0 = none
	    or a
	    jr z, rd_done	; usual case
	    out (0x12) ,a
	    ld c, 13(ix)
	    ld b, 14(ix)
	    ld a, 4(ix)
	    or a
	    jr nz, rd_part2
	    ld hl, #0x8000
	    jr copy_part2
rd_part2:   ld de, #0x8000
copy_part2: ldir
rd_done:
            call map_kernel	; map the kernel and return
	    ei
	    pop ix
            ret            

;
;	FIXME: should be safe to drop the di/ei on these
;
_scroll_up:
	    ld a, (_int_disabled)
	    push af
	    di
	    in a, (0x11)
	    push af
	    ld a, #0x43		; main memory, bank 3 (video etc)
	    out (0x11), a
	    ld hl, #VIDEO_BASE + 512
	    ld de, #VIDEO_BASE
	    ld bc, #VIDEO_SIZE - 512 - 1
	    ldir
	    jr vtdone

_scroll_down:
	    ld a, (_int_disabled)
	    push af
	    di
	    in a, (0x11)
	    push af
	    ld a, #0x43		; main memory, bank 3 (video etc)
	    out (0x11), a
	    ld hl, #VIDEO_BASE + 0xFFF
	    ld de, #VIDEO_BASE + 0xFFF - 512
	    ld bc, #VIDEO_SIZE - 512 - 1
	    lddr
vtdone:	    pop af
	    out (0x11), a
	    pop af
	    or a
	    ret nz
	    ei
_vtattr_notify:
_cursor_disable:
	    ret

;
;	Turn a co-ordinate pair in DE into an address in DE and map the
; video. Return B = 0-3 for the character within the 3 byte set
; preserves H, L, C
;
;
;	What we are effectively doing here is Y * 512 +  X * 3 / 4
;	and keeping a remainder too
;
addr_de:
	    push hl
	    ld a,#0x43
            out (0x11),a
	    ld a,e 	; Y. We have 64 bytes per line 8 per row so
	    rlca	; We know high bit was 0 so low bit is 0
	    add #VIDEO_BASEH
	    ld h,a	; And that's the row sorted
	    ld a,d	; X
	    and #3
	    ld b,a	; bit offset info (remainder)
	    ld a,d	; X
	    add a
	    add d	; X * 3	gives us the left side of the char
	    rrca
            rrca
	    and #0x3F
	    ld l,a
	    ex de,hl
	    pop hl
	    ret

;
;	We rely upon the font data ending up above 0x8000. On the current
; size that should never be a problem.
;
_plot_char:
	    pop hl
	    pop de	; d, e = co-ords
	    pop bc	; c = char
	    push bc
	    push de
	    push hl
	    ld a, (_int_disabled)
	    push af
	    di
	    in a, (0x11)
	    push af
	    call addr_de
	    push de	; save while we sort the char out
	    ld  a, c
	    sub #32
            jr nc, chok
	    xor a	; space
chok:
	    add a	; 128 chars only so this also nicely wraps it
	    ld l,a
	    ld h,#0
	    add hl,hl
	    add hl,hl	; 8 bytes per char
	    ld de,#_fontdata_6x8
	    add hl,de	; HL is now the font bytes we want with 1 byte per
			; line right packed
	    ; There are four possible rendering positions
	    push hl
            pop ix
	    pop hl
	    ld a,b
	    ld b,#8
	    ld de,#64
	    or a
	    jr z, render0
	    dec a
	    jr z, render1
	    dec a
	    jr z, render2
	    ; easiest - right 6 of byte 2
render3_l:
	    ld a,(hl)
            and #0xC0
            or (ix)
            ld (hl),a
            inc ix
	    add hl,de
	    djnz render3_l
            jr vtdone
render0:    ; left 6 bits of byte 0
            ld a,(hl)
            and #0x03
	    ld c,(ix)
            sla c
            sla c
            or c
            ld (hl),a
	    inc ix
	    add hl,de
            djnz render0
            jr vtdone
render1:    ; 2bits of byte 0, 4 of byte 1
render1_l:
            ld a,(hl)
            and #0xFC
            ld c,a
            ld a,(ix)
	    rrca
            rrca
            rrca
            rrca
            ld e,a
            and #0x03
            or c
            ld (hl),a
            inc hl
            ld a,(hl)
            and #0x0F
            ld c,a
            ld a,e
            and #0xF0
            ld (hl),a
            inc ix
            ld e,#63
            add hl,de
            djnz render1_l
	    jp vtdone
render2:    ; 4 of byte 1, 2 of byte 2
render2_l:
	    ld a,(hl)
            and #0xF0
            ld c,a
	    ld a,(ix)
            rrca
            rrca
            ld e,a
	    and #0x0F
            or c
            ld (hl),a
            inc hl
            ld a,(hl)
            and #0x3F
            ld c,a
            ld a,e
            and #0xC0
            or c
            ld (hl),a
            inc ix
            ld e,#63
            add hl,de
            djnz render2_l
	    jp vtdone

_clear_lines:
	    pop hl
	    pop de		; E = y, D = count
	    push de
	    push hl
	    ld a, (_int_disabled)
	    push af
	    di
	    in a, (0x11)
	    push af
	    ld c, d
	    ld d, #0
	    call addr_de
	    ld a, c		; lines
	    or a
	    jp z, vtdone
lines:
	    ld h, d
	    ld l, e
	    ld (hl), #0x0
	    inc de
	    ld bc, #511
	    ldir
            dec a
	    jr nz, lines
	    jp vtdone

_clear_across:
	    pop hl
	    pop de		; E = y, D = x
	    pop bc		; C = count
	    push bc
	    push de
	    push hl
	    ld a, (_int_disabled)
	    push af
	    di
	    in a, (0x11)
	    push af
	    call addr_de
	    ex de, hl
	    ; How many full bytes ?
	    ld a,c
	    and #3
	    ld b,a		; remainder bits in B for the moment
            ld a,c		; 3 * count
	    add a
	    add c
	    rrca
	    rrca
	    and #0x3F		; 3/4 count - byte count for clear

	    ld c,b		; move remainder into C

            ld b,#8		; Lines
            ld de,#64		; Add for next line

rowloop:
            push bc		; Save working state
            push hl
	    push af
	    or a		; We might only have parts of a byte to do
	    jr z, clear_end
	    ld b,a
            xor a
clear_l:    ld (hl),a		; Clear whole bytes
            inc hl
            djnz clear_l
clear_end:
            ld a,c		; Check on partials 0 - nop wo
            or a		;
	    jr z,nextrow	; No extra bits
	    dec a
            jr z,clear_1	; Clear top 6 bits of next byte
            dec a
            jr z,clear_2	; Clear 4 bits
            ld a,(hl)		; Clear 2 bits
            and #0x3F
nextrow:
            ld (hl),a
	    pop af
            pop hl
            add hl,de
            pop bc
            djnz rowloop
            jp vtdone
clear_1:
	    ld a,(hl)
            and #0x0F
            jr nextrow
clear_2:
            ld a,(hl)
            and #0x03
            jr nextrow
	
_cursor_on:
	    pop hl
	    pop de
	    push de
	    push hl
cursor_do:
	    ld a, (_int_disabled)
	    push af
            di
	    in a, (0x11)
	    push af
	    ld (_cursorpos), de
	    call addr_de
	    ex de,hl
	    ld de,#64
	    ld a,b
	    ld b,#8
	    or a
	    jr z,cursor0
	    dec a
	    jr z,cursor1
	    dec a
	    jr z,cursor2
cursor3:
cursor3_l:
	    ; low 6 bits byte 2
	    ld a,(hl)
            xor #0x3F
            ld (hl),a
	    add hl, de
	    djnz cursor3_l
	    jp vtdone
cursor0:    ; top bits byte 0
            ld a,(hl)
            xor #0xFC
            ld (hl),a
            add hl,de
            djnz cursor0
            jp vtdone
cursor1:    ; 2 bits byte 0 4 byte 1
            ld e,#63
cursor1_l:
            ld a,(hl)
            xor #0x03
            ld (hl),a
            inc hl
            ld a,(hl)
            xor #0xF0
            ld (hl),a
            add hl,de
            djnz cursor1_l
	    jp vtdone
cursor2:	; 4 bits byte 1 2 of byte 2
            ld e,#63
cursor2_l:
            ld a,(hl)
            xor #0x0F
            ld (hl),a
            inc hl
            ld a,(hl)
            xor #0xC0
            ld (hl),a
            add hl,de
            djnz cursor2_l
	    jp vtdone

_cursor_off:
	    ld de, (_cursorpos)
	    jr cursor_do


;
;	Turn a co-ordinate pair in DE into an address in DE
;	and map the video. Unlike the font version we work in bytes across
;	(for NC100, NC200 needs doing)
;
addr_de_pix:
	    ld a, #0x43
	    out (0x11), a

	    ld b, d	; save X
	    ld a, e	; turn Y into a pixel row
	    sla d	; X is in 0-63, rotate it left twice
	    sla d
	    srl a	; multiple by 64 A into DE
	    rr  d	; roll two bits into D pushing the X bits
	    srl a	; back where they came from
	    rr  d
	    add #VIDEO_BASEH	; screen start (0x7000 or 0x6000 for NC200)
	    ld  e, d
	    ld  d, a
	    ret
;
;	For NC100 (NC200 needs doing)
;
;	video_cmd(uint8_t *buf)
;
_video_cmd:
	    pop de
	    pop hl
	    push hl
	    push de
	    in a, (0x11)
	    push af
	    ld e,(hl)
	    inc hl
	    inc hl
	    ld d,(hl)
	    inc hl
	    inc hl
	    call addr_de_pix	; turn DE into screen address (HL is kept)
nextline:
	    push de
nextop:
	    xor a
	    ld b, (hl)
	    cp b
	    jr z, endline
	    inc hl
	    ld c,(hl)
	    inc hl
oploop:
	    ld a,(de)
	    and c
	    xor (hl)
	    ld (de), a
	    inc de
	    djnz oploop
	    inc hl
	    jr nextop
endline:    pop de
	    ex de,hl
	    ld bc, #64
	    add hl, bc
	    ex de, hl
	    inc hl
	    xor a
	    cp (hl)		; 0 0 = end (for blank lines just do 01 ff 00)
	    jr nz, nextline
	    pop af
	    out (0x11), a
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
