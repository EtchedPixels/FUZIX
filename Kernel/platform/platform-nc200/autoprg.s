.globl entry

; This is a simple bootstrap loader which uses the Amstrad's built-in software to
; load files off disk. It first searches for LOAD????.?? files, where the first
; field is a hex address and the second is a bank ID, and loads them all. Then it
; searches for a CALL????.?? file, pages in the bank, and calls that address.
; The paged in bank is always at 0x4000. Typically you'll want something like:
;
;   LOAD4000.80
;   LOAD4000.81
;   CALL4000.80
;
; If there are multiple CALL????.?? files, it will pick one at random and ignore
; the rest.

; Amstrad kernel entrypoints

kmreadchar = 0xb9b3
textout = 0xb81e
txtclearwindow = 0xb824
txtoutput = 0xb833

; Ranger FDD entrypoints. I have no idea whether these are consistent between ROM
; versions, if there are different ROM versions, but I've found at least two
; machines with these values. Unfortunately the Ranger software doesn't expose
; things like read_sector via a system call.

r_finish = 0xf34b
r_set_dta = 0xf391
r_find_first = 0xf396
r_find_next = 0xf3bf
.data.gp = 0xb7e0
.data.dta = 0xb7f1
load_sectors_to_4000 = 0xe884
print_hex_i8 = 0xcb0e
press_any_key = 0xcb50
advance_to_next_cluster = 0xd8ca
read_sector = 0xdffe
calculate_location_of_cluster = 0xda57
update_watchdog_timer = 0xcc78

    ; On entry, the memory map is as follows:
    ;
    ; 0x0000-0x3fff: Kernel workspace
    ; 0x4000-0x7ccc: Dedicated 16kB bank for us
    ; 0x8000-0xbfff: Kernel workspace
    ; 0xc000-0xffff: Ranger floppy disk routines
    ;
    ; The only bank we can change while still keeping the Amstrad kernel running
    ; from is the one at 0x4000. So we need to relocate ourself into the transient
    ; data area at 0xa000 to allow this. This is 4kB wide.

entry:
    ld de, #0xa000
    ld hl, #0x4000
    ld bc, #0x1000
    ldir
    jp start
start:
    ld (.data.entry_stack), sp
    call txtclearwindow
    ld hl, #.str.hello
    call textout

    ld hl, #.str.load_pattern
    ld a, #0xff
    call r_find_first
filename_loop:
    jp c, failed

    ld hl, (.data.dta)
    call textout
    ld a, #' '
    call txtoutput
    call parse_filename
    call load_file

    call r_find_next
    jr filename_loop

failed:
    cp #0x12 ; ran out of files?
    jr z, finished
    ; Otherwise it's a hard error.
really_failed:
    push af
    ld hl, #.str.error
    call textout
    pop af
    call print_hex_i8
press_any_key_and_exit:
    call press_any_key
exit:
    ld sp, (.data.entry_stack)
    ret

finished:
    ld a, (.data.found_a_file)
    or a
    jr z, no_bank_files

    ld hl, #.str.call_pattern
    ld a, #0xff
    call r_find_first
    jr c, no_boot_file
    call parse_filename

    push af
    ld hl, #.str.booting
    call textout
    pop af
    call print_hex_i8

    call r_finish
    ld sp, (.data.entry_stack)
    ld hl, (.data.destination)
    jp (hl)

no_bank_files:
    ld hl, #.str.no_bank_files
textout_and_exit:
    call textout
    jr press_any_key_and_exit
no_boot_file:
    ld hl, #.str.no_boot_file
    jr textout_and_exit

nl:
    ld hl, #.str.nl
    jp textout

; Assumes that the dta contains a dirent for a file and that .data.destination
; is set correctly; reads it all in.
load_file:
    ld a, #1
    ld (.data.found_a_file), a

    ld iy, (.data.dta)
    ld l, 0x1c(iy) ; start cluster in dirent
    ld h, 0x1d(iy)
cluster_loop:
    push hl
    call calculate_location_of_cluster
    ld 0x18(ix), a ; track
    ld 0x1a(ix), l ; sector

    ld hl, (.data.destination)
    call read_sector
    jr c, really_failed
    call update_watchdog_timer

    ld a, 0x1a(ix)
    inc a
    ld 0x1a(ix), a

    ld hl, (.data.destination)
    ld de, #0x200
    add hl, de
    ld (.data.destination), hl
    call read_sector
    jp c, really_failed
    call update_watchdog_timer

    ld hl, (.data.destination)
    ld de, #0x200
    add hl, de
    ld (.data.destination), hl
    
    ld a, #'.'
    call txtoutput

    pop hl
    call advance_to_next_cluster
    jr nc, cluster_loop
    jp nl

; Assumes that the dta contains the dirent for the current file.
parse_filename:
    ld iy, (.data.dta)

    ld d, 6(iy)
    ld e, 7(iy)
    call parse_hex
    ld (.data.destination+0), a

    ld d, 4(iy)
    ld e, 5(iy)
    call parse_hex
    ld (.data.destination+1), a

    ld d, 9(iy)
    ld e, 10(iy)
    call parse_hex
    ld (0xb001), a ; tell the OS that we're changing banks
    out (0x11), a  ; actually change banks
    ret

; Takes a hex number in D, E and reads it into A.
parse_hex:
    ld a,d
    call parse_hex_digit
    add a,a
    add a,a
    add a,a
    add a,a
    ld d,a
    ld a,e
    call parse_hex_digit
    or d
    ret

parse_hex_digit:
    sub a, #'0'
    cp #10
    ret c
    sub a, #'A'-'0'-10
    ret

.data.entry_stack:  .dw 0
.data.found_a_file: .db 0
.data.destination:  .dw 0

.str.hello:         .ascii "Loading..." ; falls through
.str.nl:            .asciz "\r\n"

.str.no_bank_files: .asciz "No files to load!"
.str.no_boot_file:  .asciz "No boot configuration!"
.str.load_pattern:  .asciz "LOAD????.??"
.str.call_pattern:  .asciz "CALL????.??"
.str.error:         .asciz "\r\nError: "
.str.booting:       .asciz "Starting bank "
