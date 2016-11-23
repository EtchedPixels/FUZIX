// $Id: syn.bcpl,v 1.5 2004/12/21 10:32:12 rn Exp $

//    MASTER

GET "LIBHDR"

GLOBAL $(
CHBUF:100; PRSOURCE:110
PPTRACE:127; OPTION:128
FORMTREE:150; PLIST:152
TREEP:167; TREEVEC:168
CHARCODE:190; REPORTCOUNT:191; REPORTMAX:192
SOURCESTREAM:193; SYSPRINT:194; OCODE:195; SYSIN:196
COMPILEAE:245
SAVESPACESIZE:282
$)

LET CHARCODE(CH) = CH  // THE DEFAULT SETTING OF CHARCODE

AND EBCDICTOASCII(CH) = CH!TABLE
      0,   0,   0,   0,   0, #11,   0,   0,
      0,   0,   0, #13, #14, #15,   0,   0,
      0,   0,   0,   0,   0, #12,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0, #12,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
    #40,   0,#133,#135,   0,   0,   0,   0,
      0,   0,   0, #56, #74, #50, #53,#174,
    #46,   0,   0,   0,   0,   0,   0,   0,
      0,   0, #41, #44, #52, #51, #73,#176,
    #55, #57,#134,   0,   0,#136,#137,   0,
      0,   0,   0, #54, #45,#140, #76, #77,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0, #72, #43,#100, #47, #75, #42,
      0,#141,#142,#143,#144,#145,#146,#147,
   #150,#151,   0,   0,   0,   0,   0,   0,
      0,#152,#153,#154,#155,#156,#157,#160,
   #161,#162,   0,   0,   0,   0,   0,   0,
      0,   0,#163,#164,#165,#166,#167,#170,
   #171,#172,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,#101,#102,#103,#104,#105,#106,#107,
   #110,#111,   0,   0,   0,   0,   0,   0,
      0,#112,#113,#114,#115,#116,#117,#120,
   #121,#122,   0,   0,   0,   0,   0,   0,
      0,   0,#123,#124,#125,#126,#127,#130,
   #131,#132,   0,   0,   0,   0,   0,   0,
    #60, #61, #62, #63, #64, #65, #66, #67,
    #70, #71,   0,   0,   0,   0,   0,   0
LET START(PARM) BE
$(1
SYSIN := INPUT()
SYSPRINT := OUTPUT()
SELECTOUTPUT(SYSPRINT)

WRITEF("*NBCPL %N*N", @START)

$( LET OPT = VEC 20
   AND TREESIZE = 5500
   OPTION := OPT
   SAVESPACESIZE := 2
   PPTRACE := FALSE
   PRSOURCE := FALSE
   FOR I = 0 TO 20 DO OPT!I := FALSE

SOURCESTREAM := FINDINPUT("OPTIONS")

UNLESS SOURCESTREAM=0 DO
$(P LET CH = 0
    AND N = 0
    SELECTINPUT(SOURCESTREAM)
    WRITES("OPTIONS  ")

    $( CH := RDCH()
    L: IF CH='*N' \/ CH=ENDSTREAMCH BREAK
       WRCH(CH)
       IF CH='P' DO N := 1
       IF CH='T' DO N := 2
       IF CH='C' DO N := 3
       IF CH='M' DO N := 4
       IF CH='N' DO N := 5
       IF CH='A' DO CHARCODE := EBCDICTOASCII
       IF CH='S' DO PRSOURCE := TRUE
       IF CH='E' DO PPTRACE := TRUE
       IF CH='L' DO  $( TREESIZE := READN()
                        WRITEN(TREESIZE)
                        CH := TERMINATOR
                        GOTO L  $)
       IF CH='3' DO SAVESPACESIZE := 3
       OPTION!N := TRUE
                 $) REPEAT

    NEWLINE()
    ENDREAD()  $)P

   REPORTMAX := 20
   REPORTCOUNT := 0



SOURCESTREAM := SYSIN
SELECTINPUT(SOURCESTREAM)

OCODE := FINDOUTPUT("OCODE")
IF OCODE=0 DO OCODE := SYSPRINT

$(2 LET COMP(V, TREEMAX) BE
    $(C LET B = VEC 63
        CHBUF := B

      $(3 TREEP, TREEVEC := V+TREEMAX, V

        $( LET A = FORMTREE()
           IF A=0 BREAK

           WRITEF("*NTREE SIZE %N*N", TREEMAX+TREEVEC-TREEP)

           IF OPTION!2 DO $( WRITES('AE TREE*N')
                             PLIST(A, 0, 20)
                             NEWLINE()  $)


           UNLESS REPORTCOUNT=0 DO STOP(8)

           UNLESS OPTION!3 DO
                  $( SELECTOUTPUT(OCODE)
                     COMPILEAE(A)
                     SELECTOUTPUT(SYSPRINT)  $)

      $)3 REPEAT
    $)C

   APTOVEC(COMP, TREESIZE)

   ENDREAD()
   IF OPTION!4 DO MAPSTORE()
   WRITES('*NPHASE 1 COMPLETE*N')
   UNLESS REPORTCOUNT=0 DO STOP(8)
   FINISH   $)1
.

//    LEX1


GET "SYNHDR"

LET NEXTSYMB() BE
$(1   NLPENDING := FALSE

NEXT: IF PPTRACE DO WRCH(CH)

    SWITCHON CH INTO

    $( CASE '*P':
       CASE '*N': LINECOUNT := LINECOUNT + 1
                  NLPENDING := TRUE  // IGNORABLE CHARACTERS
       CASE '*T':
       CASE '*S': RCH() REPEATWHILE CH='*S'
                  GOTO NEXT

       CASE '0':CASE '1':CASE '2':CASE '3':CASE '4':
       CASE '5':CASE '6':CASE '7':CASE '8':CASE '9':
            SYMB := S.NUMBER
            READNUMBER(10)
            RETURN

       CASE 'A':CASE 'B':CASE 'C':CASE 'D':CASE 'E':
       CASE 'F':CASE 'G':CASE 'H':CASE 'I':CASE 'J':
       CASE 'K':CASE 'L':CASE 'M':CASE 'N':CASE 'O':
       CASE 'P':CASE 'Q':CASE 'R':CASE 'S':CASE 'T':
       CASE 'U':CASE 'V':CASE 'W':CASE 'X':CASE 'Y':
       CASE 'Z':
              RDTAG(CH)
              SYMB := LOOKUPWORD()
              IF SYMB=S.GET DO $( PERFORMGET(); GOTO NEXT  $)
              RETURN

       CASE '$': RCH()
                 UNLESS CH='(' \/ CH=')' DO CAEREPORT(91)
                 SYMB := CH='(' -> S.LSECT, S.RSECT
                 RDTAG('$')
                 LOOKUPWORD()
                 RETURN

       CASE '[':
       CASE '(': SYMB := S.LPAREN; GOTO L
       CASE ']':
       CASE ')': SYMB := S.RPAREN; GOTO L

       CASE '#': SYMB := S.NUMBER
                 RCH()
                 IF '0'<=CH<='7' DO $( READNUMBER(8); RETURN  $)
                 IF CH='B' DO $( RCH(); READNUMBER(2); RETURN  $)
                 IF CH='O' DO $( RCH(); READNUMBER(8); RETURN  $)
                 IF CH='X' DO $( RCH(); READNUMBER(16); RETURN  $)
                 CAEREPORT(33)

       CASE '?': SYMB := S.QUERY; GOTO L
       CASE '+': SYMB := S.PLUS; GOTO L
       CASE ',': SYMB := S.COMMA; GOTO L
       CASE ';': SYMB := S.SEMICOLON; GOTO L
       CASE '@': SYMB := S.LV; GOTO L
       CASE '&': SYMB := S.LOGAND; GOTO L
       CASE '=': SYMB := S.EQ; GOTO L
       CASE '!': SYMB := S.VECAP; GOTO L
       CASE '_': SYMB := S.ASS; GOTO L
       CASE '**': SYMB := S.MULT; GOTO L

       CASE '/': RCH()
                 IF CH='\' DO $( SYMB := S.LOGAND; GOTO L $)
                 IF CH='/' GOTO COMMENT
                 UNLESS CH='**' DO $( SYMB := S.DIV; RETURN  $)

                 RCH()

                 UNTIL CH=ENDSTREAMCH DO TEST CH='**'

                       THEN $( RCH()
                               UNLESS CH='/' LOOP
                               RCH()
                               GOTO NEXT  $)

                       OR $( IF CH='*N' DO LINECOUNT := LINECOUNT+1
                             RCH()  $)

                 CAEREPORT(63)


       COMMENT: RCH() REPEATUNTIL CH='*N' \/ CH=ENDSTREAMCH
                GOTO NEXT

       CASE '|': RCH()
                 IF CH='|' GOTO COMMENT
                 SYMB := S.LOGOR
                 RETURN

       CASE '\': RCH()
                 IF CH='/' DO $( SYMB := S.LOGOR; GOTO L  $)
                 IF CH='=' DO $( SYMB := S.NE; GOTO L  $)
                 SYMB := S.NOT
                 RETURN

       CASE '<': RCH()
                 IF CH='=' DO $( SYMB := S.LE; GOTO L  $)
                 IF CH='<' DO $( SYMB := S.LSHIFT; GOTO L $)
                 SYMB := S.LS
                 RETURN

       CASE '>': RCH()
                 IF CH='=' DO $( SYMB := S.GE; GOTO L  $)
                 IF CH='>' DO $( SYMB := S.RSHIFT; GOTO L  $)
                 SYMB := S.GR
                 RETURN

       CASE '-': RCH()
                 IF CH='>' DO $( SYMB := S.COND; GOTO L  $)
                 SYMB := S.MINUS
                 RETURN

       CASE ':': RCH()
                 IF CH='=' DO $( SYMB := S.ASS; GOTO L  $)
                 SYMB := S.COLON
                 RETURN

        CASE '*'':CASE '*"':
             $(1 LET QUOTE = CH
                 CHARP := 0

              $( RCH()
                 IF CH=QUOTE \/ CHARP=255 DO
                        $( UNLESS CH=QUOTE DO CAEREPORT(95)
                           IF CHARP=1 & CH='*'' DO
                                   $( SYMB := S.NUMBER
                                      DECVAL := CHARCODE(DECVAL)
                                      GOTO L  $)
                           CHARV!0 := CHARP
                           WORDSIZE := PACKSTRING(CHARV, WORDV)
                           SYMB := S.STRING
                           GOTO L   $)


                 IF CH='*N' DO LINECOUNT := LINECOUNT + 1

                 IF CH='**' DO
                        $( RCH()
                           IF CH='*N' DO
                               $( LINECOUNT := LINECOUNT+1
                                  RCH() REPEATWHILE CH='*S' \/ CH='*T'
                                  UNLESS CH='**' DO CAEREPORT(34)
                                  LOOP  $)
                           IF CH='T' DO CH := '*T'
                           IF CH='S' DO CH := '*S'
                           IF CH='N' DO CH := '*N'
                           IF CH='B' DO CH := '*B'
                           IF CH='P' DO CH := '*P'  $)

                 DECVAL, CHARP := CH, CHARP+1
                 CHARV!CHARP := CH  $) REPEAT  $)1



       DEFAULT: IF CH=ENDSTREAMCH DO
       CASE '.':    $( IF GETP=0 DO
                             $( SYMB := S.END
                                RETURN   $)

                       ENDREAD()
                       GETP := GETP - 3
                       SOURCESTREAM := GETV!GETP
                       SELECTINPUT(SOURCESTREAM)
                       LINECOUNT := GETV!(GETP+1)
                       CH := GETV!(GETP+2)
                       GOTO NEXT  $)

                   CH := '*S'
                   CAEREPORT(94)
                   RCH()
                   GOTO NEXT

       L: RCH()   $)1

AND READNUMBER(RADIX) BE
    $( LET D = VALUE(CH)
       DECVAL := D
       IF D>=RADIX DO CAEREPORT(33)

       $( RCH()
          D := VALUE(CH)
          IF D>=RADIX RETURN
          DECVAL := RADIX*DECVAL + D  $) REPEAT
    $)


AND VALUE(CH) = '0'<=CH<='9' -> CH-'0',
                'A'<=CH<='F' -> CH-'A'+10,
                100

.

//    LEX2


GET "SYNHDR"

LET D(S, ITEM) BE $( UNPACKSTRING(S, CHARV)
                     WORDSIZE := PACKSTRING(CHARV, WORDV)
                     LOOKUPWORD()
                     WORDNODE!0 := ITEM  $)

AND DECLSYSWORDS() BE
     $( D("AND", S.AND)

        D("BE", S.BE)
        D("BREAK", S.BREAK)
        D("BY", S.BY)

        D("CASE", S.CASE)

        D("DO", S.DO)
        D("DEFAULT", S.DEFAULT)

        D("EQ", S.EQ)
        D("EQV", S.EQV)
        D("ELSE", S.OR)
        D("ENDCASE", S.ENDCASE)

        D("FALSE", S.FALSE)
        D("FOR", S.FOR)
        D("FINISH", S.FINISH)

        D("GOTO", S.GOTO)
        D("GE", S.GE)
        D("GR", S.GR)
        D("GLOBAL", S.GLOBAL)
        D("GET", S.GET)

        D("IF", S.IF)
        D("INTO", S.INTO)

        D("LET", S.LET)
        D("LV", S.LV)
        D("LE", S.LE)
        D("LS", S.LS)
        D("LOGOR", S.LOGOR)
        D("LOGAND", S.LOGAND)
        D("LOOP", S.LOOP)
        D("LSHIFT", S.LSHIFT)

        D("MANIFEST", S.MANIFEST)

        D("NE", S.NE)
        D("NOT", S.NOT)
        D("NEQV", S.NEQV)

        D("OR", S.OR)

        D("RESULTIS", S.RESULTIS)
        D("RETURN", S.RETURN)
        D("REM", S.REM)
        D("RSHIFT", S.RSHIFT)
        D("RV", S.RV)
        D("REPEAT", S.REPEAT)
        D("REPEATWHILE", S.REPEATWHILE)
        D("REPEATUNTIL", S.REPEATUNTIL)

        D("SWITCHON", S.SWITCHON)
        D("STATIC", S.STATIC)

        D("TO", S.TO)
        D("TEST", S.TEST)
        D("TRUE", S.TRUE)
        D("THEN", S.DO)
        D("TABLE", S.TABLE)

        D("UNTIL", S.UNTIL)
        D("UNLESS", S.UNLESS)

        D("VEC", S.VEC)
        D("VALOF", S.VALOF)

        D("WHILE", S.WHILE)

        D("$", 0); NULLTAG := WORDNODE  $)

AND LOOKUPWORD() = VALOF

$(1     LET HASHVAL = (WORDV!0+WORDV!WORDSIZE >> 1) REM NAMETABLESIZE
        LET M = @NAMETABLE!HASHVAL

  NEXT: WORDNODE := !M
        UNLESS WORDNODE=0 DO
             $(2 FOR I = 0 TO WORDSIZE DO
                   IF WORDNODE!(I+2) NE WORDV!I DO
                   $( M := WORDNODE+1
                      GOTO NEXT  $)
                 RESULTIS WORDNODE!0  $)2

        WORDNODE := NEWVEC(WORDSIZE+2)
        WORDNODE!0, WORDNODE!1 := S.NAME, NAMETABLE!HASHVAL
        FOR I = 0 TO WORDSIZE DO WORDNODE!(I+2) := WORDV!I
        NAMETABLE!HASHVAL := WORDNODE
        RESULTIS S.NAME
$)1

.

//    LEX3


GET "SYNHDR"

LET RCH() BE
    $( CH := RDCH()

       IF PRSOURCE DO IF GETP=0 /\ CH NE ENDSTREAMCH DO
          $( UNLESS LINECOUNT=PRLINE DO $( WRITEF("%I4  ", LINECOUNT)
                                           PRLINE := LINECOUNT  $)
             WRCH(CH)  $)

       CHCOUNT := CHCOUNT + 1
       CHBUF!(CHCOUNT&63) := CH  $)

AND WRCHBUF() BE
    $( WRITES("*N...")
       FOR P = CHCOUNT-63 TO CHCOUNT DO
                $( LET K = CHBUF!(P&63)
                   UNLESS K=0 DO WRCH(K)  $)
       NEWLINE()  $)


AND RDTAG(X) BE
    $( CHARP, CHARV!1 := 1, X

        $(  RCH()
            UNLESS 'A'<=CH<='Z' \/
                   '0'<=CH<='9' \/
                    CH='.' BREAK
            CHARP := CHARP+1
            CHARV!CHARP := CH  $) REPEAT

       CHARV!0 := CHARP
       WORDSIZE := PACKSTRING(CHARV, WORDV)  $)


AND PERFORMGET() BE
    $( NEXTSYMB()
       UNLESS SYMB=S.STRING THEN CAEREPORT(97)

       IF OPTION!5 RETURN

       GETV!GETP := SOURCESTREAM
       GETV!(GETP+1) := LINECOUNT
       GETV!(GETP+2) := CH
       GETP := GETP + 3
       LINECOUNT := 1
       SOURCESTREAM := FINDINPUT(WORDV)
       IF SOURCESTREAM=0 THEN
           SOURCESTREAM := FINDLIBINPUT(WORDV)
       IF SOURCESTREAM=0 THEN CAEREPORT(96,WORDV)
       SELECTINPUT(SOURCESTREAM)
       RCH()   $)

AND APPEND(D, S) BE
    $( LET ND = GETBYTE(D, 0)
       AND NS = GETBYTE(S, 0)
       FOR I = 1 TO NS DO $(
           ND := ND + 1
           PUTBYTE(D, ND, GETBYTE(S, I)) $)
       PUTBYTE(D, 0, ND) $)

AND FINDLIBINPUT(NAME) = VALOF
    $( LET PATH = VEC 64
       AND DIR = "/usr/local/lib/bcplkit/"
       TEST GETBYTE(DIR, 0) + GETBYTE(NAME, 0) > 255
       THEN RESULTIS 0
         OR $( PUTBYTE(PATH, 0, 0)
               APPEND(PATH, DIR)
               APPEND(PATH, NAME)
               RESULTIS FINDINPUT(PATH) $)
    $)


.

//    CAE0


GET "SYNHDR"

LET NEWVEC(N) = VALOF
    $( TREEP := TREEP - N - 1
       IF TREEP<=TREEVEC DO
                $( REPORTMAX := 0
                   CAEREPORT(98)  $)
        RESULTIS TREEP  $)

AND LIST1(X) = VALOF
    $( LET P = NEWVEC(0)
       P!0 := X
       RESULTIS P  $)

AND LIST2(X, Y) = VALOF
     $( LET P = NEWVEC(1)
        P!0, P!1 := X, Y
        RESULTIS P   $)

AND LIST3(X, Y, Z) = VALOF
     $( LET P = NEWVEC(2)
        P!0, P!1, P!2 := X, Y, Z
        RESULTIS P     $)

AND LIST4(X, Y, Z, T) = VALOF
     $( LET P = NEWVEC(3)
        P!0, P!1, P!2, P!3 := X, Y, Z, T
        RESULTIS P   $)

AND LIST5(X, Y, Z, T, U) = VALOF
     $( LET P = NEWVEC(4)
        P!0, P!1, P!2, P!3, P!4 := X, Y, Z, T, U
        RESULTIS P   $)

AND LIST6(X, Y, Z, T, U, V) = VALOF
     $( LET P = NEWVEC(5)
        P!0, P!1, P!2, P!3, P!4, P!5 := X, Y, Z, T, U, V
        RESULTIS P  $)

AND CAEREPORT(N, A) BE
     $( REPORTCOUNT := REPORTCOUNT + 1
        WRITEF("*NSYNTAX ERROR NEAR LINE %N:  ", LINECOUNT)
        CAEMESSAGE(N, A)
        WRCHBUF()
        IF REPORTCOUNT GR REPORTMAX DO
                    $( WRITES('*NCOMPILATION ABORTED*N')
                       STOP(8)   $)
        NLPENDING := FALSE

        UNTIL SYMB=S.LSECT \/ SYMB=S.RSECT \/
              SYMB=S.LET \/ SYMB=S.AND \/
              SYMB=S.END \/ NLPENDING DO NEXTSYMB()
        LONGJUMP(REC.P, REC.L)   $)

AND FORMTREE() =  VALOF
    $(1 CHCOUNT := 0
        FOR I = 0 TO 63 DO CHBUF!I := 0

     $( LET V = VEC 10   // FOR 'GET' STREAMS
        GETV, GETP, GETT := V, 0, 10

     $( LET V = VEC 100
        WORDV := V

     $( LET V = VEC 256
        CHARV, CHARP := V, 0

     $( LET V = VEC 100
        NAMETABLE, NAMETABLESIZE := V, 100
        FOR I = 0 TO 100 DO NAMETABLE!I := 0

        REC.P, REC.L := LEVEL(), L

        LINECOUNT, PRLINE := 1, 0
        RCH()

        IF CH=ENDSTREAMCH RESULTIS 0
        DECLSYSWORDS()

     L: NEXTSYMB()

        IF OPTION!1 DO   //   PP DEBUGGING OPTION
             $( WRITEF("%N %S*N", SYMB, WORDV)
                IF SYMB=S.END RESULTIS 0
                GOTO L  $)

     $( LET A = RDBLOCKBODY()
        UNLESS SYMB=S.END DO $( CAEREPORT(99); GOTO L  $)

        RESULTIS A        $)1



AND CAEMESSAGE(N, A) BE
    $( LET S = VALOF

         SWITCHON N INTO

         $( DEFAULT:  WRITEN(N); RETURN

            CASE 91: RESULTIS "'8'  '(' OR ')' EXPECTED"
            CASE 94: RESULTIS "ILLEGAL CHARACTER"
            CASE 95: RESULTIS "STRING TOO LONG"
            CASE 96: RESULTIS "NO INPUT %S"
            CASE 97: RESULTIS "STRING OR NUMBER EXPECTED"
            CASE 98: RESULTIS "PROGRAM TOO LARGE"
            CASE 99: RESULTIS "INCORRECT TERMINATION"

            CASE 8:CASE 40:CASE 43:
                     RESULTIS "NAME EXPECTED"
            CASE 6: RESULTIS "'$(' EXPECTED"
            CASE 7: RESULTIS "'$)' EXPECTED"
            CASE 9: RESULTIS "UNTAGGED '$)' MISMATCH"
            CASE 32: RESULTIS "ERROR IN EXPRESSION"
            CASE 33: RESULTIS "ERROR IN NUMBER"
            CASE 34: RESULTIS "BAD STRING"
            CASE 15:CASE 19:CASE 41: RESULTIS "')' MISSING"
            CASE 30: RESULTIS "',' MISSING"
            CASE 42: RESULTIS "'=' OR 'BE' EXPECTED"
            CASE 44: RESULTIS "'=' OR '(' EXPECTED"
            CASE 50: RESULTIS "ERROR IN LABEL"
            CASE 51: RESULTIS "ERROR IN COMMAND"
            CASE 54: RESULTIS "'OR' EXPECTED"
            CASE 57: RESULTIS "'=' EXPECTED"
            CASE 58: RESULTIS "'TO' EXPECTED"
            CASE 60: RESULTIS "'INTO' EXPECTED"
            CASE 61:CASE 62: RESULTIS "':' EXPECTED"
            CASE 63: RESULTIS "'**/' MISSING"
                       $)

         WRITEF(S, A)  $)


.

//    CAE1


GET "SYNHDR"

LET RDBLOCKBODY() = VALOF
    $(1 LET P, L = REC.P, REC.L
        LET A = 0

        REC.P, REC.L := LEVEL(), RECOVER

        IGNORE(S.SEMICOLON)

        SWITCHON SYMB INTO
     $( CASE S.MANIFEST:
        CASE S.STATIC:
        CASE S.GLOBAL:
            $(  LET OP = SYMB
                NEXTSYMB()
                A := RDSECT(RDCDEFS)
                A := LIST3(OP, A, RDBLOCKBODY())
                GOTO RET  $)


        CASE S.LET: NEXTSYMB()
                    A := RDEF()
           RECOVER: WHILE SYMB=S.AND DO
                          $( NEXTSYMB()
                             A := LIST3(S.AND, A, RDEF())  $)
                    A := LIST3(S.LET, A, RDBLOCKBODY())
                    GOTO RET

        DEFAULT: A := RDSEQ()

                 UNLESS SYMB=S.RSECT \/ SYMB=S.END DO
                          CAEREPORT(51)

        CASE S.RSECT: CASE S.END:
        RET:   REC.P, REC.L := P, L
               RESULTIS A   $)1

AND RDSEQ() = VALOF
    $( LET A = 0
       IGNORE(S.SEMICOLON)
       A := RCOM()
       IF SYMB=S.RSECT \/ SYMB=S.END RESULTIS A
       RESULTIS LIST3(S.SEQ, A, RDSEQ())   $)


AND RDCDEFS() = VALOF
    $(1 LET A, B = 0, 0
        LET PTR = @A
        LET P, L = REC.P, REC.L
        REC.P, REC.L := LEVEL(), RECOVER

        $( B := RNAME()
           TEST SYMB=S.EQ \/ SYMB=S.COLON THEN NEXTSYMB()
                                            OR CAEREPORT(45)
           !PTR := LIST4(S.CONSTDEF, 0, B, REXP(0))
           PTR := @H2!(!PTR)
  RECOVER: IGNORE(S.SEMICOLON) $) REPEATWHILE SYMB=S.NAME

        REC.P, REC.L := P, L
        RESULTIS A  $)1

AND RDSECT(R) = VALOF
    $(  LET TAG, A = WORDNODE, 0
        CHECKFOR(S.LSECT, 6)
        A := R()
        UNLESS SYMB=S.RSECT DO CAEREPORT(7)
        TEST TAG=WORDNODE
             THEN NEXTSYMB()
               OR IF WORDNODE=NULLTAG DO
                      $( SYMB := 0
                         CAEREPORT(9)  $)
        RESULTIS A   $)


AND RNAMELIST() = VALOF
    $(  LET A = RNAME()
        UNLESS SYMB=S.COMMA RESULTIS A
        NEXTSYMB()
        RESULTIS LIST3(S.COMMA, A, RNAMELIST())   $)


AND RNAME() = VALOF
    $( LET A = WORDNODE
       CHECKFOR(S.NAME, 8)
       RESULTIS A  $)

AND IGNORE(ITEM) BE IF SYMB=ITEM DO NEXTSYMB()

AND CHECKFOR(ITEM, N) BE
      $( UNLESS SYMB=ITEM DO CAEREPORT(N)
         NEXTSYMB()  $)

.

//    CAE2


GET "SYNHDR"
LET RBEXP() = VALOF
  $(1   LET A, OP = 0, SYMB

        SWITCHON SYMB INTO

    $(  DEFAULT:
            CAEREPORT(32)

        CASE S.QUERY:
            NEXTSYMB(); RESULTIS LIST1(S.QUERY)

        CASE S.TRUE:
        CASE S.FALSE:
        CASE S.NAME:
            A := WORDNODE
            NEXTSYMB()
            RESULTIS A

        CASE S.STRING:
            A := NEWVEC(WORDSIZE+1)
            A!0 := S.STRING
            FOR I = 0 TO WORDSIZE DO A!(I+1) := WORDV!I
            NEXTSYMB()
            RESULTIS A

        CASE S.NUMBER:
            A := LIST2(S.NUMBER, DECVAL)
            NEXTSYMB()
            RESULTIS A

        CASE S.LPAREN:
            NEXTSYMB()
            A := REXP(0)
            CHECKFOR(S.RPAREN, 15)
            RESULTIS A

        CASE S.VALOF:
            NEXTSYMB()
            RESULTIS LIST2(S.VALOF, RCOM())

        CASE S.VECAP: OP := S.RV
        CASE S.LV:
        CASE S.RV: NEXTSYMB(); RESULTIS LIST2(OP, REXP(35))

        CASE S.PLUS: NEXTSYMB(); RESULTIS REXP(34)

        CASE S.MINUS: NEXTSYMB()
                      A := REXP(34)
                      TEST H1!A=S.NUMBER
                          THEN H2!A := - H2!A
                            OR A := LIST2(S.NEG, A)
                      RESULTIS A

        CASE S.NOT: NEXTSYMB(); RESULTIS LIST2(S.NOT, REXP(24))

        CASE S.TABLE: NEXTSYMB()
                      RESULTIS LIST2(S.TABLE, REXPLIST())   $)1



AND REXP(N) = VALOF
    $(1 LET A = RBEXP()

        LET B, C, P, Q = 0, 0, 0, 0

  L: $( LET OP = SYMB

        IF NLPENDING RESULTIS A

        SWITCHON OP INTO
    $(B DEFAULT: RESULTIS A

        CASE S.LPAREN: NEXTSYMB()
                       B := 0
                       UNLESS SYMB=S.RPAREN DO B := REXPLIST()
                       CHECKFOR(S.RPAREN, 19)
                       A := LIST3(S.FNAP, A, B)
                       GOTO L

        CASE S.VECAP: P := 40; GOTO LASSOC

        CASE S.REM:CASE S.MULT:CASE S.DIV: P := 35; GOTO LASSOC

        CASE S.PLUS:CASE S.MINUS: P := 34; GOTO LASSOC

        CASE S.EQ:CASE S.NE:
        CASE S.LE:CASE S.GE:
        CASE S.LS:CASE S.GR:
                IF N>=30 RESULTIS A

            $(R NEXTSYMB()
                B := REXP(30)
                A := LIST3(OP, A, B)
                TEST C=0 THEN C :=  A
                           OR C := LIST3(S.LOGAND, C, A)
                A, OP := B, SYMB  $)R REPEATWHILE S.EQ<=OP<=S.GE

                A := C
                GOTO L

        CASE S.LSHIFT:CASE S.RSHIFT: P, Q := 25, 30; GOTO DIADIC

        CASE S.LOGAND: P := 23; GOTO LASSOC

        CASE S.LOGOR: P := 22; GOTO LASSOC

        CASE S.EQV:CASE S.NEQV: P := 21; GOTO LASSOC

        CASE S.COND:
                IF N>=13 RESULTIS A
                NEXTSYMB()
                B := REXP(0)
                CHECKFOR(S.COMMA, 30)
                A := LIST4(S.COND, A, B, REXP(0))
                GOTO L

        LASSOC: Q := P

        DIADIC: IF N>=P RESULTIS A
                NEXTSYMB()
                A := LIST3(OP, A, REXP(Q))
                GOTO L                     $)B     $)1

LET REXPLIST() = VALOF
    $(1 LET A = 0
        LET PTR = @A

     $( LET B = REXP(0)
        UNLESS SYMB=S.COMMA DO $( !PTR := B
                                  RESULTIS A  $)
        NEXTSYMB()
        !PTR := LIST3(S.COMMA, B, 0)
        PTR := @H3!(!PTR)  $) REPEAT
    $)1

LET RDEF() = VALOF
    $(1 LET N = RNAMELIST()

        SWITCHON SYMB INTO

     $( CASE S.LPAREN:
             $( LET A = 0
                NEXTSYMB()
                UNLESS H1!N=S.NAME DO CAEREPORT(40)
                IF SYMB=S.NAME DO A := RNAMELIST()
                CHECKFOR(S.RPAREN, 41)

                IF SYMB=S.BE DO
                     $( NEXTSYMB()
                        RESULTIS LIST5(S.RTDEF, N, A, RCOM(), 0)  $)

                IF SYMB=S.EQ DO
                     $( NEXTSYMB()
                        RESULTIS LIST5(S.FNDEF, N, A, REXP(0), 0)  $)

                CAEREPORT(42)  $)

        DEFAULT: CAEREPORT(44)

        CASE S.EQ:
                NEXTSYMB()
                IF SYMB=S.VEC DO
                     $( NEXTSYMB()
                        UNLESS H1!N=S.NAME DO CAEREPORT(43)
                        RESULTIS LIST3(S.VECDEF, N, REXP(0))  $)
                RESULTIS LIST3(S.VALDEF, N, REXPLIST())  $)1

.


//    CAE4



GET "SYNHDR"

LET RBCOM() = VALOF
   $(1 LET A, B, OP = 0, 0, SYMB

        SWITCHON SYMB INTO
     $( DEFAULT: RESULTIS 0

        CASE S.NAME:CASE S.NUMBER:CASE S.STRING:
        CASE S.TRUE:CASE S.FALSE:CASE S.LV:CASE S.RV:CASE S.VECAP:
        CASE S.LPAREN:
                A := REXPLIST()

                IF SYMB=S.ASS  THEN
                    $(  OP := SYMB
                        NEXTSYMB()
                        RESULTIS LIST3(OP, A, REXPLIST())  $)

                IF SYMB=S.COLON DO
                     $( UNLESS H1!A=S.NAME DO CAEREPORT(50)
                        NEXTSYMB()
                        RESULTIS LIST4(S.COLON, A, RBCOM(), 0)  $)

                IF H1!A=S.FNAP DO
                     $( H1!A := S.RTAP
                        RESULTIS A  $)

                CAEREPORT(51)
                RESULTIS A

        CASE S.GOTO:CASE S.RESULTIS:
                NEXTSYMB()
                RESULTIS LIST2(OP, REXP(0))

        CASE S.IF:CASE S.UNLESS:
        CASE S.WHILE:CASE S.UNTIL:
                NEXTSYMB()
                A := REXP(0)
                IGNORE(S.DO)
                RESULTIS LIST3(OP, A, RCOM())

        CASE S.TEST:
                NEXTSYMB()
                A := REXP(0)
                IGNORE(S.DO)
                B := RCOM()
                CHECKFOR(S.OR, 54)
                RESULTIS LIST4(S.TEST, A, B, RCOM())

        CASE S.FOR:
            $(  LET I, J, K = 0, 0, 0
                NEXTSYMB()
                A := RNAME()
                CHECKFOR(S.EQ, 57)
                I := REXP(0)
                CHECKFOR(S.TO, 58)
                J := REXP(0)
                IF SYMB=S.BY DO $( NEXTSYMB()
                                   K := REXP(0)  $)
                IGNORE(S.DO)
                RESULTIS LIST6(S.FOR, A, I, J, K, RCOM())  $)

        CASE S.LOOP:
        CASE S.BREAK:CASE S.RETURN:CASE S.FINISH:CASE S.ENDCASE:
                A := WORDNODE
                NEXTSYMB()
                RESULTIS A

        CASE S.SWITCHON:
                NEXTSYMB()
                A := REXP(0)
                CHECKFOR(S.INTO, 60)
                RESULTIS LIST3(S.SWITCHON, A, RDSECT(RDSEQ))

        CASE S.CASE:
                NEXTSYMB()
                A := REXP(0)
                CHECKFOR(S.COLON, 61)
                RESULTIS LIST3(S.CASE, A, RBCOM())

        CASE S.DEFAULT:
                NEXTSYMB()
                CHECKFOR(S.COLON, 62)
                RESULTIS LIST2(S.DEFAULT, RBCOM())

        CASE S.LSECT:
                RESULTIS RDSECT(RDBLOCKBODY)   $)1


AND RCOM() = VALOF
    $(1 LET A = RBCOM()

        IF A=0 DO CAEREPORT(51)

        WHILE SYMB=S.REPEAT \/ SYMB=S.REPEATWHILE \/
                    SYMB=S.REPEATUNTIL DO
                  $( LET OP = SYMB
                     NEXTSYMB()
                     TEST OP=S.REPEAT
                         THEN A := LIST2(OP, A)
                           OR A := LIST3(OP, A, REXP(0))   $)

        RESULTIS A  $)1

.

//    PLIST


GET "SYNHDR"

LET PLIST(X, N, D) BE
    $(1 LET SIZE = 0
        LET V = TABLE 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

        IF X=0 DO $( WRITES("NIL"); RETURN  $)

        SWITCHON H1!X INTO
    $(  CASE S.NUMBER: WRITEN(H2!X); RETURN

        CASE S.NAME: WRITES(X+2); RETURN

        CASE S.STRING: WRITEF("*"%S*"", X+1); RETURN

        CASE S.FOR:
                SIZE := SIZE + 2

        CASE S.COND:CASE S.FNDEF:CASE S.RTDEF:
        CASE S.TEST:CASE S.CONSTDEF:
                SIZE := SIZE + 1

        CASE S.VECAP:CASE S.FNAP:
        CASE S.MULT:CASE S.DIV:CASE S.REM:CASE S.PLUS:CASE S.MINUS:
        CASE S.EQ:CASE S.NE:CASE S.LS:CASE S.GR:CASE S.LE:CASE S.GE:
        CASE S.LSHIFT:CASE S.RSHIFT:CASE S.LOGAND:CASE S.LOGOR:
        CASE S.EQV:CASE S.NEQV:CASE S.COMMA:
        CASE S.AND:CASE S.VALDEF:CASE S.VECDEF:
        CASE S.ASS:CASE S.RTAP:CASE S.COLON:CASE S.IF:CASE S.UNLESS:
        CASE S.WHILE:CASE S.UNTIL:CASE S.REPEATWHILE:
        CASE S.REPEATUNTIL:
        CASE S.SWITCHON:CASE S.CASE:CASE S.SEQ:CASE S.LET:
        CASE S.MANIFEST:CASE S.STATIC:CASE S.GLOBAL:
                SIZE := SIZE + 1

        CASE S.VALOF:CASE S.LV:CASE S.RV:CASE S.NEG:CASE S.NOT:
        CASE S.TABLE:CASE S.GOTO:CASE S.RESULTIS:CASE S.REPEAT:
        CASE S.DEFAULT:
                SIZE := SIZE + 1

        CASE S.LOOP:
        CASE S.BREAK:CASE S.RETURN:CASE S.FINISH:CASE S.ENDCASE:
        CASE S.TRUE:CASE S.FALSE:CASE S.QUERY:
        DEFAULT:
                SIZE := SIZE + 1

                IF N=D DO $( WRITES("ETC")
                             RETURN  $)

                WRITES ("OP")
                WRITEN(H1!X)
                FOR I = 2 TO SIZE DO
                     $( NEWLINE()
                        FOR J=0 TO N-1 DO WRITES( V!J )
                        WRITES("**-")
                        V!N := I=SIZE->"  ","! "
                        PLIST(H1!(X+I-1), N+1, D)  $)
                RETURN  $)1
