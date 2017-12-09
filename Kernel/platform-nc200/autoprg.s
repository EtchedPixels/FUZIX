.globl entry

; Amstrad kernel entrypoints

kmreadchar = 0xb9b3
textout = 0xb81e
txtclearwindow = 0xb824
txtoutput = 0xb833

; Ranger FDD entrypoints

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
    ; The only bank we can change while still keeping the Amstral kernel running
    ; from is the one at 0x4000. So we need to relocate ourself into the transient
    ; data area at 0xa000 to allow this. This is 4kB wide.

entry:
    ld de, #0xa000
    ld hl, #0x4000
    ld bc, #0x1000
    ldir
    jp start
start:
    call txtclearwindow
    ld hl, #.str.hello
    call textout

    ld hl, #.str.pattern
    ld a, #0xff
    call r_find_first
filename_loop:
    jp c, failed

    ld hl, (.data.dta)
    call textout
    ld a, #' '
    call txtoutput
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
    jp press_any_key

finished:
    ld a, (.data.found_a_file)
    or a
    jr z, no_files
    call r_finish
    jp press_any_key
no_files:
    ld hl, #.str.nofiles
    call textout
    jp press_any_key

nl:
    ld hl, #.str.nl
    jp textout

load_file:
    ld a, #1
    ld (.data.found_a_file), a

    ld hl, #0x4000
    ld (.data.destination), hl
    
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
    jr c, really_failed
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

.data.found_a_file:
    .db 0
.data.destination:
    .dw 0

.str.hello:
    .ascii "Loading..."
.str.nl:
    .ascii "\r\n"
    .db 0

.str.nofiles:
    .ascii "No files to load!"
    .db 0

.str.pattern:
    .ascii "BANK??.IMG"
    .db 0

.str.error:
    .ascii "\r\nError: "
    .db 0