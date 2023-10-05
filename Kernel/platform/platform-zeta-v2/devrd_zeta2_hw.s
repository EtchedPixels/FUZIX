        .module devrd_hw

        ; imported symbols
        .globl map_kernel, mpgsel_cache, _kernel_pages
        .globl _rd_plt_copy

        ; exported symbols
        .globl _rd_page_copy
        .globl _rd_cpy_count
        .globl _rd_reverse
        .globl _rd_dst_userspace
        .globl _rd_dst_address
        .globl _rd_src_address
        .globl _devmem_read
        .globl _devmem_write

        .include "kernel.def"
        .include "../../cpu-z80/kernel-z80.def"

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
        ld hl, (_udata + U_DATA__U_OFFSET)
        ld (_rd_src_address), hl
        ld hl, (_udata + U_DATA__U_OFFSET+2)
        ld (_rd_src_address+2), hl
        ld hl, (_udata + U_DATA__U_COUNT)
        ld (_rd_cpy_count), hl
        ; for single byte transfers we can optimise away the outer loop
        dec l                           ; test for HL=1
        ld a, h
        or l
        jp nz, _rd_plt_copy        ; > 1 byte, do it the hard way
        call _rd_page_copy              ; transfer single byte
        ld hl, #1                       ; return with HL set appropriately
        ret

        .area _COMMONMEM
;=========================================================================
; _rd_page_copy - Copy data from one physical page to another
; See notes in devrd.h for input parameters
;=========================================================================
_rd_page_copy:
        ; split rd_src_address into page and offset -- it's limited to 20 bits (max 0xFFFFF)
        ; example address 0x000ABCDE 
        ; in memory it is stored: DE BC 0A 00
        ; offset would be 0x0ABCDE & 0x3FFF = 0x3CDE
        ; page would be   0x0ABCDE >> 14    = 0x2A

        ; compute source page number
        ld a,(_rd_src_address+1)        ; load 0xBC -> B
        ld b, a
        ld a,(_rd_src_address+2)        ; load 0x0A -> A
        rl b                            ; grab the top bit into carry
        rla                             ; shift accumulator left, load carry bit at the bottom 
        rl b                            ; and again
        rla                             ; now A is the page number (0x2A)

        ; map source page
        ld (mpgsel_cache+1),a           ; save the mapping
        out (MPGSEL_1),a                ; map source page to bank #1

        ; compute source page offset, store in DE
        ld a,(_rd_src_address+1)
        and #0x3F                       ; mask to 16KB
        or #0x40                        ; add offset for bank 1
        ld d, a
        ld a,(_rd_src_address+0)
        ld e, a                         ; now offset is in DE

        ; compute destination page index (addr 0xABCD >> 14 = 0x02)
        ld a,(_rd_dst_address+1)        ; load top 8 bits
        and #0xc0                       ; mask off top 2 bits
        rlca                            ; rotate into lower 2 bits
        rlca
        ld b, #0
        ld c, a                         ; store in l

        ; look up page number
        ld a,(_rd_dst_userspace)        ; are we loading into userspace memory?
        or a
        jr nz, rd_translate_userspace
        ld hl, #_kernel_pages           ; get kernel page table
        jr rd_do_translate
rd_translate_userspace:
        ld hl, #_udata + U_DATA__U_PAGE          ; get user process page table
rd_do_translate:
        add hl, bc                      ; add index to base ptr (uint8_t *)
        ld a, (hl)                      ; load the page number from the page table

        ; map destination page
        ld (mpgsel_cache+2),a           ; save the mapping
        out (MPGSEL_2),a                ; map destination page to bank #2

        ; compute destination page offset, store in HL
        ld a,(_rd_dst_address+1)
        and #0x3F                       ; mask to 16KB
        or #0x80                        ; add offset for bank #2
        ld h, a
        ld a, (_rd_dst_address+0)
        ld l, a                         ; now offset is in HL

        ; load byte count
        ld bc,(_rd_cpy_count)           ; bytes to copy

        ; check if reversed
        ld a, (_rd_reverse)
        or a
        jr nz, go
        ex de,hl                        ; reverse if necessary
go:
        ldir                            ; do the copy
        jp map_kernel                   ; map back the kernel

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
