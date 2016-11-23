// $Id: trn.bcpl,v 1.1 2004/12/09 20:29:37 rn Exp $

//    TRN0

GET "TRNHDR"

LET NEXTPARAM() = VALOF
    $( PARAMNUMBER := PARAMNUMBER + 1
       RESULTIS PARAMNUMBER  $)

AND TRANSREPORT(N, X) BE
    $( SELECTOUTPUT(SYSPRINT)
       REPORTCOUNT := REPORTCOUNT + 1
       IF REPORTCOUNT GE REPORTMAX DO
                $( WRITES("*NCOMPILATION ABORTED*N")
                   STOP(8)  $)
       WRITES("*NREPORT:   "); TRNMESSAGE(N)
       WRITEF("*NCOMMANDS COMPILED %N*N", COMCOUNT)
       PLIST(X, 0, 4); NEWLINE()
       SELECTOUTPUT(OCODE)  $)

AND TRNMESSAGE(N) BE
$( LET S = VALOF
    SWITCHON N INTO

    $( DEFAULT: WRITEF("COMPILER ERROR  %N", N); RETURN

       CASE 141: RESULTIS "TOO MANY CASES"
       CASE 104: RESULTIS "ILLEGAL USE OF BREAK, LOOP OR RESULTIS"
       CASE 101:
       CASE 105: RESULTIS "ILLEGAL USE OF CASE OR DEFAULT"
       CASE 106: RESULTIS "TWO CASES WITH SAME CONSTANT"
       CASE 144: RESULTIS "TOO MANY GLOBALS"
       CASE 142: RESULTIS "NAME DECLARED TWICE"
       CASE 143: RESULTIS "TOO MANY NAMES DECLARED"
       CASE 115: RESULTIS "NAME NOT DECLARED"
       CASE 116: RESULTIS "DYNAMIC FREE VARIABLE USED"
       CASE 117:CASE 118:CASE 119:
                 RESULTIS "ERROR IN CONSTANT EXPRESSION"
       CASE 110:CASE 112:
                 RESULTIS "LHS AND RHS DO NOT MATCH"
       CASE 109:CASE 113:
                 RESULTIS "LTYPE EXPRESSION EXPECTED"
                   $)

   WRITES(S)   $)


LET COMPILEAE(X) BE
   $(1 LET A = VEC 1200
       LET D = VEC 100
       LET K = VEC 150
       LET L = VEC 150

       DVEC, DVECS, DVECE, DVECP, DVECT := A, 3, 3, 3, 1200
       DVEC!0, DVEC!1, DVEC!2 := 0, 0, 0

       GLOBDECL, GLOBDECLS, GLOBDECLT := D, 0, 100

       CASEK, CASEL, CASEP, CASET, CASEB := K, L, 0, 150, -1
       ENDCASELABEL, DEFAULTLABEL := 0, 0

       RESULTLABEL, BREAKLABEL, LOOPLABEL := -1, -1, -1

       COMCOUNT, CURRENTBRANCH := 0, X

       OCOUNT := 0

       PARAMNUMBER := 0
       SSP := SAVESPACESIZE
       OUT2(S.STACK, SSP)
       DECLLABELS(X)
       TRANS(X)
       OUT2(S.GLOBAL, GLOBDECLS/2)

    $( LET I = 0
       UNTIL I=GLOBDECLS DO
          $( OUTN(GLOBDECL!I)
             OUTL(GLOBDECL!(I+1))
             I := I + 2  $)

       ENDOCODE()  $)1

.

//    TRN1


GET "TRNHDR"

LET TRANS(X) BE
  $(TR
NEXT:
 $( LET SW = FALSE
    IF X=0 RETURN
    CURRENTBRANCH := X

    SWITCHON H1!X INTO
$(  DEFAULT: TRANSREPORT(100, X); RETURN

    CASE S.LET:
      $( LET A, B, S, S1 = DVECE, DVECS, SSP, 0
         LET V = VECSSP
         DECLNAMES(H2!X)
         CHECKDISTINCT(B, DVECS)
         DVECE := DVECS
         VECSSP, S1 := SSP, SSP
         SSP := S
         TRANSDEF(H2!X)
         UNLESS SSP=S1 DO TRANSREPORT(110, X)
         UNLESS SSP=VECSSP DO $( SSP := VECSSP
                                 OUT2(S.STACK, SSP)  $)
         OUT1(S.STORE)
         DECLLABELS(H3!X)
         TRANS(H3!X)
         VECSSP := V
         UNLESS SSP=S DO OUT2(S.STACK, S)
         DVECE, DVECS, SSP := A, B, S
         RETURN   $)

    CASE S.STATIC:
    CASE S.GLOBAL:
    CASE S.MANIFEST:
     $(1 LET A, B, S = DVECE, DVECS, SSP
         AND OP = H1!X
         AND Y = H2!X

         IF OP=S.MANIFEST DO OP := S.NUMBER

         UNTIL Y=0 DO
           $( TEST OP=S.STATIC THEN
                $( LET M = NEXTPARAM()
                   ADDNAME(H3!Y, S.LABEL, M)
                   COMPDATALAB(M)
                   OUT2(S.ITEMN, EVALCONST(H4!Y))  $)

                OR ADDNAME(H3!Y, OP, EVALCONST(H4!Y))

              Y := H2!Y
              DVECE := DVECS  $)

         DECLLABELS(H3!X)
         TRANS(H3!X)
         DVECE, DVECS, SSP := A, B, S
         RETURN   $)1


    CASE S.ASS:
       ASSIGN(H2!X, H3!X)
       RETURN

    CASE S.RTAP:
     $( LET S = SSP
        SSP := SSP+SAVESPACESIZE
        OUT2(S.STACK, SSP)
        LOADLIST(H3!X)
        LOAD(H2!X)
        OUT2(S.RTAP, S)
        SSP := S
        RETURN  $)

    CASE S.GOTO:
        LOAD(H2!X)
        OUT1(S.GOTO)
        SSP := SSP-1
        RETURN

    CASE S.COLON:
        COMPLAB(H4!X)
        TRANS(H3!X)
        RETURN

    CASE S.UNLESS: SW := TRUE
    CASE S.IF:
     $( LET L = NEXTPARAM()
        JUMPCOND(H2!X, SW, L)
        TRANS(H3!X)
        COMPLAB(L)
        RETURN   $)

    CASE S.TEST:
     $( LET L, M = NEXTPARAM(), NEXTPARAM()
        JUMPCOND(H2!X, FALSE, L)
        TRANS(H3!X)
        COMPJUMP(M)
        COMPLAB(L)
        TRANS(H4!X)
        COMPLAB(M)
        RETURN   $)

    CASE S.LOOP:
        IF LOOPLABEL<0 DO TRANSREPORT(104, X)
        IF LOOPLABEL=0 DO LOOPLABEL := NEXTPARAM()
        COMPJUMP(LOOPLABEL)
        RETURN

    CASE S.BREAK:
        IF BREAKLABEL<0 DO TRANSREPORT(104, X)
        IF BREAKLABEL=0 DO BREAKLABEL := NEXTPARAM()
        COMPJUMP(BREAKLABEL)
        RETURN

    CASE S.RETURN: OUT1(S.RTRN)
                   RETURN

    CASE S.FINISH: OUT1(S.FINISH)
                   RETURN

    CASE S.RESULTIS:
        IF RESULTLABEL<0 DO TRANSREPORT(104, X)
        LOAD(H2!X)
        OUT2P(S.RES, RESULTLABEL)
        SSP := SSP - 1
        RETURN

    CASE S.WHILE: SW := TRUE
    CASE S.UNTIL:
     $( LET L, M = NEXTPARAM(), NEXTPARAM()
        LET BL, LL = BREAKLABEL, LOOPLABEL
        BREAKLABEL, LOOPLABEL := 0, M

        COMPJUMP(M)
        COMPLAB(L)
        TRANS(H3!X)
        COMPLAB(M)
        JUMPCOND(H2!X, SW, L)
        UNLESS BREAKLABEL=0 DO COMPLAB(BREAKLABEL)
        BREAKLABEL, LOOPLABEL := BL, LL
        RETURN   $)

    CASE S.REPEATWHILE: SW := TRUE
    CASE S.REPEATUNTIL:
    CASE S.REPEAT:
     $( LET L, BL, LL = NEXTPARAM(), BREAKLABEL, LOOPLABEL
        BREAKLABEL, LOOPLABEL := 0, 0
        COMPLAB(L)
        TEST H1!X=S.REPEAT
            THEN $( LOOPLABEL := L
                    TRANS(H2!X)
                    COMPJUMP(L)  $)
              OR $( TRANS(H2!X)
                    UNLESS LOOPLABEL=0 DO COMPLAB(LOOPLABEL)
                    JUMPCOND(H3!X, SW, L)  $)
        UNLESS BREAKLABEL=0 DO COMPLAB(BREAKLABEL)
        BREAKLABEL, LOOPLABEL := BL, LL
        RETURN   $)

    CASE S.CASE:
     $( LET L, K = NEXTPARAM(), EVALCONST(H2!X)
        IF CASEP>=CASET DO TRANSREPORT(141, X)
        IF CASEB<0 DO TRANSREPORT(105, X)
        FOR I = CASEB TO CASEP-1 DO
                    IF CASEK!I=K DO TRANSREPORT(106, X)
        CASEK!CASEP := K
        CASEL!CASEP := L
        CASEP := CASEP + 1
        COMPLAB(L)
        TRANS(H3!X)
        RETURN   $)

    CASE S.DEFAULT:
        IF CASEB<0 DO TRANSREPORT(105, X)
        UNLESS DEFAULTLABEL=0 DO TRANSREPORT(101, X)
        DEFAULTLABEL := NEXTPARAM()
        COMPLAB(DEFAULTLABEL)
        TRANS(H2!X)
        RETURN

    CASE S.ENDCASE: IF CASEB<0 DO TRANSREPORT(105, X)
                    COMPJUMP(ENDCASELABEL)
                    RETURN

    CASE S.SWITCHON:
        TRANSSWITCH(X)
        RETURN

    CASE S.FOR: TRANSFOR(X)
                RETURN

    CASE S.SEQ:
        TRANS(H2!X)
        COMCOUNT :=  COMCOUNT + 1
        X := H3!X
        GOTO NEXT        $)TR
.

//    TRN2


GET "TRNHDR"

LET DECLNAMES(X) BE UNLESS X=0 SWITCHON H1!X INTO

     $(  DEFAULT: TRANSREPORT(102, CURRENTBRANCH)
                  RETURN

         CASE S.VECDEF: CASE S.VALDEF:
               DECLDYN(H2!X)
               RETURN

         CASE S.RTDEF: CASE S.FNDEF:
               H5!X := NEXTPARAM()
               DECLSTAT(H2!X, H5!X)
               RETURN

         CASE S.AND:
               DECLNAMES(H2!X)
               DECLNAMES(H3!X)
               RETURN    $)


AND DECLDYN(X) BE UNLESS X=0 DO

    $( IF H1!X=S.NAME DO
          $( ADDNAME(X, S.LOCAL, SSP)
             SSP := SSP + 1
             RETURN   $)

       IF H1!X=S.COMMA DO
          $( ADDNAME(H2!X, S.LOCAL, SSP)
             SSP := SSP + 1
             DECLDYN(H3!X)
             RETURN  $)

       TRANSREPORT(103, X)   $)

AND DECLSTAT(X, L) BE
    $(1 LET T = CELLWITHNAME(X)

       IF DVEC!(T+1)=S.GLOBAL DO
          $( LET N = DVEC!(T+2)
             ADDNAME(X, S.GLOBAL, N)
             IF GLOBDECLS>=GLOBDECLT DO TRANSREPORT(144, X)
             GLOBDECL!GLOBDECLS := N
             GLOBDECL!(GLOBDECLS+1) := L
             GLOBDECLS := GLOBDECLS + 2
             RETURN  $)


    $( LET M = NEXTPARAM()
       ADDNAME(X, S.LABEL, M)
       COMPDATALAB(M)
       OUT2P(S.ITEML, L)    $)1


AND DECLLABELS(X) BE
    $( LET B = DVECS
       SCANLABELS(X)
       CHECKDISTINCT(B, DVECS)
       DVECE := DVECS   $)


AND CHECKDISTINCT(E, S) BE
       UNTIL E=S DO
          $( LET P = E + 3
             AND N = DVEC!E
             WHILE P<S DO
                $( IF DVEC!P=N DO TRANSREPORT(142, N)
                   P := P + 3  $)
             E := E + 3  $)


AND ADDNAME(N, P, A) BE
    $( IF DVECS>=DVECT DO TRANSREPORT(143, CURRENTBRANCH)
       DVEC!DVECS, DVEC!(DVECS+1), DVEC!(DVECS+2) := N, P, A
       DVECS := DVECS + 3  $)


AND CELLWITHNAME(N) = VALOF
    $( LET X = DVECE

       X := X - 3 REPEATUNTIL X=0 \/ DVEC!X=N

       RESULTIS X  $)


AND SCANLABELS(X) BE UNLESS X=0 SWITCHON H1!X INTO

    $( DEFAULT: RETURN

       CASE S.COLON:
            H4!X := NEXTPARAM()
            DECLSTAT(H2!X, H4!X)

       CASE S.IF: CASE S.UNLESS: CASE S.WHILE: CASE S.UNTIL:
       CASE S.SWITCHON: CASE S.CASE:
            SCANLABELS(H3!X)
            RETURN

       CASE S.SEQ:
            SCANLABELS(H3!X)

       CASE S.REPEAT:
       CASE S.REPEATWHILE: CASE S.REPEATUNTIL: CASE S.DEFAULT:
            SCANLABELS(H2!X)
            RETURN

       CASE S.TEST:
            SCANLABELS(H3!X)
            SCANLABELS(H4!X)
            RETURN    $)


AND TRANSDEF(X) BE
    $(1 TRANSDYNDEFS(X)
        IF STATDEFS(X) DO
           $( LET L, S= NEXTPARAM(), SSP
              COMPJUMP(L)
              TRANSSTATDEFS(X)
              SSP := S
              OUT2(S.STACK, SSP)
              COMPLAB(L)  $)1


AND TRANSDYNDEFS(X) BE
        SWITCHON H1!X INTO
     $( CASE S.AND:
            TRANSDYNDEFS(H2!X)
            TRANSDYNDEFS(H3!X)
            RETURN

        CASE S.VECDEF:
            OUT2(S.LLP, VECSSP)
            SSP := SSP + 1
            VECSSP := VECSSP + 1 + EVALCONST(H3!X)
            RETURN

        CASE S.VALDEF: LOADLIST(H3!X)
                       RETURN

        DEFAULT: RETURN  $)

AND TRANSSTATDEFS(X) BE
        SWITCHON H1!X INTO
     $( CASE S.AND:
             TRANSSTATDEFS(H2!X)
             TRANSSTATDEFS(H3!X)
             RETURN

        CASE S.FNDEF: CASE S.RTDEF:
         $(2 LET A, B, C = DVECE, DVECS, DVECP
             AND BL, LL = BREAKLABEL, LOOPLABEL
             AND RL, CB = RESULTLABEL, CASEB
             BREAKLABEL, LOOPLABEL := -1, -1
             RESULTLABEL, CASEB := -1, -1

             COMPENTRY(H2!X, H5!X)
             SSP := SAVESPACESIZE

             DVECP := DVECS
             DECLDYN(H3!X)
             CHECKDISTINCT(B, DVECS)
             DVECE := DVECS
             DECLLABELS(H4!X)

             OUT2(S.SAVE, SSP)

             TEST H1!X=S.FNDEF
                THEN $( LOAD(H4!X); OUT1(S.FNRN)  $)
                  OR $( TRANS(H4!X); OUT1(S.RTRN)  $)

             OUT2(S.ENDPROC, 0)

             BREAKLABEL, LOOPLABEL := BL, LL
             RESULTLABEL, CASEB := RL, CB
             DVECE, DVECS, DVECP := A, B, C   $)2

        DEFAULT: RETURN   $)

AND STATDEFS(X) = H1!X=S.FNDEF \/ H1!X=S.RTDEF -> TRUE,
                  H1!X NE S.AND -> FALSE,
                  STATDEFS(H2!X) -> TRUE,
                  STATDEFS(H3!X)


.

//    TRN3


GET "TRNHDR"

LET JUMPCOND(X, B, L) BE
$(JC LET SW = B
     SWITCHON H1!X INTO
     $( CASE S.FALSE: B := NOT B
        CASE S.TRUE: IF B DO COMPJUMP(L)
                     RETURN

        CASE S.NOT: JUMPCOND(H2!X, NOT B, L)
                    RETURN

        CASE S.LOGAND: SW := NOT SW
        CASE S.LOGOR:
         TEST SW THEN $( JUMPCOND(H2!X, B, L)
                         JUMPCOND(H3!X, B, L)  $)

                   OR $( LET M = NEXTPARAM()
                         JUMPCOND(H2!X, NOT B, M)
                         JUMPCOND(H3!X, B, L)
                         COMPLAB(M)  $)

         RETURN

        DEFAULT: LOAD(X)
                 OUT2P(B -> S.JT, S.JF, L)
                 SSP := SSP - 1
                 RETURN     $)JC

AND TRANSSWITCH(X) BE
    $(1 LET P, B, DL = CASEP, CASEB, DEFAULTLABEL
        AND ECL = ENDCASELABEL
        LET L = NEXTPARAM()
        ENDCASELABEL := NEXTPARAM()
        CASEB := CASEP

        COMPJUMP(L)
        DEFAULTLABEL := 0
        TRANS(H3!X)
        COMPJUMP(ENDCASELABEL)

        COMPLAB(L)
        LOAD(H2!X)
        IF DEFAULTLABEL=0 DO DEFAULTLABEL := ENDCASELABEL
        OUT3P(S.SWITCHON, CASEP-P, DEFAULTLABEL)

        FOR I = CASEB TO CASEP-1 DO $( OUTN(CASEK!I)
                                       OUTL(CASEL!I)  $)

        SSP := SSP - 1
        COMPLAB(ENDCASELABEL)
        ENDCASELABEL := ECL
        CASEP, CASEB, DEFAULTLABEL := P, B, DL   $)1

AND TRANSFOR(X) BE
     $( LET A, B = DVECE, DVECS
        LET L, M = NEXTPARAM(), NEXTPARAM()
        LET BL, LL = BREAKLABEL, LOOPLABEL
        LET K, N = 0, 0
        LET STEP = 1
        LET S = SSP
        BREAKLABEL, LOOPLABEL := 0, 0

        ADDNAME(H2!X, S.LOCAL, S)
        DVECE := DVECS
        LOAD(H3!X)

        TEST H1!(H4!X)=S.NUMBER
            THEN K, N := S.LN, H2!(H4!X)
              OR $( K, N := S.LP, SSP
                    LOAD(H4!X)  $)

        UNLESS H5!X=0 DO STEP := EVALCONST(H5!X)

        OUT1(S.STORE)
        COMPJUMP(L)
        DECLLABELS(H6!X)
        COMPLAB(M)
        TRANS(H6!X)
        UNLESS LOOPLABEL=0 DO COMPLAB(LOOPLABEL)
        OUT2(S.LP, S); OUT2(S.LN, STEP); OUT1(S.PLUS); OUT2(S.SP, S)
        COMPLAB(L)
        OUT2(S.LP, S); OUT2(K, N); OUT1(STEP<0 -> S.GE, S.LE)
        OUT2P(S.JT, M)

        UNLESS BREAKLABEL=0 DO COMPLAB(BREAKLABEL)
        BREAKLABEL, LOOPLABEL, SSP := BL, LL, S
        OUT2(S.STACK, SSP)
        DVECE, DVECS := A, B  $)

.

//    TRN4


GET "TRNHDR"

LET LOAD(X) BE
    $(1 IF X=0 DO $( TRANSREPORT(148, CURRENTBRANCH)
                     LOADZERO()
                     RETURN  $)

     $( LET OP = H1!X

        SWITCHON OP INTO
     $( DEFAULT: TRANSREPORT(147, CURRENTBRANCH)
                 LOADZERO()
                 RETURN

        CASE S.DIV: CASE S.REM: CASE S.MINUS:
        CASE S.LS: CASE S.GR: CASE S.LE: CASE S.GE:
        CASE S.LSHIFT: CASE S.RSHIFT:
            LOAD(H2!X)
            LOAD(H3!X)
            OUT1(OP)
            SSP := SSP - 1
            RETURN

        CASE S.VECAP: CASE S.MULT: CASE S.PLUS: CASE S.EQ: CASE S.NE:
        CASE S.LOGAND: CASE S.LOGOR: CASE S.EQV: CASE S.NEQV:
         $( LET A, B = H2!X, H3!X
            IF H1!A=S.NAME \/ H1!A=S.NUMBER DO
                               A, B := H3!X, H2!X
            LOAD(A)
            LOAD(B)
            IF OP=S.VECAP DO $( OUT1(S.PLUS); OP := S.RV  $)
            OUT1(OP)
            SSP := SSP - 1
            RETURN   $)

        CASE S.NEG: CASE S.NOT: CASE S.RV:
            LOAD(H2!X)
            OUT1(OP)
            RETURN

        CASE S.TRUE: CASE S.FALSE: CASE S.QUERY:
            OUT1(OP)
            SSP := SSP + 1
            RETURN

        CASE S.LV: LOADLV(H2!X)
                   RETURN

        CASE S.NUMBER:
            OUT2(S.LN, H2!X)
            SSP := SSP + 1
            RETURN

        CASE S.STRING:
         $( LET S = @H2!X
            OUT2(S.LSTR, GETBYTE(S, 0))
            FOR I = 1 TO GETBYTE(S, 0) DO OUTC(GETBYTE(S, I))
            WRC('*S')
            SSP := SSP + 1
            RETURN   $)

        CASE S.NAME:
             TRANSNAME(X, S.LP, S.LG, S.LL, S.LN)
             SSP := SSP + 1
             RETURN

        CASE S.VALOF:
         $( LET RL = RESULTLABEL
            LET A, B = DVECS, DVECE
            DECLLABELS(H2!X)
            RESULTLABEL := NEXTPARAM()
            TRANS(H2!X)
            COMPLAB(RESULTLABEL)
            OUT2(S.RSTACK, SSP)
            SSP := SSP + 1
            DVECS, DVECE := A, B
            RESULTLABEL := RL
            RETURN   $)


        CASE S.FNAP:
         $( LET S = SSP
            SSP := SSP + SAVESPACESIZE
            OUT2(S.STACK, SSP)
            LOADLIST(H3!X)
            LOAD(H2!X)
            OUT2(S.FNAP, S)
            SSP := S + 1
            RETURN   $)

        CASE S.COND:
         $( LET L, M = NEXTPARAM(), NEXTPARAM()
            LET S = SSP
            JUMPCOND(H2!X, FALSE, M)
            LOAD(H3!X)
            COMPJUMP(L)
            SSP := S; OUT2(S.STACK, SSP)
            COMPLAB(M)
            LOAD(H4!X)
            COMPLAB(L)
            RETURN   $)

        CASE S.TABLE:
         $( LET M = NEXTPARAM()
            COMPDATALAB(M)
            X := H2!X
            WHILE H1!X=S.COMMA DO
                  $( OUT2(S.ITEMN, EVALCONST(H2!X))
                     X := H3!X   $)
            OUT2(S.ITEMN, EVALCONST(X))
            OUT2P(S.LLL, M)
            SSP := SSP + 1
            RETURN  $)                         $)1


AND LOADLV(X) BE
    $(1 IF X=0 GOTO ERR

        SWITCHON H1!X INTO
     $( DEFAULT:
        ERR:     TRANSREPORT(113, CURRENTBRANCH)
                 LOADZERO()
                 RETURN

        CASE S.NAME:
              TRANSNAME(X, S.LLP, S.LLG, S.LLL, 0)
              SSP := SSP + 1
              RETURN

        CASE S.RV:
            LOAD(H2!X)
            RETURN

        CASE S.VECAP:
         $( LET A, B = H2!X, H3!X
            IF H1!A=S.NAME DO A, B := H3!X, H2!X
            LOAD(A)
            LOAD(B)
            OUT1(S.PLUS)
            SSP := SSP - 1
            RETURN   $)  $)1

AND LOADZERO() BE $( OUT2(S.LN, 0)
                     SSP := SSP + 1  $)

AND LOADLIST(X) BE UNLESS X=0 DO
    $( UNLESS H1!X=S.COMMA DO $( LOAD(X); RETURN  $)

       LOADLIST(H2!X)
       LOADLIST(H3!X)  $)
.

//    TRN5


GET "TRNHDR"

LET EVALCONST(X) = VALOF
    $(1 IF X=0 DO $( TRANSREPORT(117, CURRENTBRANCH)
                     RESULTIS 0  $)

        SWITCHON H1!X INTO
     $( DEFAULT: TRANSREPORT(118, X)
                 RESULTIS 0

        CASE S.NAME:
         $( LET T = CELLWITHNAME(X)
            IF DVEC!(T+1)=S.NUMBER RESULTIS DVEC!(T+2)
            TRANSREPORT(119, X)
            RESULTIS 0  $)

        CASE S.NUMBER: RESULTIS H2!X
        CASE S.TRUE: RESULTIS TRUE
        CASE S.FALSE: RESULTIS FALSE

        CASE S.NEG: RESULTIS - EVALCONST(H2!X)

        CASE S.MULT: RESULTIS EVALCONST(H2!X) * EVALCONST(H3!X)
        CASE S.DIV:  RESULTIS EVALCONST(H2!X) / EVALCONST(H3!X)
        CASE S.PLUS: RESULTIS EVALCONST(H2!X) + EVALCONST(H3!X)
        CASE S.MINUS:RESULTIS EVALCONST(H2!X) - EVALCONST(H3!X)
                    $)1


AND ASSIGN(X, Y) BE
    $(1 IF X=0 \/ Y=0 DO
            $( TRANSREPORT(110, CURRENTBRANCH)
               RETURN  $)

        SWITCHON H1!X INTO
     $( CASE S.COMMA:
            UNLESS H1!Y=S.COMMA DO
                       $( TRANSREPORT(112, CURRENTBRANCH)
                          RETURN   $)
            ASSIGN(H2!X, H2!Y)
            ASSIGN(H3!X, H3!Y)
            RETURN

        CASE S.NAME:
            LOAD(Y)
            TRANSNAME(X, S.SP, S.SG, S.SL, 0)
            SSP := SSP - 1
            RETURN

        CASE S.RV: CASE S.VECAP: CASE S.COND:
            LOAD(Y)
            LOADLV(X)
            OUT1(S.STIND)
            SSP := SSP - 2
            RETURN

        DEFAULT: TRANSREPORT(109, CURRENTBRANCH)   $)1


AND TRANSNAME(X, P, G, L, N) BE
    $(1 LET T = CELLWITHNAME(X)
        LET K, A = DVEC!(T+1), DVEC!(T+2)

        IF T=0 DO $( TRANSREPORT(115, X)
                     OUT2(G, 2)
                     RETURN  $)

        SWITCHON K INTO
        $( CASE S.LOCAL: IF T<DVECP DO TRANSREPORT(116, X)
                         OUT2(P, A); RETURN

           CASE S.GLOBAL: OUT2(G, A); RETURN

           CASE S.LABEL: OUT2P(L, A); RETURN

           CASE S.NUMBER: IF N=0 DO $( TRANSREPORT(113, X)
                                       N := P  $)
                          OUT2(N, A)  $)1

.

//    TRN6


GET "TRNHDR"

LET COMPLAB(L) BE OUT2P(S.LAB, L)

AND COMPENTRY(N, L) BE
    $(  LET S = @N!2
        OUT3P(S.ENTRY, GETBYTE(S, 0), L)
        FOR I = 1 TO GETBYTE(S, 0) DO OUTC(GETBYTE(S, I))
        WRC('*S')  $)

AND COMPDATALAB(L) BE OUT2P(S.DATALAB, L)

AND COMPJUMP(L) BE OUT2P(S.JUMP, L)

AND OUT1(X) BE
    $( WRITEOP(X); WRC('*S')  $)

AND OUT2(X, Y) BE
    $( WRITEOP(X); WRC('*S')
       WRN(Y); WRC('*S')   $)

AND OUT2P(X, Y) BE
    $( WRITEOP(X); WRC('*S'); WRC('L')
       WRN(Y); WRC('*S')   $)

AND OUT3P(X, Y, Z) BE
    $( WRITEOP(X); WRC('*S')
       WRN(Y); WRC('*S'); WRC('L')
       WRN(Z); WRC('*S')   $)


AND OUTN(N) BE WRN(N)

AND OUTL(X) BE
    $( WRC('*S'); WRC('L'); WRN(X); WRC('*S')  $)

AND OUTC(X) BE
    $( WRN(CHARCODE(X)); WRC('*S')   $)

AND WRITEOP(X) BE
    $(1 LET S = VALOF SWITCHON X INTO
        $( DEFAULT: TRANSREPORT(199, CURRENTBRANCH)
                    RESULTIS 'ERROR'

           CASE S.MULT:    RESULTIS "MULT"
           CASE S.DIV:     RESULTIS "DIV"
           CASE S.REM:     RESULTIS "REM"
           CASE S.PLUS:    RESULTIS "PLUS"
           CASE S.MINUS:   RESULTIS "MINUS"
           CASE S.EQ:      RESULTIS "EQ"
           CASE S.NE:      RESULTIS "NE"
           CASE S.LS:      RESULTIS "LS"
           CASE S.GR:      RESULTIS "GR"
           CASE S.LE:      RESULTIS "LE"
           CASE S.GE:      RESULTIS "GE"
           CASE S.LSHIFT:  RESULTIS "LSHIFT"
           CASE S.RSHIFT:  RESULTIS "RSHIFT"
           CASE S.LOGAND:  RESULTIS "LOGAND"
           CASE S.LOGOR:   RESULTIS "LOGOR"
           CASE S.EQV:     RESULTIS "EQV"
           CASE S.NEQV:    RESULTIS "NEQV"

           CASE S.NEG:     RESULTIS "NEG"
           CASE S.NOT:     RESULTIS "NOT"
           CASE S.RV:      RESULTIS "RV"

           CASE S.TRUE:    RESULTIS "TRUE"
           CASE S.FALSE:   RESULTIS "FALSE"
           CASE S.QUERY:   RESULTIS "QUERY"

           CASE S.LP:      RESULTIS "LP"
           CASE S.LG:      RESULTIS "LG"
           CASE S.LN:      RESULTIS "LN"
           CASE S.LSTR:    RESULTIS "LSTR"
           CASE S.LL:      RESULTIS "LL"

           CASE S.LLP:     RESULTIS "LLP"
           CASE S.LLG:     RESULTIS "LLG"
           CASE S.LLL:     RESULTIS "LLL"

           CASE S.SP:      RESULTIS "SP"
           CASE S.SG:      RESULTIS "SG"
           CASE S.SL:      RESULTIS "SL"
           CASE S.STIND:   RESULTIS "STIND"

           CASE S.JUMP:    RESULTIS "JUMP"
           CASE S.JT:      RESULTIS "JT"
           CASE S.JF:      RESULTIS "JF"
           CASE S.GOTO:    RESULTIS "GOTO"
           CASE S.LAB:     RESULTIS "LAB"
           CASE S.STACK:   RESULTIS "STACK"
           CASE S.STORE:   RESULTIS "STORE"

           CASE S.ENTRY:   RESULTIS "ENTRY"
           CASE S.SAVE:    RESULTIS "SAVE"
           CASE S.FNAP:    RESULTIS "FNAP"
           CASE S.FNRN:    RESULTIS "FNRN"
           CASE S.RTAP:    RESULTIS "RTAP"
           CASE S.RTRN:    RESULTIS "RTRN"
           CASE S.ENDPROC: RESULTIS "ENDPROC"
           CASE S.RES:     RESULTIS "RES"
           CASE S.RSTACK:  RESULTIS "RSTACK"
           CASE S.FINISH:  RESULTIS "FINISH"

           CASE S.SWITCHON:RESULTIS "SWITCHON"
           CASE S.GLOBAL:  RESULTIS "GLOBAL"
           CASE S.DATALAB: RESULTIS "DATALAB"
           CASE S.ITEML:   RESULTIS "ITEML"
           CASE S.ITEMN:   RESULTIS "ITEMN"   $)

        FOR I = 1 TO GETBYTE(S, 0) DO WRC(GETBYTE(S, I))   $)1


AND WRN(N) BE $( TEST N<0 DO WRC('-') OR N := - N
                 WRNN(N)  $)

AND WRNN(N) BE $( IF N<-9 DO WRNN(N/10)
                  WRC('0' - N REM 10)  $)

AND ENDOCODE() BE $( WRCH('*N'); OCOUNT := 0  $)


AND WRC(CH) BE $( OCOUNT := OCOUNT + 1
                  IF OCOUNT>62 /\ CH='*S' DO
                            $( WRCH('*N'); OCOUNT := 0; RETURN  $)
                  WRCH(CH)  $)
