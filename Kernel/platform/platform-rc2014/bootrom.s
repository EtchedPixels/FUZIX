;
;	ROM boot for FUZIX on the RC2014
;
		.module bootrom
		.include "kernel.def"

		.area _LOADER (ABS)
		.org 0x0000
start:
		; map ROM page 0 to bank #0 and enable paging
		di			; better be safe than sorry
		xor a
		out (MPGSEL_0),a	; map page 0 (ROM) to bank #0
		ld a,#1
		out (MPGENA),a		; enable paging

		; copy FUZIX kernel to RAM
		; 4 pages, starting from ROM page 0, RAM page 32
		xor a
kernel_copy:
		out (MPGSEL_1),a	; map ROM page to bank #1
		add #32			; RAM page = ROM page + 32	
		out (MPGSEL_2),a	; map RAM page to bank #2
		ld hl,#0x4000		; source - bank #1 offset
		ld de,#0x8000		; destination - bank #2 offset
		ld bc,#0x4000		; count - 16 KiB
		ldir			; copy it
		sub a,#31		; next ROM page = RAM page - 32 + 1
		cp #4			; are we there yet (RAM page == 4?)
		jr nz,kernel_copy

;; 		; copy data to RAM disk
;; 		; 16 pages, starting from ROM page 4, RAM page 48
;; 		ld a,#4
;; ramdisk_copy:
;; 		out (MPGSEL_1),a	; map ROM page to bank #1
;; 		add #44			; RAM page = ROM page + 44
;; 		out (MPGSEL_2),a	; map RAM page to bank #2
;; 		ld hl,#0x4000		; source - bank #1 offset
;; 		ld de,#0x8000		; destination - bank #2 offset
;; 		ld bc,#0x4000		; count - 16 KiB
;; 		ldir			; copy it
;; 		sub #43			; next ROM page = RAM page - 44 + 1
;; 		cp #20			; are we there yet (RAM page == 4 + 16?)
;; 		jr nz,ramdisk_copy
;; 
		; scary... switching memory bank under our feet
		ld a,#32		; map page 32 (RAM) to bank 0 
		out (MPGSEL_0),a
		inc a			; map page 33 (RAM+16k) to bank 1
		out (MPGSEL_1),a
		inc a			; map page 34 (RAM+32K) to bank 2
		out (MPGSEL_2),a
		inc a			; map page 35 (RAM+48K) to bank 3
		out (MPGSEL_3),a

		jp 0x8B                 ; jump to init_from_rom in crt0
; pad
		.ds	(0x88-(.-start))
