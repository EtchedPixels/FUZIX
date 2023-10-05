; 2015-01-19 Will Sowerbutts
; Simple P112 floppy bootloader for Fuzix
; Boots only from drive 0, floppy is used solely for kernel, one
; cannot have a filesystem on there as well.
;
; Assembles to a single sector 
;
; based on:
; Boot loader for P112 UZI180 floppy disks.
; Uses the ROM monitor Disk I/O routines.
; Copyright (C) 2001, Hector Peraza

        .module flopboot
        .z180

        .include "kernel.def"
        .include "../../cpu-z180/z180.def"

dparm       .equ    0x0B
loadaddr    .equ    0x8000  ; P112 bootstrap loads us here
himem       .equ    0xF800  ; We use F800 to approx FC00
stack       .equ    0xFE00  ; P112 ROM uses FE00 upwards
fuzix_entry .equ    0x88    ; Kernel must be loaded at 0x88 upwards
cmdline     .equ    0x80    ; CP/M command line area

        .area _LOADER (ABS)
        .org himem
start:

        ; P112 ROM loads us at 8000, we copy ourselves to the correct location
        ld      hl, #loadaddr
        ld      de, #himem
        ld      bc, #512
        ldir                        ; copy into high memory
        jp      loader              ; jump into new copy

loader:
        ld      sp, #stack          ; put SP somewhere safe
        ld      a,#0xF8             ; keep loader and BIOS data area mapped, with ROM in low 32K
        out0    (MMU_CBAR),a
        in0     a, (MMU_CBR)
        out0    (MMU_BBR), a
        in0     a,(Z182_SYSCONFIG)  
        set     3,a                 ; enable the BIOS ROM in case it was shadowed
        out0    (Z182_SYSCONFIG),a  
        ld      hl, #msg
        rst     #0x20
        ld      a, (sectors)       ; load number of sectors -- set by flopboot-mkimage
        ld      b, a
        ld      hl, #1              ; kernel starts in the second sector on the floppy
        ld      de, #fuzix_entry    ; load address in memory
loop:
        push    bc
        push    hl
        push    de
        call spin
        ld      ix,(dparm)
        call    xlate           ; translate block number to track and sector
        ld      a, #2           ; read command
        ld      b, #1           ; number of sectors
        ld      d, #0           ; drive 0
        ld      hl, #bfr
        rst     #0x08           ; P112 disk services
        jr      c, error
        ld      hl, #bfr
        pop     de              ; restore load address
        ld      bc, #512
        ld      a,#0xF0         ; RAM into low 32K
        out0    (MMU_CBAR),a
        ldir
        ld      a,#0xF8         ; ROM into low 32K
        out0    (MMU_CBAR),a
        pop     hl
        pop     bc
        ; setup for next block
        inc     hl
        djnz    loop
        ; completed loading
        ld hl, #donemsg
        rst #0x20
        in0     a,(Z182_SYSCONFIG)
        set     3,a             ; disable ROM
        out0    (Z182_SYSCONFIG),a
        ld      a,#0xF0         ; RAM into low 32K
        out0    (MMU_CBAR),a
        ; store an empty command line
        xor a
        ld      hl,#cmdline
        ld (hl), a
        inc hl
        ld (hl), a
        ; jump in to kernel code
        jp      fuzix_entry

msg:    .ascii "Loading ...  "
        .db 0

whirl:  .ascii "/-\|"

donemsg: .db 8, 32, 13
        .ascii "Booting ..."
        .db 13, 0

spin:
        push hl
        ld hl, #whirl
        ld a, b
        and #3
        add l
        ld l, a
        ld a, #8 ; backspace
        rst #0x18
        ld a, (hl)
        rst #0x18
        pop hl
        ret

error:
        ld      hl, #errmsg
        rst     #0x20
        rst     #0x38

errmsg: .ascii "Load error"
        .db 13, 10, 0

; input:  block number in HL
; output: track/side in C, sector in E
xlate:  ld      e,4(ix)        ; SPT
        call    div
        ld      c,l            ; track
        add     a,13(ix)       ; add 1st sector number
        ld      e,a
        ret

; HL/E = HL remainder in A
div:    ld      b,#16+1
        xor     a
div1:   adc     a,a
        sbc     a,e
        jr      nc,div0
        add     a,e
div0:   ccf
        adc     hl,hl
        djnz    div1
        ret

bfr:    ; next 512 bytes used as temporary buffer

space:  .ds (510-(.-start))
sectors:  .db 0
checksum: .db 0
.ifne (512-(.-start))
.error something has gone wrong, this should assemble to exactly 512 bytes.
.endif
