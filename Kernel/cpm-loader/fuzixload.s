                .module fuzixload

                .area _LOADER (ABS)
                .org 0x100

start:          ; jump to start of code
                jp start2

                ; fuzix executable header --------------------------
                .db 'F'
                .db 'Z'
                .db 'X'
                .db '1'

                .db 0x01                ; page to load at
                .dw 0                   ; chmem ("0 - 'all'")
                .dw 0                   ; gives us code size info
                .dw 0                   ; gives us data size info
                .dw 0                   ; bss size info
                .dw 0                   ; spare
                ; --------------------------------------------------

msg:            .ascii 'booting ...\r\n'
endmsg:

start2: 
                ; sync()
                ld hl, #11              ; syscall #
                push hl
                rst #0x30               ; execute

                ; write(0, msg, strlen(msg));
                ld hl, #(endmsg-msg)    ; count
                push hl
                ld hl, #msg             ; buffer
                push hl
                ld hl, #0               ; fd
                push hl
                ld hl, #8               ; syscall #
                push hl
                rst #0x30               ; execute

                ; sync()
                ld hl, #11              ; syscall #
                push hl
                rst #0x30               ; execute

                di                      ; now we steal control of the machine from the old kernel!

                ld de, #0                       ; copy ourselves to bottom of RAM
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
endloader:                                      ; end of code to copy

                ; the data is in a trailer, with a 4-byte header:
load_address:
                .ds 2
load_length:
                .ds 2
payload_start:
