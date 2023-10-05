; 2017-01-04 William R Sowerbutts

        .module devrd_hw
        .z180

        ; exported symbols
        .globl _rd_plt_copy
        .globl _rd_cpy_count
        .globl _rd_reverse
        .globl _rd_dst_userspace
        .globl _rd_dst_address
        .globl _rd_src_address
        .globl _devmem_read
        .globl _devmem_write

        .include "kernel.def"
        .include "../../cpu-z80/kernel-z80.def"
        .include "../cpu-z180/z180.def"

        .area _CODE
_devmem_write:
        ld a, #1
        ld (_rd_reverse), a             ; 1 = write
        jr _devmem_go

_devmem_read:
        xor a
        ld (_rd_reverse), a             ; 0 = read
        inc a
_devmem_go:
        ld (_rd_dst_userspace), a       ; 1 = userspace
        ; load the other parameters
        ld hl, (_udata + U_DATA__U_BASE)
        ld (_rd_dst_address), hl
        ld hl, (_udata + U_DATA__U_COUNT)
        ld (_rd_cpy_count), hl
        ld hl, (_udata + U_DATA__U_OFFSET)
        ld (_rd_src_address), hl
        ld hl, (_udata + U_DATA__U_OFFSET+2)
        ld (_rd_src_address+2), hl
        ; FALL THROUGH INTO _rd_plt_copy

;=========================================================================
; _rd_page_copy - Copy data from one physical page to another
; See notes in devrd.h for input parameters
; This code is Z180 specific so can safely use ld a,i
;=========================================================================
_rd_plt_copy:
        ; save interrupt flag on stack then disable interrupts
        ld a, i
        push af
        di

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
        ld hl,(_udata + U_DATA__U_PAGE)          ; load page number
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
        ; CPU stalls until DMA burst completes

        ; recover interrupt flag from stack and restore ints if required
        pop af
        ret po
        ei
        ret                     ; return with HL=_rd_cpy_count, as required by char device drivers

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
