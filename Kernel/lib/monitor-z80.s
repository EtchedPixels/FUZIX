; 2015-01-15 William R Sowerbutts
; A simple Z80 monitor program, mainly useful for inspecting machine state
;
; Based on the the socz80 ROM monitor program, and the UZI-180 monitor
; program, itself based on YM.MAC and M580 monitor.
;
; Commands:
;   D arg1 [arg2]  - display memory at address arg1 (to arg2)
;   I arg1         - input from port address arg1
;   O arg1 arg2    - output arg2 to port address arg1
;   J arg1         - jump to arg1 and execute code

                .z80

                ; exported symbols
                .globl monitor_entry

                ; imported symbols
                .globl inchar
                .globl outchar
                .globl outcharhex
                .globl outhl
                .globl outnewline
                .globl outstring

                ; NOTE: no .area declaration -- this file should be
                ; .include'd from platform code (see monitor.s in
                ; platform-p112 or platform-n8vem-mark4)

; expect to arrive here with return PC on stack
monitor_entry:  di                      ; turn off pesky interrupts
                ld hl, #0
                add hl, sp              ; save SP
                ld (entry_sp), hl       ; save to memory
                pop bc                  ; save PC
                ld sp, #monitor_stack
                push bc                 ; orig PC
                ex de,hl                ; orig SP -> DE
                ld hl, #entrystr1
                call outstring
                ex de,hl                ; orig SP -> HL
                call outhl
                ld hl, #entrystr2
                call outstring
                pop hl
                call outhl
monitor_loop:   call outnewline
                ld hl, #prompt
                call outstring
                ld hl, #linebuffer
                ld b, #(linebuffer_end-linebuffer) ; length
                call instring
                call outnewline
                call parse_args
                ld hl, #monitor_loop
                push hl ; return address
                ld hl,(arg1)
                ld de,(arg2)
                ;ld bc,(arg3)
                ld a, (linebuffer)
                cp #'D'
                jr z, dump
                cp #'I'
                jp z, inport
                cp #'O'
                jp z, outport
                cp #'J'
                jp z, jump
badinput:       ld a, #'?'
                jp outchar  ; ret back to monitor_loop

parse_args:     ld hl, #0
                ld (arg1), hl
                ld (arg2), hl
                ;ld (arg3), hl
                ld de, #linebuffer+1
                call gethex
                ld (arg1), hl
                ld (arg2), hl
                ret c
                call gethex
                ld (arg2), hl
                ret c
                ;call gethex
                ;ld (arg3), hl
                ;ret c
                ; more?!
                jr badinput

; D addr1,addr2
; Dump region addr1...addr2

dump:           call    out_addr
                push    hl
dmph:           ld      a,(hl)
                call    outbyte
                call    hl_eq_de
                jr      z,enddmp
                inc     hl
                ld      a,l
                and     #0x0F
                jr      nz,dmph
                pop     hl
                call    dumpl
                jr      dump
enddmp:         pop     hl
dumpl:          ld      a,#'|'
                call outchar
                ld      a,#' '
                call outchar
dumpln:         ld      a,(hl)
                cp      #0x20
                jr      c,outdot
                cp      #0x7f
                jr      c,char
outdot:         ld      a,#'.'
char:           call    outchar
                call    hl_eq_de
                ret     z
                inc     hl
                ld      a,l
                and     #0x0F
                jr      nz,dumpln
                ret

hl_eq_de:       ld      a,h
                cp      d
                ret     nz
                ld      a,l
                cp      e
                ret

; J addr
; Jump to addr and execute code
jump:           jp (hl)
                ; return address is already on the stack

; I port
; Input from port

inport:         push hl
                ld      a,l
                call    outcharhex
                ld      a,#'='
                call    outchar
                pop hl
                ld      c,l
                ld      b,h
                in      a,(c)
                call    outcharhex
                ret

; O port,byte
; Output to port

outport:        ld      c,l
                ld      b,h
                ld      a,e
                out     (c),a
                ret

out_addr:       call    outnewline
                call    outhl
                ld      a,#':'
                call    outchar
                ld      a,#' '
                jp      outchar

outbyte:        call    outcharhex
                ld      a,#' '
                jp      outchar

;cmd_debug:
;                ld hl, (arg1)
;                call outhl
;                call outnewline
;                ld hl, (arg2)
;                call outhl
;                call outnewline
;                ld hl, (arg3)
;                call outhl
;                call outnewline
;                jp monitor_loop


; parse 16-bit hex number at (DE) into HL
; return with carry flag set if end of string
gethex:         ld      hl,#0
ghskip:         ld      a,(de)
                cp      #' '
                jr      nz, gh1
                inc     de
                jr      ghskip
gh1:            ld      a,(de)
                inc     de
                or      a
                jp      z,aend
                cp      #','
                ret     z
                cp      #' '
                ret     z
                sub     #'0'
                jp      m,badinput
                cp      #10
                jp      m,dig
                cp      #0x11
                jp      m,badinput
                cp      #0x17
                jp      p,badinput
                sub     #7
dig:            ld      c,a
                ld      b,#0
                add     hl,hl
                add     hl,hl
                add     hl,hl
                add     hl,hl
                jp      c,badinput
                add     hl,bc
                jp      gh1
aend:           scf
                ret

; instring (from socz80, added buffer length checking, added case conversion)
; reads a string from terminal to memory. HL=buffer address, B=buffer length.
; Returns length of string (excluding 0 terminator) in C
instring:       ld c, #0        ; we use C to remember our string length
instringloop:   call inchar
                ; test for cr/lf
                cp #0x0d
                jr z, cr
                cp #0x0a
                jr z, cr
                ; test for backspace
                cp #0x08
                jr z, backspace
                cp #0x7f
                jr z, backspace
                ; test for non-printing characters
                cp #0x20 ; < 0x20?
                jp c, instringloop 
                cp #0x7e ; > 0x7e?
                jp nc, instringloop
                cp #'a'
                jr c, storechar
                cp #'z'+1
                jr nc, storechar
                and #0x5f ; uppercase: characters from a-z are now A-Z
storechar:      ; store the character in the buffer
                ld (hl), a
                inc hl
                inc c
                call outchar ; echo back the character typed
                ; test buffer length
                ld a, c
                sub b
                jr nz, instringloop ; space remains - next character
                ld a, #7
                call outchar ; ring the bell
                ; buffer full - fall through to backspace
backspace:      ld a, c
                cp #0
                jr z, instringloop ; cannot backspace past the start
                dec hl
                dec c
                ld a, #0x08 ; move back
                call outchar
                ld a, #0x20 ; print space
                call outchar
                ld a, #0x08 ; move back again
                call outchar
                jr instringloop
cr:             ld a, #0
                ld (hl), a
                ret


; strings
entrystr1:      .db 13, 10
                .ascii "monitor: SP="
                .db 0
entrystr2:      .ascii ", PC="
                .db 0
prompt:         .ascii ":( "
                .db 0

; arguments
arg1:           .ds 2
arg2:           .ds 2
;arg3:           .ds 2

; input buffer
linebuffer:     .ds 20
linebuffer_end:

; keep a record of the entry SP -- I always forget to write it down!
entry_sp:       .ds 2

; stack
                .ds 40
monitor_stack:
