; 2015-01-21 Will Sowerbutts <will@sowerbutts.com>
;
; Boot sector for N8VEM Mark IV SBC with UNA BIOS, based on
; my UNA CP/M boot sector (2014-07-11)

        .module diskboot


; we are loaded at 0x8000
himem     = 0xFA00 ; our location in memory
buffer    = 0xF800 ; disk buffer (512 bytes)
stacktop  = 0xFE00 ; top of stack (512 bytes)

; UNA BIOS constants
UNABIOS_STUB_ENTRY          = 0xFFFD    ; main UNA entry vector
UNABIOS_BOOTHISTORY         = 0xFC      ; C register (subfunction in B)
UNABIOS_BOOT_GET            = 0x00      ;   B register (BOOTHISTORY subfunction)
UNABIOS_GETINFO             = 0xFA      ; C regsister (subfunction in B)
UNABIOS_GET_USER_PAGES      = 0x05      ;   B register (GETINFO subfunction)
UNABIOS_BANKEDMEM           = 0xFB      ; C register (subfunction in B)
UNABIOS_BANK_GET            = 0x00      ;   B register (BANKEDMEM subfunction)
UNABIOS_BANK_SET            = 0x01      ;   B register (BANKEDMEM subfunction)
UNABIOS_GET_HMA             = 0xF1      ; C register (subfunction in B)
UNABIOS_BLOCK_SETLBA        = 0x41      ; C register (unit number in B, 28-bit LBA in DEHL)
UNABIOS_BLOCK_READ          = 0x42      ; C register (unit number in B, buffer address in DE, sector count in L)
UNABIOS_OUTPUT_WRITE        = 0x12      ; C register (unit number in B)

        .area _LOADER (ABS)
        .org himem
start:
        ; UNA BIOS loads us from disk sector 0 at 0x8000
        jr gocopy                       ; we must start with a JP or JR instruction.
        .ds 0x40 - (.-start)            ; must leave room for floppy or partition superblock information

        ; Copy us up into high memory
gocopy: ld hl, #0x8000
        ld de, #himem
        ld bc, #512
        ldir
        jp go
        
        ; Executes in high memory
go:     ld sp, #stacktop                ; set inital stack
        ; write a character
        ld e, #0x5B                     ; '['
        call printchar

        ; determine the boot unit
        ld bc, #(UNABIOS_BOOT_GET << 8 | UNABIOS_BOOTHISTORY)
        call #UNABIOS_STUB_ENTRY
        ld a, l
        ld (unit), a                    ; save boot unit

        ; get the page number for the user memory bank
        ld bc, #(UNABIOS_GET_USER_PAGES << 8 | UNABIOS_GETINFO)
        call #UNABIOS_STUB_ENTRY
        ; returns EXEC_PAGE value in DE

        ; map in user memory bank
        ld bc, #(UNABIOS_BANK_SET << 8 | UNABIOS_BANKEDMEM)
        call #UNABIOS_STUB_ENTRY

        ; write unabios vector in user memory
        ld hl, #UNABIOS_STUB_ENTRY
        ld de, #0x0008
        ld bc, #3
        ldir

        ; wipe BDOS entry vector
        ld a, #0x76                     ; halt instruction
        ld (0x0005), a                  ; this is used as a marker to detect cold boot versus warm reload

        ; wipe persistent memory pointer (persist_ptr, immediately below UNA UBIOS stub / HMA)
        ld c, #UNABIOS_GET_HMA          ; get pointer to lowest byte used by UNA BIOS stub
        call #UNABIOS_STUB_ENTRY        ; returns lowest used byte in HL.
        xor a                           ; zero out the two bytes below that.
        dec hl
        ld (hl), a
        dec hl
        ld (hl), a

        ld a, (firstblock)
        ld (block), a

nextblock:
        ; print a = character only every other block
        ld a, (block)
        and #1
        jr z, setlba
        ld e, #0x3D                     ; '='
        call printchar

setlba: ; set LBA
        xor a
        ld d, a
        ld e, a
        ld h, a
        ld a, (block)
        ld l, a
        inc a               ; setup for next block now
        ld (block), a
        ld c, #UNABIOS_BLOCK_SETLBA
        ld a, (unit)
        ld b, a
        call #UNABIOS_STUB_ENTRY
        jr nz, error

        ; read block into buffer
        ld c, #UNABIOS_BLOCK_READ
        ld a, (unit)
        ld b, a
        ld l, #1
        ld de, #buffer
        call #UNABIOS_STUB_ENTRY
        jr nz, error

        ; copy block into low memory
        ld hl, #buffer
        ld de, (copyaddr)
        ld bc, #512
        ldir

        ld (copyaddr), de
        ld a, (firstblock)
        ld d, a
        ld a, (count)
        ld e, a
        ld a, (block)
        sub d
        cp e
        jr nz, nextblock

        ld e, #0x5D         ; ']'
        call printchar
        ld e, #0x0D
        call printchar
        ld e, #0x0A
        call printchar

        ; we're loaded. let's go.
        ld hl, (entryaddr)
        jp (hl)


; print a hex byte in A
byt_out:
        push af         ; save low nibble
        rrca            ; move high nibble into position
        rrca            ; **
        rrca
        rrca
        call nib_out    ; put out the high nibble
        pop af          ; fall into nib_out to put out low nibble
; print a hex-nibble in A
nib_out:
        and #0x0F       ; mask the nibble
        add #0          ; clear the AUX carry bit
        daa             ; decimal adjust the A
        add #0xF0       ; move hi-nib into carry, hi-nib is 0 or F
        adc #0x40       ; form ascii character
        ld  e, a
        ; fall through into printchar
printchar:
        ld bc, #UNABIOS_OUTPUT_WRITE
        jp UNABIOS_STUB_ENTRY

error:
        ; print the error
        ld a, c
        call byt_out
        ld e, a
        call printchar

        ; sad face :(
        ld e, #0x20     ; space
        call printchar
        ld e, #0x3A     ; ':'
        call printchar
        ld e, #0x28     ; '('
        call printchar

        ; park the vehicle
        halt

block:      .ds 1
unit:       .ds 1
firstblock: .db 2       ; start loading at sector 2
count:      .db 124     ; max sectors we can load before we overwrite ourselves (62KB)
copyaddr:   .dw 0x0088  ; load address
entryaddr:  .dw 0x0088  ; entry address

    .ds 0x1BE - ( . -start) ; pad to start of partition tables
    .db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ; partition 1
    .db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ; partition 2
    .db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ; partition 3
    .db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ; partition 4
    .dw 0xAA55              ; DOS boot signature
