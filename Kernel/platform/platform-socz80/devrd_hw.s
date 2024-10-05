            .module devrd_hw

            ; imported symbols
            .globl _rd_dptr, _rd_dlen, _rd_address             ; from devrd.c
            .globl page17in, page17out                         ; from socz80.s

            ; exported symbols (used by devrd.c)
            .globl _ramdisk_read
            .globl _ramdisk_write

            .include "socz80.def"

            .area _CODE

_ramdisk_read:
            call ramdisk_setup
            call page17in
            ret

_ramdisk_write:
            call ramdisk_setup
            call page17out
            ret


; load MMU page17 to point to start of sector on RAM disk
; load DE with byte count
; load HL with destination pointer
ramdisk_setup:
            ld hl, (_rd_address)
            ld a, #MMU_SELECT_PAGE17
            out (MMU_SELECT), a
            xor a
            out (MMU_PTR_VAL0), a
            out (MMU_PTR_VAL3), a
            ld a, h
            out (MMU_PTR_VAL1), a
            ld a, l
            out (MMU_PTR_VAL2), a
            ld de, (_rd_dlen)
            ld hl, (_rd_dptr)
            ret
