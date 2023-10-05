;
;	    Microbee 128K and 256TC  hardware support
;

            .module ubee

            ; exported symbols
            .globl init_early
            .globl init_hardware
            .globl interrupt_handler
            .globl _program_vectors
	    .globl map_buffers
	    .globl map_kernel
	    .globl map_kernel_di
	    .globl map_kernel_restore
	    .globl map_proc
	    .globl map_proc_di
	    .globl map_proc_a
	    .globl map_proc_always
	    .globl map_proc_always_di
	    .globl map_save_kernel
	    .globl map_restore
	    .globl map_for_swap
	    .globl plt_interrupt_all
	    .globl _kernel_flag
	    .globl _int_disabled

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
	    .globl _vtinit

	    .globl _ubee_model

	    .globl s__COMMONMEM
	    .globl l__COMMONMEM

            .include "kernel.def"
            .include "../../cpu-z80/kernel-z80.def"

;
; Buffers (we use asm to set this up as we need them in a special segment
; so we can recover the discard memory into the buffer pool
;

	    .globl _bufpool
	    .area _BUFFERS

_bufpool:
	    .ds BUFSIZE * NBUFS

;
; COMMON MEMORY BANK (kept even when we task switch)
;
            .area _COMMONMEM

_int_disabled:
	    .db 1

plt_interrupt_all:
	    in a,(0xef)			; FIXME: remove this line once debugged
	    ret

_plt_monitor:
_plt_reboot:
	    di
	    halt
;
;	Sit in common and play with the banks to see what we have
;
size_ram:
	    ; We could have < 128, 128 or various extensions up to 512K or
	    ; so.
	    ld ix,#0x80			; safe scribble
	    ld c,#0x01
	    ld de,#page_codes
	    ld (ix),#0			; clear in bank 0 low
	    ld hl,#32
scan_ram:
	    add hl,hl
	    ld a,(de)			; try entry in table
	    or a
	    jr z,scan_done		; finished
	    inc de
	    out (0x50),a		; select proposed bank
	    ld (ix),c			; write to it
	    ld a,#0x0C
	    out (0x50),a		; back to bank 0
	    ld a,(ix)			; read it back
	    cp c			; did it mess with bank 0L ?
	    jr nz, scan_ram

;	We found our first mismatch HL is our memory size
;	info if at least 128K is present (or 64 if not)
scan_done:
	    ret

page_codes:
	    ; Detect a standard or premium 128K system
	    .byte 0x0E  ;	write 1 to bank 1L 0x80
	    ; Detect a system with a 256K expansion mod
	    .byte 0x4C	;	write 1 to bank 2L 0x80
	    ; Detect a system with a 512K expansion mod
	    .byte 0x8C	;	write 1 to bank 4L 0x80
	    ; We don't handle the modern ubee premium plus emulated thing
	    .byte 0x00  ;	and done

; -----------------------------------------------------------------------------
; KERNEL MEMORY BANK (below 0xE800, only accessible when the kernel is mapped)
; -----------------------------------------------------------------------------
            .area _CODE

	    .globl _ctc_load
	    .globl _ctc6545

_ctc_load:
	    pop de
	    pop hl
	    push hl
	    push de
ctcload:
	    ld bc, #0x0F0C
ctcloop:    out (c), b			; register
	    ld a, (hl)
	    out (0x0D), a		; data
	    inc hl
	    dec b
	    jp p, ctcloop
	    ret

;
;	This setting list comes from the Microbee 256TC documentation
;	and is the quoted table for 80x25 mode
;
_ctc6545:				; registers in reverse order
	    .db 0x00, 0x00, 0x00, 0x20, 0x0A, 0x09, 0x0A, 0x48
	    .db 0x1A, 0x19, 0x05, 0x1B, 0x37, 0x58, 0x50, 0x6B
_ctc6545_64:
	    .db 0x00, 0x00, 0x00, 0x00, 0x0F, 0x2F, 0x0F, 0x48
	    .db 0x12, 0x10, 0x09, 0x12, 0x37, 0x51, 0x40, 0x6B
_ctc6545_40:
	    .db 0x00, 0x00, 0x00, 0x20, 0x0A, 0x2A, 0x0A, 0x48
	    .db 0x1A, 0x19, 0x05, 0x1B, 0x24, 0x2D, 0x28, 0x35

	    .area _DISCARD

init_early:
            ; load the 6545 parameters
	    ld hl, #_ctc6545
	    call ctcload
	    ; ensure the CTC clock is right
	    ld a, #0
	    in a, (9)			; manual says in but double check
	    xor a
	    out (0x0B),a		; vanish the character rom
	    ld a,#0x40
	    out (0x08),a		; colour, black background
	    ld a,#0x14			; map the video in at 0x8000
	    out (0x50),a
   	    ; clear screen
	    ld hl, #0x8000
	    ld (hl), #'*'		; debugging aid in top left
	    inc hl
	    ld de, #0x8002
	    ld bc, #1998
	    ld (hl), #' '
	    ldir
	    ld hl,#0x8800
	    ld de,#0x8801
	    ld (hl), #4			; green characters/black
	    ld bc,#1999
	    ldir
	    ld a,(_ubee_model)
	    or a
	    jr z, no_attribs
	    ;
	    ;	Map in and wipe attribute memory on the premium and tc
	    ;	models.
	    ;
	    ld a,#0x10			; Enable attribute memory and
	    out (0x1C),a		; wipe it
	    ld hl,#0x8000
 	    ld de,#0x8001
	    ld bc,#1999
	    ld (hl),#0
	    ldir
	    ld a,#0x80			; extended PCG on
	    out (0x1C),a
no_attribs:
	    ld a,#0x0C			; video back off
	    out (0x50),a
	    jp _vtinit

init_hardware:
	    ld a,(_ubee_model)
	    cp #2			; 256TC
	    ld hl,#256			; 256TC has 256K
	    call nz, size_ram
is_tc:
            ld (_ramsize), hl
	    ld de,#64			; 64K for kernel
	    or a
	    sbc hl,de
            ld (_procmem), hl

            ; set up interrupt vectors for the kernel (also sets up common memory in page 0x000F which is unused)
            ld hl, #0
            push hl
            call _program_vectors
            pop hl

	    ld a,(_ubee_model)
	    cp #2
	    ld hl,#pio_setup
	    jr nz, not_t256
	    ld hl,#pio_setup_t256
not_t256:
	    call init_ports
	    call init_ports
	    ;
	    ; set up the RTC driven periodic timer. The PIA should already
	    ; have been configured for us
	    ;
	    ; we don't necessarily have one. In which case this
	    ; routine will pee into the void and do no harm whatsoever
	    ;
	    ld bc,#0x0A04
	    out (c), b			; select register A
	    ld a,#0x70			; reset divier
	    out (0x06), a
	    ld a,#0x2D			; and select 32KHz operation
	    out (0x06), a		; with an 8Hz interrupt
	    ld a,#0x40
	    inc b
	    out (c),b
	    ld a,#0x46			; PIE, binary, 24 hour
	    out (0x06), a
	    inc b
	    out (c),b
	    in a,(0x07)			; Clear pending interrupt

            im 1 ; set CPU interrupt mode

            ret

;
;	This nifty routine is c/o Stewart Kay's CP/M 3 for the Microbee
;	- slightly tweaked to make it re-callable for series of tables
;
init_ports:
	    ld b,(hl)
	    inc hl
	    ld c,(hl)
	    inc hl
	    otir
	    ld a,(hl)
	    or a
	    jr nz,init_ports
	    inc hl
	    ret

pio_setup:
	    .byte	0x04
	    .byte	0x01
	    ; vector, mode 0 (output), int control off, on
	    .byte	0x00,0x0F,0x03,0x83
	    .byte	0
	    ; and port B
	    .byte	0x06
	    .byte	0x03
	    ; vector 0, mode 3, 7/4/3/0 are input
	    ; interrupt enable, or, high, mask follows
	    ; interrupt on 7/4
	    ; int control on
	    .byte	0x00, 0xCF, 0x99, 0xB7, 0x6F, 0x83
	    .byte	0x01
	    .byte	0x02
	    ; and set the data lines so the rs232 looks sane (not that
	    ; we care right now).
	    .byte	0x24
	    .byte	0x00

pio_setup_t256:
	    .byte	0x04
	    .byte	0x01
	    ; vector, mode 0 (output), int control off, on
	    .byte	0x00,0x0F,0x03,0x83
	    .byte	0
	    ; and port B
	    .byte	0x06
	    .byte	0x03
	    ; vector 0, mode 3, 7/4/3/1/0 are input
	    ; interrupt enable, or, high, mask follows
	    ; interrupt on 7/4/1
	    ; int control on
	    .byte	0x00, 0xCF, 0x9B, 0xB7, 0x6D, 0x83
	    .byte	0x01
	    .byte	0x02
	    ; and set the data lines so the rs232 looks sane (not that
	    ; we care right now).
	    .byte	0x24
	    .byte	0x00

;------------------------------------------------------------------------------
; COMMON MEMORY PROCEDURES FOLLOW

            .area _COMMONMEM

mapreg:    .db 0
mapsave:   .db 0

_kernel_flag:
	    .db 1	; We start in kernel mode

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

            ; set restart vector for FUZIX system calls
            ld (0x0030), a   ;  (rst 30h is unix function call vector)
            ld hl, #unix_syscall_entry
            ld (0x0031), hl

            ld (0x0000), a   
            ld hl, #null_handler   ;   to Our Trap Handler
            ld (0x0001), hl

            ld (0x0066), a  ; Set vector for NMI
            ld hl, #fd_nmi_handler
            ld (0x0067), hl

;
;	Mapping set up for the Microbee
;
;	The top 32K bank holds kernel code and pieces of common memory
;	The lower 32K is switched between the various user banks.
;	We don't have a separate 32K for buffers as memory is too precious
;	on must Microbees
;
map_buffers:
map_kernel:
map_kernel_di:
map_kernel_restore:
	    push af
	    ld a, #0x0C		; bank 0, 1 no ROM or video
	    ld (mapreg), a
	    out (0x50), a
	    pop af
	    ret
map_proc:
map_proc_di:
	    ld a, h
	    or l
	    jr z, map_kernel
map_proc_hl:
	    ld a, (hl)
map_for_swap:
map_proc_a:			; used by bankfork
	    ld (mapreg), a
	    out (0x50), a
            ret

map_proc_always:
map_proc_always_di:
	    push af
	    push hl
	    ld hl, #_udata + U_DATA__U_PAGE
	    call map_proc_hl
	    pop hl
	    pop af
	    ret

map_save_kernel:
	    push af
	    ld a, (mapreg)
	    ld (mapsave), a
	    ld a, #0x0C		; bank 0, 1 no ROM or video
	    ld (mapreg), a
	    out (0x50), a
	    pop af
	    ret

map_restore:
	    push af
	    ld a, (mapsave)
	    ld (mapreg), a
	    out (0x50), a
	    pop af
	    ret
	    
; No UART (could use printer port ?)
outchar:
            ret

;
;	Ubee Keyboard. Except for the TC the Ubee pulls this crazy (or neat
;	depending how you look at it) trick of demuxing a keyboard with the
;	lightpen input.
;
;	See the Ubee technical manual for more information on scanning.
;
	    .area _CODE

	    .globl _kbscan
	    .globl _kbtest
	    .globl _lpen_kbd_last
;
_kbscan:
	    in a,(0x0C)
	    bit 6,a		; No light pen signal - no key
	    jr z, nokey		; Fast path exit

            ld a, (_lpen_kbd_last)	; Rather than the slow scan try and
            cp #255		; test if we are holding down the same key
            jr z, notlast	; assuming one was held down
            call ispressed	; see if the key is held down (usual case)
	    ld a, (_lpen_kbd_last)
				; if so return the last key
            jr nz, retkey
notlast:
	    ld l,#57		; Scan the keys in two banks, the first 57
	    ld de,#0x0000
	    call scanner
	    ld a,#57
	    jr nz, gotkey
            ld l,#5
	    ld de,#0x03A0	; and if that fails the last 5 oddities
	    call scanner	; in order to handle shift etc
            ld a,#63
	    jr nz, gotkey
nokey:
	    ld l,#255
	    ret
gotkey:
	    sub l
retkey:
	    ld l,a
	    ret

_kbtest:
	    pop hl
	    pop de
	    push de
	    push hl
	    ld a,e
;
;	Check if key is currently pressed
;	Split the key by matrix position and use our scanner
;
ispressed:
	    rrca
	    rrca
	    rrca
	    rrca
	    ld e,a
	    and #0x0F
	    ld d,a
            ld a,e
            and #0xF0
            ld e,a
            ld l,#1

;
;	Test for keys
;
;	On entry L is the number of keys to test
;	DE is the address to scan
;
;	Returns L holding the count of keys until we found a hit
;
;	Internal use of registers is as follows
;	A - scratch				A
;	B - low update register			E
;	C - 6545 control port			C
;	DE - from caller			HL
;	H - dummy register index		D
;	L - counter from caller			B
;
; Obscure Z80ism in a,(n) does not affect flags but in r,(c) does. We
; rely on this for the scanner and scanner exit code so don't mess with the
; use of in here as we have a cunning plan...
;
scanner:
	    ld a,#1
	    out (0x0b),a		; character ROM so we can scan
	    ld bc,#0x130c		; for convenience
	    ld h,#31			; port numbers used in the loop
	    ld a,#16			; select lpen high
            out (c),a
            in a,(0x0d)			; clear status
sethigh:    ld a,#18			; update register high
            out (c),a
            ld a,d
            out (0x0d),a		; load passed address
            ld a,e
            push de			; save working address
            ld d,#16
setlow:     out (c),b			; set the low byte from the passed address
	    out (0x0d),a
            out (c),h			; dummy reg to reset
            out (0x0d),a
strobe:     in e,(c)			; spin for a strobe
            jp p, strobe
	    ; for the below we could do in f,(c) jr nz but we are sticking
	    ; to documented instructions for the moment even if longer
            in e,(c)
            bit 6,e			; did we get a strobe ?
            jr nz,scanner_done
            dec l			; next key to test
            jr z,scanner_done		; are we done ?
            add a,d
            jp nz,setlow		; next inner scan
            pop de
            ld e,a
            inc d
            jp sethigh			; next outer scan
	    ;
	    ;	L holds the count remaining and thus computes the keycode
	    ;   0 means 'we found nothing'
	    ;   Also Z = no luck, NZ = got one
	    ;
scanner_done:
	    pop de			; recover position we are at
            ld a,#16			; lpen reset
            out (0x0c),a
            in a,(0x0d)			; VDU reset
            ld a,#0			; Not xor as we are preserving Z
            out (0x0b),a
	    ret


;
;	Video support code. This has to live below F000 so it can use the
;	video memory and not common. Note that this means we need to disable
;	interrupts briefly when we do the flipping
;
;	TODO: it would make sense to map the ROM and RAM font space and
;	copy the ROM font to RAM so we can support font setting ?
;

	    .area _VIDEO

	    .globl _cursor_off
	    .globl _cursor_disable
	    .globl _do_cursor_on
	    .globl _scroll_up
	    .globl _scroll_down
	    .globl _vwrite
	    .globl _map_video_font
	    .globl _unmap_video_font
	    .globl _video_40
	    .globl _video_80

	    .globl _int_disabled

	    .globl _vtwidth
	    .globl _vtaddr
	    .globl _vtcount
	    .globl _vtattrib
	    .globl _vtchar
;
;	6545 Hardware Cursor
;
_cursor_off:
	    ret
_cursor_disable:
	    ld a,(_int_disabled)
	    push af
	    di
	    ld bc,#0x0e0c	; Address register
	    out (c),b
	    ld bc,#0x000d	; Set to 0 will hide cursor nicely
				; as the 6545 sees video RAM as 0x2000+
	    out (c),b
	    jr popout
_do_cursor_on:
	    ld a, (_int_disabled)
	    push af
	    di
	    ld de, (_vtaddr)
	    ld c,#0x0d
	    ld a,#0x0e
	    out (0x0c),a
	    set 5,d		; As seen by the 6545 its 0x20xx
	    out (c),d
	    inc a
	    out (0x0c),a
	    out (c),e
popout:
	    pop af
	    or a
	    ret nz
	    ei
	    ret
;
;	Scroll the display (the memory wraps on a 2K boundary, the
;	display wraps on 2000 bytes). Soft scroll - hard scroll only works
;	in 64x16 mode
;
;	FIXME: remove hardcoding of 80x25 display size
;
_scroll_up:
	    ld a,(_int_disabled)
	    push af
	    di
	    ld a, (mapreg)
	    push af
	    and #0xF7		; enable video memory
	    or #0x10		; and put it at 0x8000
            ld (mapreg),a
	    out (0x50),a
	    ld hl,#0x8000
	    push hl
	    ld de, (_vtwidth)
	    add hl,de
	    pop de
	    ld bc,#1920		; FIXME
	    push bc
	    ldir
	    ld hl,#0x8800	; copy the colour
	    push hl
	    ld de, (_vtwidth)
	    add hl,de
	    pop de
	    pop bc
	    push bc
	    ldir
	    pop bc
	    ld a, (_ubee_model)
	    or a
	    jr z, unmap_out
	    ; and attribute RAM
	    ld a,#0x90
	    out (0x1c),a
	    ld hl,#0x8000
	    push hl
	    ld de, (_vtwidth)
	    add hl,de
	    pop de
	    ldir
	    ld a,#0x80
	    out (0x1c),a
unmap_out:
	    ; now put the RAM back
	    pop af
	    ld (mapreg),a
	    out (0x50),a
	    ; We usually only do one char
	    jr popout

;
;	FIXME: this needs rewriting to match scroll_up once scroll_up is
;	       right
;
_scroll_down:
	    ld a,(_int_disabled)
	    push af
	    di
	    ld a, (mapreg)
	    push af
	    and #0xF7		; enable video memory
	    or #0x10		; and put it at 0x8000
            ld (mapreg),a
	    out (0x50),a
	    ld hl,#0x87CF	; end of display
	    push hl
	    ld de, (_vtwidth)
	    or a
	    sbc hl,de
	    pop de
	    ex de,hl
	    ld bc,#1920		; FIXME compute for widths
	    push bc
	    lddr
	    pop bc
	    ld hl,#0x8FCF	; end of display
	    push hl
	    ld de, (_vtwidth)
	    or a
	    sbc hl,de
	    pop de
	    ex de,hl
	    push bc
	    lddr
	    pop bc
	    ld a, (_ubee_model)
	    or a
	    jr z, unmap_out
	    ; and attribute RAM
	    ld a,#0x90
	    out (0x1c),a
	    ld hl,#0x87CF
	    push hl
	    ld de, (_vtwidth)
	    add hl,de
	    pop de
	    ex de,hl
	    lddr
	    ld a,#0x80
	    out (0x1c),a
	    jr unmap_out
;
;	Write to the display
;
;	In theory we can avoid the di/ei but that needs some careful review
;	of the banking paths on interrupt
;
_vwrite:
	    ld a,(_int_disabled)
	    push af
	    di
	    ld a, (mapreg)
	    push af
	    and #0xF7		; enable video memory
	    or #0x10		; and put it at 0x8000
            ld (mapreg),a
	    out (0x50),a
	    ld hl,(_vtaddr)
	    ld bc,(_vtcount)
	    ld a,h
	    and #0x07
	    or #0x80
	    ld h,a
	    ld de,(_vtattrib)
vloop:
	    ld a,(_vtchar)
	    ld (hl),a		; character
	    ld a,(_ubee_model)
	    or a
	    jr z, noattrib
	    ld a,#0x90		; attribute RAM / 0x90 if we enable PCG extended
	    out (0x1c),a	; latch in attribute RAM
	    ld (hl),e		; attribute
	    xor a		; #0x80 if enable PCG extended
	    out (0x1c),a
noattrib:
	    set 3,h		; colour is at F8-FF
	    ld (hl),d		; colour
	    dec bc
	    ld a,b
	    or c
	    jr nz, nextchar
	    jp unmap_out
nextchar:
	    res 3,h
	    inc hl
	    jr vloop

_map_video_font:
	    ld a, (mapreg)
	    and #0xF7
	    or #0x10
	    di
	    out (0x50),a
	    xor a
	    out (0x08),a
	    ret
_unmap_video_font:
	    ld a,#0x40
	    out (0x08),a
	    ld a, (mapreg)
	    out (0x50),a
	    ei
	    ret
_video_40:
	    ld a,#1
	    jr _video_set
_video_80:
	    xor a
_video_set:
	    in a,(9)
	    ret
;
;	Ensure these are in the video mapping
;
_vtaddr:    .word 0
_vtcount:   .word 0
_vtattrib:  .word 0
_vtwidth:   .word 80			; FIXME should be variable
_vtchar:    .byte 0

;
;	Double speed upgrade
;
	    .globl _engage_warp_drive

_engage_warp_drive:
	    ld a,#2
	    in a,(9)
	    ret
