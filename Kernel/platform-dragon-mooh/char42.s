********************************************************************
*
* Taken from NitrOS-9 level1/modules/co42.asm 2018-08-18
*
* Co42 - Hi-Res 42x24 Graphics Console Output Subroutine for VTIO
* Based from CoHR
*
* $Id$
*
* Edt/Rev  YYYY/MM/DD  Modified by
* Comment
* ------------------------------------------------------------------
*   1      ????/??/??
* Original Dragon distribution version
*
*          2003/09/22  Rodney Hamilton
* Recoded fcb arrays, added labels & some comments
*
*          2004/11/15  P.Harvey-Smith
* Added code to turn off the drives on the Dragon Alpha.
*
*	   2004/12/01  P.Harvey-Smith
* Began converting drvr51 to CoHR, removed all keyboard
* related code, added symbolic defines for a lot of things.
*
*          2004/12/02  P.Harvey-Smith
* Finished converting to c051 driver, moved all variable
* storage into ccio module (defined in cciodefs).
*
*          2005/04/09  P.Harvey-Smith
* Replaced all ; comment chars with * for benefit of native
* asm. Re-implemented (hopefully) non-destructive cursor which
* is XORed onto the screen. Commented character drawing routines
* and replaced the V51xx names with more meaningful ones.
*
*          2005/04/24  P.Harvey-Smith
* Addded routines to flash the cursor, this is as it was in the
* Dragon Data 51 column driver.
*
*          2017/04/23  Felipe Antoniosi
* Create this driver as 42x24 column
*
*          2018/01/20  David Ladd
* Moved Driver Entry Table closer to Term to allow fall through.
* This is to save bytes and cycles.  Changed lbra Write to bra
* Write to save cyrcles each time characters are written to screen.
* Also changed lda and ldb to a ldd to save cycle(s) and space.
* Also a few other optimizations to code.
*
*          2018/08/18  Tormod Volden
* Extract DrawCharacter routine for use in FUZIX
*

	.module char42

	.globl _m6847_plot_char_42
	.globl _m6847_cursor_on_42
	.globl _m6847_cursor_off_42

	include "kernel.def"

ScreenSize equ  $1800     ; Screen Size in Bytes

	.area .videodata

ScreenMask	.dw 0
BytePixOffset	.db 0
XORFlag		.db 0
CursorChanged	.db 0
ReverseFlag	.db 1     ; are we in reverse mode ?
CursorSave	.dw 0

	.area .video

* FIXME clear_across is missing

* void m6847_cursor_on(int8_t newy, int8_t newx);

_m6847_cursor_on_42:
	pshs y,b          ; newy
	lda  5,s          ; newx
	std  CursorSave
	ldb  #$7f
	inc  XORFlag
	lbsr DrawCharacter ; stack: newx C-pc y newy pc
	dec  XORFlag
	puls b,y,pc


* void m6847_cursor_off(void);

_m6847_cursor_off_42:
	ldd  CursorSave
	pshs a
	pshs u,y,b
	ldb  #$7f
	inc  XORFlag
	lbsr DrawCharacter ; stack: C-pc oldx u y oldy pc
	dec  XORFlag
	puls b,y,u
	puls a,pc


* called as plot_char(int8_t y, int8_t x, uint16_t c)

_m6847_plot_char_42:
	pshs y,b          ; Ypos
	tfr  x,d          ; character now in b
	bsr  DrawCharacter
	puls b,y,pc

*
* Draw the normal characters $20..$7f, in the b register
* Stack: Xpos C-pc y Ypos pc

DrawCharacter
         lda   #$3F
         sta   $FFA0      ; unmap kernel in video memory area
         subb  #$20       ; Make b an offset into table
         lda   #5
         ldx   #CharacterShapes   ; point to character shape table
         mul              ; Multiply by 5 (5 bytes / character)
         leax  d,x        ; Point X at required character's bitmap
         ldb   #$06       ; Work out pixel X co-ordinate of current cursor
         lda   7,s        ; Xpos
         mul
         pshs  b          ; Save pixel x
         lsra             ; Divide pixel-x by 8, to get byte offset into line
         rorb
         lsra
         rorb
         lsra
         rorb
         lda   ,s
         anda  #$07       ; Calculate offset within byte where character begins
         stb   ,s
*         puls  a          ; restore pixel X
*         anda  #$07       ; Calculate offset within byte where character begins
*         pshs  b
         sta   BytePixOffset
         tst   XORFlag
         bne   L01FF
         tfr   a,b        ; Calculate a mask for character data
         lda   #$FC       ; shifts $fc right b times
         tstb
         beq   L01FA      ; Done all bits ?
L01E5    lsra             ; shift mask right
         decb             ; decrement count
         bhi   L01E5      ; done all ?
         bne   L01EE      ; have we shifted any mask bits off right hand end ?
         rorb
         bra   L01FA

L01EE    pshs  b          ; Save count on stack
         ldb   #$80       ; start to build mask for second byte as well
L01F2    lsra             ; shift bits from bottom of a to top of b
         rorb
         dec   ,s         ; decrement count
         bne   L01F2      ; if any shifts left loop again
         leas  1,s        ; drop count

* When we reach here we should have a pair of bytes in d which indicate where exactly the
* character should be drawn, this may be partly in each

L01FA    coma
         comb
         std   ScreenMask         ; Save screen mask

* The code below works out the offset of the character cell to be updated, this works because
* the y coordinate is loaded into the high byte of d, effectively multiplying it by 256, since
* each screen line is 32 bytes wide, and each character is 8 pixels tall this works out as 8x32=256

L01FF    ldy   #VIDEO_BASE        ; Point y at screen memory address
         lda   3,s        ; YPos
         ldb   ,s+        ; Retrieve byte offset from stack
         leay  d,y        ; calculate screen address.
*         lda   #$08       ; get character data byte count, 8 bytes
*         pshs  a
         inc   CursorChanged      ; flag character at cursor being changed

* The caracters are packed in 5-bytes following the order
* 00000111
* 11222223
* 33334444
* 45555566
* 66677777

L0211    lda   ,x         ; row 0
         anda  #$F8       ; mask out character
         bsr   L0236      ; update screen

         lda   ,x+        ; row 1
         ldb   ,x
         lsra
         rorb
         lsra
         rorb
         lsra
         rorb
         tfr   b,a
         anda  #$F8       ; mask out character
         bsr   L0236      ; update screen

         lda   ,x         ; row 2
         lsla
         lsla
         anda  #$F8       ; mask out character
         bsr   L0236      ; update screen

         lda   ,x+        ; row 3
         ldb   ,x
         lsra
         rorb
         tfr   b,a
         anda  #$F8       ; mask out character
         bsr   L0236      ; update screen

         lda   ,x+        ; row 4
         ldb   ,x
         lslb
         rola
         lslb
         rola
         lslb
         rola
         lslb
         rola
         anda  #$F8       ; mask out character
         bsr   L0236      ; update screen

         lda   ,x         ; row 5
         lsla
         anda  #$F8       ; mask out character
         bsr   L0236      ; update screen

         lda   ,x+        ; row 6
         ldb   ,x
         lsra
         rorb
         lsra
         rorb
         tfr   b,a
         anda  #$F8       ; mask out character
         bsr   L0236      ; update screen

         lda   ,x         ; row 7
         lsla
         lsla
         lsla
         anda  #$F8       ; mask out character
         bsr   L0236      ; update screen

*         dec   ,s         ; Decrement character data byte counter
*         bne   L0211      ; all done ?
         dec   CursorChanged      ; Flag character update finished
         lda   #4
         sta   $ffA0	  ; remap kernel
         rts


*L0227    ldb   BytePixOffset
*         subb  #$04
*         bhi   L023B
*         beq   L0250
*L0230    lsla
*         incb
*         bne   L0230
*         bra   L0250

L0236    ldb   BytePixOffset      ; Retrieve byte pixel offset
         beq   L0250

L023B    lsra             ; manipulate character data into correct position
         decb             ; in a similar way to the mask above
         bhi   L023B
         bne   L0244
         rorb
         bra   L0250
L0244    pshs  b
         ldb   #$80
L0248    lsra
         rorb
         dec   ,s
         bne   L0248
         leas  1,s

L0250    tst   XORFlag     ; are we XORing data direct to screen ?
         bne   L0273       ; Yes : just do it
         tst   ReverseFlag ; are we in reverse mode ?
         beq   L0262       ; no : just output data
         coma              ; set mask up for reverse mode
         comb
         eora  ScreenMask
         eorb  ScreenMask+1

L0262    pshs  b,a        ; combine mask and screen data
         ldd   ScreenMask
         anda  ,y
         andb  $01,y
         addd  ,s++

L026D    std   ,y         ; screen update
         leay  <$20,y
         rts

L0273    eora  ,y         ; XOR onto screen
         eorb  $01,y
         bra   L026D

         puls  pc,b

	.area .videodata

CharacterShapes

	fcb   $00,$00,$00,$00,$00	; ' '
	fcb   $21,$08,$40,$00,$80	; '!'
	fcb   $52,$94,$00,$00,$00	; '"'
	fcb   $52,$be,$af,$a9,$40	; '#'
	fcb   $23,$e8,$e2,$f8,$80	; '$'
	fcb   $c6,$44,$44,$4c,$60	; '%'
	fcb   $45,$11,$59,$4d,$80	; '&'
	fcb   $11,$10,$00,$00,$00	; '''
	fcb   $11,$10,$84,$10,$40	; '('
	fcb   $41,$04,$21,$11,$00	; ')'
	fcb   $25,$5c,$47,$54,$80	; '*'
	fcb   $01,$09,$f2,$10,$00	; '+'
	fcb   $00,$00,$00,$10,$88	; ','
	fcb   $00,$00,$f0,$00,$00	; '-'
	fcb   $00,$00,$00,$31,$80	; '.'
	fcb   $00,$02,$22,$22,$00	; '/'
	fcb   $74,$67,$5c,$c5,$c0	; '0'
	fcb   $23,$28,$42,$13,$e0	; '1'
	fcb   $74,$42,$26,$43,$e0	; '2'
	fcb   $74,$42,$60,$c5,$c0	; '3'
	fcb   $11,$95,$2f,$88,$40	; '4'
	fcb   $fc,$38,$20,$8b,$80	; '5'
	fcb   $32,$21,$e8,$c5,$c0	; '6'
	fcb   $fc,$44,$42,$10,$80	; '7'
	fcb   $74,$62,$e8,$c5,$c0	; '8'
	fcb   $74,$62,$f0,$89,$80	; '9'
	fcb   $00,$08,$00,$10,$00	; ':'
	fcb   $00,$08,$00,$10,$88	; ';'
	fcb   $19,$99,$86,$18,$60	; '<'
	fcb   $00,$3e,$0f,$80,$00	; '='
	fcb   $c3,$0c,$33,$33,$00	; '>'
	fcb   $74,$42,$22,$00,$80	; '?'
	fcb   $74,$42,$da,$d5,$c0	; '@'
	fcb   $22,$a3,$1f,$c6,$20	; 'A'
	fcb   $f2,$52,$e4,$a7,$c0	; 'B'
	fcb   $32,$61,$08,$24,$c0	; 'C'
	fcb   $e2,$92,$94,$ab,$80	; 'D'
	fcb   $fc,$21,$e8,$43,$e0	; 'E'
	fcb   $fc,$21,$e8,$42,$00	; 'F'
	fcb   $74,$61,$78,$c5,$c0	; 'G'
	fcb   $8c,$63,$f8,$c6,$20	; 'H'
	fcb   $71,$08,$42,$11,$c0	; 'I'
	fcb   $38,$84,$29,$49,$80	; 'J'
	fcb   $8c,$a9,$8a,$4a,$20	; 'K'
	fcb   $84,$21,$08,$43,$e0	; 'L'
	fcb   $8e,$eb,$58,$c6,$20	; 'M'
	fcb   $8e,$73,$59,$ce,$20	; 'N'
	fcb   $74,$63,$18,$c5,$c0	; 'O'
	fcb   $f4,$63,$e8,$42,$00	; 'P'
	fcb   $74,$63,$1a,$c9,$a0	; 'Q'
	fcb   $f4,$63,$ea,$4a,$20	; 'R'
	fcb   $74,$60,$e0,$c5,$c0	; 'S'
	fcb   $f9,$08,$42,$10,$80	; 'T'
	fcb   $8c,$63,$18,$c5,$c0	; 'U'
	fcb   $8c,$63,$15,$28,$80	; 'V'
	fcb   $8c,$63,$5a,$ee,$20	; 'W'
	fcb   $8c,$54,$45,$46,$20	; 'X'
	fcb   $8c,$62,$e2,$10,$80	; 'Y'
	fcb   $f8,$44,$44,$43,$e0	; 'Z'
	fcb   $72,$10,$84,$21,$c0	; '['
	fcb   $00,$20,$82,$08,$20	; '\'
	fcb   $70,$84,$21,$09,$c0	; ']'
	fcb   $22,$a2,$00,$00,$00	; '^'
	fcb   $00,$00,$00,$03,$e0	; '_'
	fcb   $41,$04,$00,$00,$00	; '`'
	fcb   $00,$1c,$17,$c5,$e0	; 'a'
	fcb   $84,$2d,$98,$e6,$c0	; 'b'
	fcb   $00,$1d,$18,$45,$c0	; 'c'
	fcb   $08,$5b,$38,$cd,$a0	; 'd'
	fcb   $00,$1d,$1f,$c1,$c0	; 'e'
	fcb   $11,$49,$f2,$10,$80	; 'f'
	fcb   $00,$1b,$39,$b4,$2e	; 'g'
	fcb   $84,$3d,$18,$c6,$20	; 'h'
	fcb   $20,$18,$42,$11,$c0	; 'i'
	fcb   $10,$0c,$21,$0a,$4c	; 'j'
	fcb   $42,$12,$a6,$29,$20	; 'k'
	fcb   $61,$08,$42,$11,$c0	; 'l'
	fcb   $00,$35,$5a,$d6,$a0	; 'm'
	fcb   $00,$2d,$98,$c6,$20	; 'n'
	fcb   $00,$1d,$18,$c5,$c0	; 'o'
	fcb   $00,$2d,$9c,$da,$10	; 'p'
	fcb   $00,$1b,$39,$b4,$21	; 'q'
	fcb   $00,$2d,$98,$42,$00	; 'r'
	fcb   $00,$1f,$0f,$07,$c0	; 's'
	fcb   $42,$3c,$84,$24,$c0	; 't'
	fcb   $00,$25,$29,$49,$a0	; 'u'
	fcb   $00,$23,$18,$a8,$80	; 'v'
	fcb   $00,$23,$5a,$d5,$40	; 'w'
	fcb   $00,$22,$a2,$2a,$20	; 'x'
	fcb   $00,$23,$19,$b4,$2e	; 'y'
	fcb   $00,$3e,$22,$23,$e0	; 'z'
	fcb   $19,$08,$82,$10,$60	; '{'
	fcb   $21,$08,$02,$10,$80	; '|'
	fcb   $c1,$08,$22,$13,$00	; '}'
	fcb   $45,$44,$00,$00,$00	; '~'
	fcb   $ff,$ff,$ff,$ff,$ff	; 'cursor'

