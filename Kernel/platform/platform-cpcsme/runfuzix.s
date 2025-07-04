.area BOOT (ABS)
.org #0x4000
    ld hl,#switch_to_fuzix
    ld de,#0x100+#switch_to_fuzix-#switch_to_fuzix_end
    ld bc,#switch_to_fuzix_end-#switch_to_fuzix
    push de
    ldir
    pop hl
    jp (hl)
switch_to_fuzix:
    ld bc,#0x7fc2
    out(c),c
switch_to_fuzix_end: