        .module vfdterm

        .include "kernel.def"
        .include "../kernel-z80.def"

        .globl  VFDTERM_PREINIT
        .globl  VFDTERM_PUTC

;------------------------
; VFD dumb terminal
;------------------------

VFDTERM_C0            .EQU   0
VFDTERM_D0            .EQU   1
VFDTERM_C1            .EQU   2
VFDTERM_D1            .EQU   3

                 .area _DATA
vfdPtr:          .DW     0
vfdLen:          .DB     0
vfdLine1:        .DW     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
vfdLine2:        .DW     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
vfdLine3:        .DW     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
vfdLine4:        .DW     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

vfdLineLen       .EQU    40


                 .area _CODE1
VFDTERM_PREINIT:
                  ; initialize first two lines
                  LD      A, #0x30
                  OUT     (VFDTERM_C0), A
                  LD      A, #0
                  OUT     (VFDTERM_D0), A
                  LD      A, #1
                  OUT     (VFDTERM_C0), A
                  LD      A, #0x0C            ; display on, cursor off
                  OUT     (VFDTERM_C0), A

                  ; initialize second two lines
                  LD      A, #0x30
                  OUT     (VFDTERM_C1), A
                  LD      A, #0
                  OUT     (VFDTERM_D1), A
                  LD      A, #1
                  OUT     (VFDTERM_C1), A
                  LD      A, #0x0F            ; display on, cursor on
                  OUT     (VFDTERM_C1), A

                  ; position to start of last line
                  CALL   VFDTERM_SOL
                  RET

                  ; assume character in E
VFDTERM_PUTC:     PUSH   AF
                  PUSH   BC
                  PUSH   DE
                  PUSH   HL

                  LD     A,E
                  CP     #0x08
                  JR     Z, VFDTERM_BSPC
                  CP     #0x0A                ; line feed
                  JR     Z, VFDTERM_NEWLINE
                  CP     #0x0D                ; CR
                  JR     Z, VFDTERM_CR

VFDTERM_STOREC:   LD     A, E               ; write the character to the vfd
                  OUT    (VFDTERM_D1), A

                  LD     HL, (vfdPtr)        ; update pointer
                  LD     (HL), E
                  INC    HL
                  LD     (vfdPtr), HL

                  LD     A, (vfdLen)        ; update position index
                  INC    A
                  LD     (vfdLen), A

                  LD     A, (vfdLen)        ; check if we just wrapped
                  CP     #vfdLineLen
                  JR     NZ, VFDTERM_PUTC_OUT

VFDTERM_NEWLINE:
                  CALL   VFDTERM_SCROLL
                  CALL   VFDTERM_REDRAW
                  CALL   VFDTERM_SOL
                  JR     VFDTERM_PUTC_OUT

VFDTERM_BSPC:     LD     A, (vfdLen)
                  CP     #0x00
                  JR     Z, VFDTERM_PUTC_OUT ; at beginning of buffer, no-op

                  DEC    A                   ; decrement line len
                  LD     (vfdLen), A
                  LD     HL, (vfdPtr)        ; decrement ptr
                  DEC    HL
                  LD     (vfdPtr), HL

                  LD     A, #0x10              ; shift cursor left one place
                  OUT    (VFDTERM_C1), A

                  JR     VFDTERM_PUTC_OUT

VFDTERM_CR:       CALL   VFDTERM_SOL
                  JR     VFDTERM_PUTC_OUT

VFDTERM_PUTC_OUT:
                  POP    HL
                  POP    DE
                  POP    BC
                  POP    AF
                  RET

                  ; scroll the buffer up one line
VFDTERM_SCROLL:
                  LD     HL, #vfdLine2
                  LD     DE, #vfdLine1
                  LD     BC, #120
                  LDIR                      ; do the move
                  LD     HL, #vfdLine4       ; clear the last line of the buffer
                  LD     BC, #vfdLineLen
                  LD     A, #' '
                  CALL   UTIL_FILL
                  RET

UTIL_FILL:
	LD	D,H		; SET DE TO HL
	LD	E,L		; SO DESTINATION EQUALS SOURCE
	LD	(HL),A		; FILL THE FIRST BYTE WITH DESIRED VALUE
	INC	DE		; INCREMENT DESTINATION
	DEC	BC		; DECREMENT THE COUNT
	LDIR			; DO THE REST
	RET			; RETURN

                  ; move pointer to start of 4th line
VFDTERM_SOL:
                  LD     HL, #vfdLine4
                  LD     (vfdPtr), HL
                  LD     A, #0
                  LD     (vfdLen), A
                  LD     A, #0xC0             ; command to move to start of line 2
                  OUT    (VFDTERM_C1), A    ; send to VFD
                  RET

VFDTERM_REDRAW:
                  LD     A, #0x80             ; command to move to start of line 1
                  OUT    (VFDTERM_C0), A
                  LD     DE, #vfdLine1
                  LD     C, #vfdLineLen
VFDTERM_REDRAW0:  LD     A, (DE)
                  INC    DE
                  OUT    (VFDTERM_D0), A
                  DEC    C
                  JR     NZ, VFDTERM_REDRAW0

                  LD     A, #0xC0             ; command to move to start of line 2
                  OUT    (VFDTERM_C0), A
                  LD     DE, #vfdLine2
                  LD     C, #vfdLineLen
VFDTERM_REDRAW1:  LD     A, (DE)
                  INC    DE
                  OUT    (VFDTERM_D0), A
                  DEC    C
                  JR     NZ, VFDTERM_REDRAW1

                  LD     A, #0x80             ; command to move to start of line 3
                  OUT    (VFDTERM_C1), A
                  LD     DE, #vfdLine3
                  LD     C, #vfdLineLen
VFDTERM_REDRAW2:  LD     A, (DE)
                  INC    DE
                  OUT    (VFDTERM_D1), A
                  DEC    C
                  JR     NZ, VFDTERM_REDRAW2

                  LD     A, #0xC0             ; command to move to start of line 4
                  OUT    (VFDTERM_C1), A
                  LD     DE, #vfdLine4
                  LD     C, #vfdLineLen
VFDTERM_REDRAW3:  LD     A, (DE)
                  INC    DE
                  OUT    (VFDTERM_D1), A
                  DEC    C
                  JR     NZ, VFDTERM_REDRAW3

                  RET



