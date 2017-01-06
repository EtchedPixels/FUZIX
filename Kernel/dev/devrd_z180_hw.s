; 2017-01-04 William R Sowerbutts

        .module devrd_hw
        .z180

        ; imported symbols
        .globl map_kernel
        .globl _rd_transfer

        ; exported symbols (used by devrd.c)
        .globl _rd_page_copy
        .globl _rd_read
        .globl _rd_write
        .globl _rd_cpy_count, _rd_reverse
        .globl _rd_dst_userspace, _rd_dst_address, _rd_src_address

        .include "kernel.def"
        .include "../kernel.def"
        .include "../cpu-z180/z180.def"

        .area _CODE
; kernel calls rd_read(), rd_write(), we just set a flag and then pass control to rd_transfer()
_rd_write:
        ld a, #1
        jr _rd_go
_rd_read:
        xor a
_rd_go: ld (_rd_reverse), a
        jp _rd_transfer

;=========================================================================
; _rd_page_copy - Copy data from one physical page to another
; Call with interrupts disabled
; See notes in devrd.h for input parameters
;=========================================================================
_rd_page_copy:
        ; load source page number
        ld de, (_rd_src_address+1) ; and +2
        ld a,  (_rd_src_address+0)
        ld b, a

        ; compute destination
        ld a,(_rd_dst_userspace)        ; are we loading into userspace memory?
        or a
        jr nz, rd_translate_userspace
        ld hl, #(OS_BANK + FIRST_RAM_BANK) << 4
        jr rd_done_translate
rd_translate_userspace:
        ld hl,(U_DATA__U_PAGE)          ; load page number
        add hl, hl                      ; shift left 4 bits
        add hl, hl
        add hl, hl
        add hl, hl
rd_done_translate:
        ; add in page offset
        ld a,(_rd_dst_address+1)        ; top 8 bits of address
        add a, l
        ld l, a
        adc a, h
        sub l
        ld h, a                         ; result in hl
        ld a,(_rd_dst_address+0)
        ld c, a

        ld a,(_rd_reverse)
        or a
        jr z,not_reversed

        ex de, hl
        out0 (DMA_SAR0L),c
        out0 (DMA_DAR0L),b
        jr topbits
not_reversed:
        out0 (DMA_SAR0L),b
        out0 (DMA_DAR0L),c
topbits:
        out0 (DMA_SAR0B),d
        out0 (DMA_SAR0H),e
        out0 (DMA_DAR0B),h
        out0 (DMA_DAR0H),l

        ld hl,(_rd_cpy_count)
        out0 (DMA_BCR0L),l
        out0 (DMA_BCR0H),h

        ; make dma go
        ld bc, #0x0240
        out0 (DMA_DMODE), b     ; 0x02 - memory to memory, burst mode
        out0 (DMA_DSTAT), c     ; 0x40 - enable DMA channel 0

        ret

; variables
_rd_cpy_count:
        .dw     0                       ; uint16_t
_rd_reverse:
        .db     0                       ; bool
_rd_dst_userspace:
        .db     0                       ; bool
_rd_dst_address:
        .dw     0                       ; uint16_t
_rd_src_address:
        .db     0                       ; uint32_t
        .db     0
        .db     0
        .db     0
;=========================================================================
