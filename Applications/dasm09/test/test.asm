;
; 6809 assembler/disassembler test file (08 Aug 1999)
;

     abx

     adca <$01
     adca $0101
     adca #$01
     adca ,x

     adcb <$01
     adcb $0101
     adcb #$01
     adcb ,x

     adda <$01
     adda $0101
     adda #$01
     adda ,x

     addb <$01
     addb $0101
     addb #$01
     addb ,x

     addd <$01
     addd $0101
     addd #$0101
     addd ,x

     anda <$01
     anda $0101
     anda #$01
     anda ,x

     andb <$01
     andb $0101
     andb #$01
     andb ,x

     andcc #$01

     asla

     aslb

     asl <$01
     asl $0101
     asl ,x

     asra

     asrb

     asr <$01
     asr $0101
     asr ,x

sloop
     bcc sloop
     lbcc lloop

     bcs sloop
     lbcs lloop

     beq sloop
     lbeq lloop

     bge sloop
     lbge lloop

     bgt sloop
     lbgt lloop

     bhi sloop
     lbhi lloop

     bhs sloop
     lbhs lloop

     bita <$01
     bita $0101
     bita #$01
     bita ,x

     bitb <$01
     bitb $0101
     bitb #$01
     bitb ,x

     ble sloop
     lble lloop

     blo sloop
     lblo lloop

     bls sloop
     lbls lloop

     blt sloop
     lblt lloop

     bmi sloop
     lbmi lloop

     bne sloop
     lbne lloop

     bpl sloop
     lbpl lloop

     bra sloop
     lbra lloop

     brn sloop
     lbrn lloop

     bsr sloop
     lbsr lloop

     bvc sloop
     lbvc lloop

     bvs sloop
     lbvs lloop

     clra

     clrb

     clr <$01
     clr $0101
     clr ,x

     cmpa <$01
     cmpa $0101
     cmpa #$01
     cmpa ,x

     cmpb <$01
     cmpb $0101
     cmpb #$01
     cmpb ,x

     cmpd <$01
     cmpd $0101
     cmpd #$0101
     cmpd ,x

     cmps <$01
     cmps $0101
     cmps #$0101
     cmps ,x

     cmpu <$01
     cmpu $0101
     cmpu #$0101
     cmpu ,x

     cmpx <$01
     cmpx $0101
     cmpx #$0101
     cmpx ,x

     cmpy <$01
     cmpy $0101
     cmpy #$0101
     cmpy ,x

     coma

     comb

     com <$01
     com $0101
     com ,x

     cwai #$FF

     daa

     deca

     decb

     dec <$01
     dec $0101
     dec ,x

     eora <$01
     eora $0101
     eora #$01
     eora ,x

     eorb <$01
     eorb $0101
     eorb #$01
     eorb ,x

     exg a,a
     exg a,b
     exg a,cc
     exg a,dp

     exg b,a
     exg b,b
     exg b,cc
     exg b,dp

     exg cc,a
     exg cc,b
     exg cc,cc
     exg cc,dp

     exg dp,a
     exg dp,b
     exg dp,cc
     exg dp,dp

     exg d,d
     exg d,s
     exg d,u
     exg d,x
     exg d,y
     exg d,pc

     exg s,d
     exg s,s
     exg s,u
     exg s,x
     exg s,y
     exg s,pc

     exg u,d
     exg u,s
     exg u,u
     exg u,x
     exg u,y
     exg u,pc

     exg x,d
     exg x,s
     exg x,u
     exg x,x
     exg x,y
     exg x,pc

     exg y,d
     exg y,s
     exg y,u
     exg y,x
     exg y,y
     exg y,pc

     exg pc,d
     exg pc,s
     exg pc,u
     exg pc,x
     exg pc,y
     exg pc,pc

     inca

     incb

     inc <$01
     inc $0101
     inc ,x

     jmp <$01
     jmp $0101
     jmp ,x

     jsr <$01
     jsr $0101
     jsr ,x

     lda <$01
     lda $0101
     lda #$01
     lda ,x

     ldb <$01
     ldb $0101
     ldb #$01
     ldb ,x

     ldd <$01
     ldd $0101
     ldd #$0101
     ldd ,x

     lds <$01
     lds $0101
     lds #$0101
     lds ,x

     ldu <$01
     ldu $0101
     ldu #$0101
     ldu ,x

     ldx <$01
     ldx $0101
     ldx #$0101
     ldx ,x

     ldy <$01
     ldy $0101
     ldy #$0101
     ldy ,x

     leas ,x
     leau ,x
     leax ,x
     leay ,x

     lsla

     lslb

     lsl <$01
     lsl $0101
     lsl ,x

     lsra

     lsrb

     lsr <$01
     lsr $0101
     lsr ,x

     mul

     nega

     negb

     neg <$01
     neg $0101
     neg ,x

     nop

     ora <$01
     ora $0101
     ora #$01
     ora ,x

     orb <$01
     orb $0101
     orb #$01
     orb ,x

     orcc #$01

     pshs a
     pshs b
     pshs cc
     pshs dp
     pshs d    ;    a,b
     pshs u
     pshs x
     pshs y
     pshs pc
     pshs a,b,cc,dp,u,x,y,pc

     pshu a
     pshu b
     pshu cc
     pshu dp
     pshu d    ;    a,b
     pshu s
     pshu x
     pshu y
     pshu pc
     pshu a,b,cc,dp,s,x,y,pc

     puls a
     puls b
     puls cc
     puls dp
     puls d    ;    a,b
     puls u
     puls x
     puls y
     puls pc
     puls a,b,cc,dp,u,x,y,pc

     pulu a
     pulu b
     pulu cc
     pulu dp
     pulu d    ;    a,b
     pulu s
     pulu x
     pulu y
     pulu pc
     pulu a,b,cc,dp,s,x,y,pc

     rola

     rolb

     rol <$01
     rol $0101
     rol ,x

     rora

     rorb

     ror <$01
     ror $0101
     ror ,x

     rti

     rts

     sbca <$01
     sbca $0101
     sbca #$01
     sbca ,x

     sbcb <$01
     sbcb $0101
     sbcb #$01
     sbcb ,x

     sex

     sta <$01
     sta $0101
     sta ,x

     stb <$01
     stb $0101
     stb ,x

     std <$01
     std $0101
     std ,x

     sts <$01
     sts $0101
     sts ,x

     stu <$01
     stu $0101
     stu ,x

     stx <$01
     stx $0101
     stx ,x

     sty <$01
     sty $0101
     sty ,x

     suba <$01
     suba $0101
     suba #$01
     suba ,x

     subb <$01
     subb $0101
     subb #$01
     subb ,x

     subd <$01
     subd $0101
     subd #$0101
     subd ,x

     swi

     swi2

     swi3

     sync

     tfr a,a
     tfr a,b
     tfr a,cc
     tfr a,dp

     tfr b,a
     tfr b,b
     tfr b,cc
     tfr b,dp

     tfr cc,a
     tfr cc,b
     tfr cc,cc
     tfr cc,dp

     tfr dp,a
     tfr dp,b
     tfr dp,cc
     tfr dp,dp

     tfr d,d
     tfr d,s
     tfr d,u
     tfr d,x
     tfr d,y
     tfr d,pc

     tfr s,d
     tfr s,s
     tfr s,u
     tfr s,x
     tfr s,y
     tfr s,pc

     tfr u,d
     tfr u,s
     tfr u,u
     tfr u,x
     tfr u,y
     tfr u,pc

     tfr x,d
     tfr x,s
     tfr x,u
     tfr x,x
     tfr x,y
     tfr x,pc

     tfr y,d
     tfr y,s
     tfr y,u
     tfr y,x
     tfr y,y
     tfr y,pc

     tfr pc,d
     tfr pc,s
     tfr pc,u
     tfr pc,x
     tfr pc,y
     tfr pc,pc

     tsta

     tstb

     tst <$01
     tst $0101
lloop
     tst ,x

; index test

     leax 0,x
     leax 1,x
     leax 2,x
     leax 3,x
     leax 4,x
     leax 5,x
     leax 6,x
     leax 7,x
     leax 8,x
     leax 9,x
     leax 10,x
     leax 11,x
     leax 12,x
     leax 13,x
     leax 14,x
     leax 15,x
     leax -16,x
     leax -15,x
     leax -14,x
     leax -13,x
     leax -12,x
     leax -11,x
     leax -10,x
     leax -9,x
     leax -8,x
     leax -7,x
     leax -6,x
     leax -5,x
     leax -4,x
     leax -3,x
     leax -2,x
     leax -1,x

     leax 0,y
     leax 1,y
     leax 2,y
     leax 3,y
     leax 4,y
     leax 5,y
     leax 6,y
     leax 7,y
     leax 8,y
     leax 9,y
     leax 10,y
     leax 11,y
     leax 12,y
     leax 13,y
     leax 14,y
     leax 15,y
     leax -16,y
     leax -15,y
     leax -14,y
     leax -13,y
     leax -12,y
     leax -11,y
     leax -10,y
     leax -9,y
     leax -8,y
     leax -7,y
     leax -6,y
     leax -5,y
     leax -4,y
     leax -3,y
     leax -2,y
     leax -1,y

     leax 0,u
     leax 1,u
     leax 2,u
     leax 3,u
     leax 4,u
     leax 5,u
     leax 6,u
     leax 7,u
     leax 8,u
     leax 9,u
     leax 10,u
     leax 11,u
     leax 12,u
     leax 13,u
     leax 14,u
     leax 15,u
     leax -16,u
     leax -15,u
     leax -14,u
     leax -13,u
     leax -12,u
     leax -11,u
     leax -10,u
     leax -9,u
     leax -8,u
     leax -7,u
     leax -6,u
     leax -5,u
     leax -4,u
     leax -3,u
     leax -2,u
     leax -1,u

     leax 0,s
     leax 1,s
     leax 2,s
     leax 3,s
     leax 4,s
     leax 5,s
     leax 6,s
     leax 7,s
     leax 8,s
     leax 9,s
     leax 10,s
     leax 11,s
     leax 12,s
     leax 13,s
     leax 14,s
     leax 15,s
     leax -16,s
     leax -15,s
     leax -14,s
     leax -13,s
     leax -12,s
     leax -11,s
     leax -10,s
     leax -9,s
     leax -8,s
     leax -7,s
     leax -6,s
     leax -5,s
     leax -4,s
     leax -3,s
     leax -2,s
     leax -1,s

     leax ,x+
     leax ,x++
     leax ,-x
     leax ,--x
     leax b,x
     leax a,x
     leax $33,x
     leax $8000,x
     leax d,x
     leax [,x++]
     leax [,--x]
     leax [,x]
     leax [b,x]
     leax [a,x]
     leax [$80,x]
     leax [$8000,x]
     leax [d,x]

     leax [$8000]

     leax ,y+
     leax ,y++
     leax ,-y
     leax ,--y
     leax b,y
     leax a,y
     leax $80,y
     leax $8000,y
     leax d,y
     leax [,y++]
     leax [,--x]
     leax [,y]
     leax [b,y]
     leax [a,y]
     leax [$80,y]
     leax [$8000,y]
     leax [d,y]

     leax ,u+
     leax ,u++
     leax ,-u
     leax ,--u
     leax b,u
     leax a,u
     leax $80,u
     leax $8000,u
     leax d,u
     leax [,u++]
     leax [,--x]
     leax [,u]
     leax [b,u]
     leax [a,u]
     leax [$33,u]
     leax [$8000,u]
     leax [d,u]

     leax ,s+
     leax ,s++
     leax ,-s
     leax ,--s
     leax b,s
     leax a,s
     leax $33,s
     leax $8000,s
     leax d,s
     leax [,s++]
     leax [,--x]
     leax [,s]
     leax [b,s]
     leax [a,s]
     leax [$33,s]
     leax [$8000,s]
     leax [d,s]

     leax $33,pc
     leax $8000,pc
