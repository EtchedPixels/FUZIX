    .area _BOOT (ABS)

.macro bios_parameter_block reserved_sectors
    .dw 512  ; bytes per sector
    .db 2    ; sectors per cluster
    .dw reserved_sectors
    .db 2    ; FAT count
    .dw 0x70 ; number of root directory entries
    .dw 144  ; filesystem size, in sectors
    .db 0xf9 ; media byte
    .dw 3    ; sectors per FAT
    .dw 9    ; number of sectors per track
    .dw 2    ; number of heads
    .dw 0    ; number of hidden sectors
.endm

    .org 0x0003
    .ascii "NCBOOT "

    .org 0x000b
    bios_parameter_block 2
    .org 0x020b
    bios_parameter_block 0

    .org 0x1be

    ; Partition 0
    .db 0          ; partition status (not bootable)
    .db 0, 2, 0    ; encoded CHS of start
    .db 0xda       ; partition type
    .db 1, 8, 7    ; encoded CHS of end
    .dw 1, 0       ; LBA of start
    .dw 0x8e, 0    ; LBA of end

    ; partition 1
    .db 0          ; partition status (not bootable)
    .db 1, 9, 7    ; encoded CHS of start
    .db 0x83       ; partition type
    .db 1, 9, 0x4f ; encoded CHS of end
    .dw 0x8f, 0    ; LBA of start
    .dw 0x511, 0   ; LBA of end

    .org 0x01fe
    .db 0x55, 0xaa
    
; We mark bad blocks in the cluster map because the Amstrad floppy disk routines
; have a bug in it which means that in bootable disks it's unable to read clusters
; which span tracks --- it applies the reserved sector offset *after* it calculates
; track/head/sector, which means that it thinks that some clusters occupy sectors
; 9 and 10 of a track, which doesn't work. We mark these as being inaccessible.
;
; The FAT entries for two cylinders (four physical tracks).
; That's 18*4 = 72 sectors = 0x12 clusters. But we are offset left
; by one sector, so clusters 0 and 5 span two tracks (and are inaccessible).
; Then the pattern is repeated again for the other two tracks.
.macro four_tracks
    .db 0xf7, 0x0f, 0x00 ; clusters 0, 1
    .db 0x00, 0x00, 0x00 ; clusters 2, 3
    .db 0x00, 0x70, 0xff ; clusters 4, 5
    .db 0x00, 0x00, 0x00 ; clusters 6, 7
    .db 0x00, 0x70, 0xff ; clusters 8, 9
    .db 0x00, 0x00, 0x00 ; clusters a, b
    .db 0x00, 0x00, 0x00 ; clusters c, d
    .db 0xf7, 0x0f, 0x00 ; clusters d, e
    .db 0x00, 0x00, 0x00 ; clusters f, 10
.endm

.macro fat_definition
    .db 0xf9, 0xff, 0xff
    .db 0x00, 0x70, 0xff ; clusters 2, 3
    .db 0x00, 0x00, 0x00 ; clusters 4, 5
    .db 0x00, 0x00, 0x00 ; clusters 6, 7
    .db 0xf7, 0x0f, 0x00 ; clusters 8, 9
    .db 0x00, 0x00, 0x00 ; clusters a, b

    four_tracks
    four_tracks
    four_tracks
.endm

    .org 0x400
    fat_definition
    .org 0xa00
    fat_definition

; Make an empty root directory.

    .org 0x1000
    .db 0
