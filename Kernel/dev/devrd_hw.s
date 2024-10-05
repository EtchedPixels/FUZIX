        .module devrd_hw

        ; imported symbols
        .globl _rd_transfer
        .globl _rd_reverse

        ; exported symbols (used by devrd.c)
        .globl _rd_read
        .globl _rd_write

        .include "../../cpu-z80/kernel-z80.def"
        .include "kernel.def"

        .area _CODE
; kernel calls rd_read(), rd_write(); we just set a flag and then pass control to rd_transfer()
_rd_write:
        ld a, #1
        jr _rd_go
_rd_read:
        xor a
_rd_go: ld (_rd_reverse), a
        jp _rd_transfer
