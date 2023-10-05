	        ; Ordering of segments for the linker.
	        ; WRS: Note we list all our segments here, even though
	        ; we don't use them all, because their ordering is set
	        ; when they are first seen.	
	        .area _CODE
		.area _CODE1
	        .area _CODE2
		.area _VIDEO
	        .area _CONST
	        .area _INITIALIZED
	        .area _DATA
	        .area _BSEG
	        .area _BSS
	        .area _HEAP
	        ; note that areas below here may be overwritten by the heap at runtime, so
	        ; put initialisation stuff in here
	        .area _INITIALIZER
	        .area _GSINIT
	        .area _GSFINAL
	        .area _COMMONMEM
		.area _FONT
		.area _DISCARD

        	; imported symbols
        	.globl _fuzix_main
	        .globl init_early
	        .globl init_hardware
	        .globl s__DATA
	        .globl l__DATA
	        .globl s__FONT
	        .globl l__FONT
	        .globl s__DISCARD
	        .globl l__DISCARD
	        .globl s__COMMONMEM
	        .globl l__COMMONMEM
		.globl s__INITIALIZER
	        .globl kstack_top
		.globl _ramsize
		.globl _procmem
		.globl _msxmaps

		; Just for the benefit of the map file
		.globl start
		.globl enaslt
		.globl _slotram
		.globl _slotrom
		.globl _vdpport
		.globl _infobits
		.globl _machine_type

		.globl _discard_size

	        ; startup code @0x100
	        .area _CODE

		.include "msx2.def"

;
; Execution begins with us correctly mapped and at 0x0x100. However the
; 4000-7FFF bank may not be mapped to the mapper on entry.
;
; We assume here that the kernel packs below 48K for now we've got a few
; KBytes free but revisit as needed
;
; On entry the bootstrap stack holds the following we load into registers
;
;		IX	Number of 16K banks in the mapper
;		DE	D = RAM bank E = ROM bank
;		BC	MSX system info bits
;		HL	VDP Port Info
;		AF	Machine Type
;
;
		.ds 0x100
start:
		di
		ld a, #0x23
		out (OPENMSX_DEBUG1), a
		ld a, #'@'
		out (OPENMSX_DEBUG2), a
		;
		; unstash info bits and memory size
		;
		pop af
		pop hl
		pop bc
		pop de
		pop ix

		ld sp, #kstack_top
		;
		; set ram in slot_page1
		;
		ex af,af'
		ld a,d
		exx
		ld hl, #PAGE1_BASE
		call enaslt

		; move the common memory where it belongs
		ld hl, #s__DATA
		ld de, #s__COMMONMEM
		ld bc, #l__COMMONMEM
		ldir

		; move font where it belongs
		; FIXME: font6x8 is shifted the wrong way for msx vdp
		;        so correct it here for now
		ld de, #s__FONT
		ld bc, #l__FONT
cpfont:
		sla (hl)
		sla (hl)
		ldi
		ld a,b
		or c
		jr nz, cpfont

		; move the discardable memory where it belongs
		ld de, #s__DISCARD
		ld bc, #l__DISCARD
		ldir

		exx
		ex af,af'
		ld (_infobits),bc
		ld (_vdpport),hl
		ld (_machine_type),a
		; restore slot data
		ld a,d
		ld (_slotram),a
		ld a,e
		ld (_slotrom),a

		; then zero the data area
		ld hl, #s__DATA
		ld de, #s__DATA + 1
		ld bc, #l__DATA - 1
		ld (hl), #0
		ldir

		; finally update memory size
		;
		push ix
		pop hl
		ld (_msxmaps), hl
		add hl, hl			; x 16 for Kb
		add hl, hl
		add hl, hl
		add hl, hl

		; set system RAM size in KB
		ld (_ramsize), hl
		ld de, #0xFFD0
		add hl, de			; subtract 48K for the kernel
		ld (_procmem), hl

		call init_early
		call init_hardware
		call _fuzix_main
		di
stop:		halt
		jr stop

		.area _DISCARD
_discard_size:
		.dw l__DISCARD
