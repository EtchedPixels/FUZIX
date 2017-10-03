; 2014-12-21 Will Sowerbutts

.module cpmload
.area _LOADER (ABS)

    ; CP/M will load us at 0x0100
    ; We want to relocate our payload (the kernel) and jump into it.
    ; We put a small stub at the very bottom of memory, copy the kernel into
    ; place above us, then jump into it.

    .org 0x100
    di
    ; copy ourselves to the very bottom of memory -- 0x00 upwards
    ld de, #0
    ld hl, #(doload)                ; start of loader code
    ld bc, #(endloader-doload)      ; length of our loader
    ldir                            ; copy copy copy!
    ld hl, (load_address)
    ld sp, hl                       ; stash copy of entry vector in SP for now
    ex de, hl                       ; load_address to de
    ld hl, #payload_start
    ld bc, (load_length)
    jp 0                            ; jump and perform copy in low memory

    ; this code gets copied to .org 0
doload:
    ldir                            ; copy image into correct place
    ld hl, #0
    add hl, sp                      ; recover entry vector
    jp (hl)                         ; run image
endloader:                          ; end of code to copy

    ; the data is in a trailer, with a 4-byte header:
load_address:
    .ds 2
load_length:
    .ds 2
payload_start:
