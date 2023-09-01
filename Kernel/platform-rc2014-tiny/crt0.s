; 2015-02-20 Sergey Kiselev
; 2013-12-18 William R Sowerbutts

        .module crt0

        ; Ordering of segments for the linker.
        ; WRS: Note we list all our segments here, even though
        ; we don't use them all, because their ordering is set
        ; when they are first seen.

	; Start with the ROM area CODE-CODE2
        .area _CODE
        .area _HOME     ; compiler stores __mullong etc in here if you use them
        .area _CODE2
	; Discard is loaded where process memory wil blow it away
        .area _DISCARD
	; The rest grows upwards from C000 starting with the udata so we can
	; swap in one block, ending with the buffers so they can expand up
        .area _COMMONMEM
        .area _CONST
        .area _INITIALIZED
        .area _DATA
        .area _BSEG
        .area _BSS
        .area _HEAP
        ; note that areas below here may be overwritten by the heap at runtime, so
        ; put initialisation stuff in here
        .area _BUFFERS     ; _BUFFERS grows to consume all before it (up to KERNTOP)
	; These get overwritten and don't matter
        .area _INITIALIZER ; binman copies this to the right place for us
        .area _GSINIT      ; unused
        .area _GSFINAL     ; unused

        ; exported symbols
        .globl init

        ; imported symbols
        .globl _fuzix_main
        .globl init_hardware
        .globl s__INITIALIZER
        .globl s__COMMONMEM
        .globl l__COMMONMEM
        .globl s__DISCARD
        .globl l__DISCARD
        .globl s__DATA
        .globl l__DATA
        .globl kstack_top

	.globl interrupt_handler
	.globl nmi_handler

	.globl ___sdcc_enter_ix

	.include "kernel.def"

        ; startup code
        .area _PAGE0(ABS)

.org	0

restart0:
	jp init
	.ds 5
restart8:
	jp	___sdcc_enter_ix
	.ds 5
restart10:
	ld	sp,ix
	pop	ix
	ret
	.ds 3
restart18:
	pop	af
	pop	ix
	ret
	.ds	4
restart20:
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	ret
	.ds	3
restart28:
	.ds 8
restart30:
	.ds 8
restart38:
	jp interrupt_handler
	.ds 5
; 0x40
	.ds 26
; 0x66
	jp nmi_handler
;
;	And 0x69 onwards we could use for code
;

	; Starts at 0x0080 at the moment

	.area _CODE

init:  
        di
	ld sp,#0x8200		; safe spot

	; Init the ATA CF
	; For now this is fairly dumb.
	; We ought to init the UART here first so we can say something
	; before loading.
	ld a,#0xE0
	out (0x16),a
	xor a
	out (0x14),a
	out (0x15),a
	; Set 8bit mode
	call wait_ready
	ld a, #1		; 8bit PIO
	out (0x11),a
	ld a, #0xEF		; SET FEATURES (8bit PIO)
	out (0x17),a
	call wait_ready

	; Load and map the rest of the image
	ld d,#1
	ld bc,#0x10		; c = data port  b = 0
	ld hl,#0x8200		; Load 8200-FFFF
loader:
	inc d
	call load_sector
	bit 6,d			; load 64 sectors 2-66
	jr z, loader

        ; switch to stack in high memory
        ld sp, #kstack_top

        ; Zero the data area
        ld hl, #s__DATA
        ld de, #s__DATA + 1
        ld bc, #l__DATA - 1
        ld (hl), #0
        ldir

        ; Hardware setup
        call init_hardware

        ; Call the C main routine
        call _fuzix_main
    
        ; fuzix_main() shouldn't return, but if it does...
        di
stop:   halt
        jr stop

;
;	Load sector d from disk into HL and advance HL accordingly
;
load_sector:
	ld a,d
	out (0x13),a		; LBA
	ld a,#1
	out (0x12),a		; 1 sector
	ld a,#0x20
	out (0x17),a		; command
	; Wait 
wait_drq:
	in a,(0x17)
	bit 3,a
	jr z, wait_drq
	; Get data, leave HL pointing to next byte, leaves B as 0 again
	inir
	inir
	ret
wait_ready:
	in a,(0x17)
	bit 6,a
	jr z,wait_ready
	ret

;
;       Stub helpers for code compactness.
;
;   Note that sdcc_enter_ix is in the standard compiler support already
;
;   The first two use an rst as a jump. In the reload sp case we don't
;   have to care. In the pop ix case for the function end we need to
;   drop the spare frame first, but we know that af contents don't
;   matter
;

___spixret:
	ld      sp,ix
	pop     ix
	ret
___ixret:
	pop     af
	pop     ix
	ret
___ldhlhl:
	ld      a,(hl)
	inc     hl
	ld      h,(hl)
	ld      l,a
	ret

