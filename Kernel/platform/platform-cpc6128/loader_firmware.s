;;from: https://cpctech.cpc-live.com/source/byteload.html, modified to load something bigger

;; This example shows how to read a file byte by byte.
;;
;; A file without a header must be read this way, it can't be
;; read using CAS IN DIRECT (unless the in-memory header is patched)
;;
;; This example doesn't have any error checking.

.area BOOT (ABS)
.org 0xa500

cas_in_open .equ 0xbc77
cas_in_close .equ 0xbc7a
cas_in_char .equ 0xbc80
kl_rom_walk .equ 0xbccb
mc_start_program    .equ 0xbd16

ld c,#0xff
ld hl,#start
call mc_start_program
start:
ld hl,#0x0100				;; address to load file data to (example)
push hl

call kl_rom_walk
;; open file for reading
ld b,#end_filename-filename
ld hl,#filename
ld de,#two_k_buffer
call cas_in_open

;; If a file is opened without a header:
;; - the filetype will be ASCII (0x16)
;; - the length and load address will be undefined.
;;
;; If a file is opened with a header, the 
;; - the filetype will be taken from the header
;; - the length and load address will be taken from the header
;;
;; A file without a header can't be read with CAS IN DIRECT
;; and must be read using CAS IN CHAR.

pop hl

;; read a char from the file, character is returned in A register

next_byte:
call cas_in_char
jr nc,not_eof
jr nz,not_eof

;; could be end of file
;; test for hard end of file byte
cp #0xf
jr nz,not_eof
jr eof

not_eof:
;; write byte to memory
ld (hl),a
inc hl
ld a,#0xA5
cp h
jr z, avoid_crash
jr next_byte


eof:
call cas_in_close
ld de,#0xA500
ld hl,#0xC000
ld bc,#0x4000
ldir
jp 0x100 ;; and go to fuzix

avoid_crash:
ld b,#0x1b ;;0xc0-0xa5
ld c,#0
add hl,bc
jr next_byte

;;-------------------------------------------------------------------
;; name of the file to read

filename:
.ascii "FUZIX.BIN"
end_filename:

;;-------------------------------------------------------------------
;; this buffer is filled with data from the file
.org 0xF7FF
two_k_buffer: 
.blkb 2048