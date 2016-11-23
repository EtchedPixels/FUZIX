// $Id: cg.bcpl,v 1.4 2004/12/21 09:37:53 rn Exp $

//    CG1

GET "CGHDR"

STATIC $( WP=0; STRSIZE=0  $)

LET T(S) = VALOF
      $( FOR I = 0 TO STRSIZE DO UNLESS S!I=WORDV!I RESULTIS FALSE
         RESULTIS TRUE  $)

LET READOP() = VALOF
    $(1 LET S = VEC 20

        CH := RDCH() REPEATWHILE CH='*N' \/ CH='*S'
        WP := 0

        WHILE 'A'<=CH<='Z' DO
           $( WP := WP + 1
              S!WP := CH
              CH := RDCH()  $)

        S!0 := WP
        STRSIZE := PACKSTRING(S, WORDV)

        SWITCHON S!1 INTO
     $( DEFAULT: IF CH=ENDSTREAMCH RESULTIS S.END
                 RESULTIS ERROR

        CASE 'D':
        RESULTIS T("DATALAB") -> S.DATALAB,
                 T("DIV") -> S.DIV,
                 T("DEBUG") -> S.DEBUG, ERROR

        CASE 'E':
        RESULTIS T("EQ") -> S.EQ,
                 T("ENTRY") -> S.ENTRY,
                 T("EQV") -> S.EQV,
                 T("ENDPROC") -> S.ENDPROC,
                 T("END") -> S.END, ERROR

        CASE 'F':
        RESULTIS T("FNAP") -> S.FNAP,
                 T("FNRN") -> S.FNRN,
                 T("FALSE") -> S.FALSE,
                 T("FINISH") -> S.FINISH, ERROR


        CASE 'G':
        RESULTIS T("GOTO") -> S.GOTO,
                 T("GE") -> S.GE,
                 T("GR") -> S.GR,
                 T("GLOBAL") -> S.GLOBAL, ERROR

        CASE 'I':
        RESULTIS T("ITEMN") -> S.ITEMN,
                 T("ITEML") -> S.ITEML,  ERROR

        CASE 'J':
        RESULTIS T("JUMP") -> S.JUMP,
                 T("JF") -> S.JF,
                 T("JT") -> S.JT,  ERROR

        CASE 'L':
        IF WP=2 DO
             SWITCHON S!2 INTO
             $( DEFAULT: RESULTIS ERROR
                CASE 'E': RESULTIS S.LE
                CASE 'N': RESULTIS S.LN
                CASE 'G': RESULTIS S.LG
                CASE 'P': RESULTIS S.LP
                CASE 'L': RESULTIS S.LL
                CASE 'S': RESULTIS S.LS  $)

        RESULTIS T("LAB") -> S.LAB,
                 T("LLG") -> S.LLG,
                 T("LLL") -> S.LLL,
                 T("LLP") -> S.LLP,
                 T("LOGAND") -> S.LOGAND,
                 T("LOGOR") -> S.LOGOR,
                 T("LSHIFT") -> S.LSHIFT,
                 T("LSTR") -> S.LSTR, ERROR

        CASE 'M':
        RESULTIS T("MINUS") -> S.MINUS,
                 T("MULT") -> S.MULT, ERROR

        CASE 'N':
        RESULTIS  T("NE") -> S.NE,
                  T("NEG") -> S.NEG,
                  T("NEQV") -> S.NEQV,
                  T("NOT") -> S.NOT,  ERROR

        CASE 'P':
        RESULTIS T("PLUS") -> S.PLUS, ERROR

        CASE 'Q':
        RESULTIS T("QUERY") -> S.QUERY, ERROR

        CASE 'R':
        RESULTIS T("RES") -> S.RES,
                 T("REM") -> S.REM,
                 T("RTAP") -> S.RTAP,
                 T("RTRN") -> S.RTRN,
                 T("RSHIFT") -> S.RSHIFT,
                 T("RSTACK") -> S.RSTACK,
                 T("RV") -> S.RV, ERROR

        CASE 'S':
        RESULTIS T("SG") -> S.SG,
                 T("SP") -> S.SP,
                 T("SL") -> S.SL,
                 T("STIND") -> S.STIND,
                 T("STACK") -> S.STACK,
                 T("SAVE") -> S.SAVE,
                 T("SWITCHON") -> S.SWITCHON,
                 T("STORE") -> S.STORE, ERROR

        CASE 'T':
        RESULTIS T("TRUE") -> S.TRUE, ERROR              $)1


AND RDN() = VALOF
    $(1 LET A, NEG = 0, FALSE

        CH := RDCH() REPEATWHILE CH='*N' \/ CH='*S'
        IF CH='-' DO $( NEG := TRUE; CH := RDCH()  $)

        WHILE '0' LE CH LE '9' DO
                  $( A := A*10 +CH - '0'
                     CH := RDCH()  $)

        RESULTIS NEG -> -A, A  $)1


AND RDL() = VALOF
    $(1 LET A = 0

        CH := RDCH() REPEATWHILE CH='*N' \/ CH='*S'

        IF CH='L' DO CH := RDCH()

        WHILE '0' LE CH LE '9' DO
                  $( A := A*10 + CH - '0'
                     CH := RDCH()  $)

        RESULTIS A   $)1


.

//    CG2


GET "CGHDR"

LET START(PARM) BE
    $(1 LET V = VEC 4000
        DATAV, DATAT := V, 4000
     $( LET V = VEC 50
        WORDV := V

        SYSIN := INPUT()
        SYSPRINT := OUTPUT()
        INTCODE := FINDOUTPUT("INTCODE")
        IF INTCODE=0 DO INTCODE := SYSPRINT

        PROGLENGTH := 0

        SELECTINPUT(SYSIN)
        SELECTOUTPUT(INTCODE)

     $( SSP, STATE := 2, NIL
        DATAP, LINEP,  PARAM := 0, 0, 500
        GENCODE() $) REPEATWHILE OP=S.GLOBAL

        SELECTOUTPUT(SYSPRINT)
        WRITEF("*NPROGRAM LENGTH = %N*N", PROGLENGTH)
        FINISH            $)1

.

//    CG3


GET "CGHDR"

LET GENCODE() BE
    $(1

NEXT: OP := READOP()

      SWITCHON OP INTO

   $( DEFAULT:    SELECTOUTPUT(SYSPRINT)
                  WRITEF("*NUNKNOWN KEY WORD:  %S*N", WORDV)
                  SELECTOUTPUT(INTCODE)
                  GOTO NEXT

      CASE S.END: RETURN

      CASE S.DEBUG:
           SELECTOUTPUT(SYSPRINT)
           WRITEF("*NSTATE=%N, SSP=%N, AD.A=%N, AD.K=%N*N",
                     STATE,    SSP,    AD.A,    AD.K)
           SELECTOUTPUT(INTCODE)
           GOTO NEXT

      CASE S.LP: LOAD(RDN(), M.IP); GOTO NEXT
      CASE S.LG: LOAD(RDN(), M.IG); GOTO NEXT
      CASE S.LL: LOAD(RDL(), M.IL); GOTO NEXT
      CASE S.LN: LOAD(RDN(), M.N); GOTO NEXT

      CASE S.LSTR: CGSTRING(RDN()); GOTO NEXT

      CASE S.TRUE:  LOAD(-1, M.N); GOTO NEXT
      CASE S.FALSE: LOAD(0, M.N); GOTO NEXT


      CASE S.LLP: LOAD(RDN(), M.P); GOTO NEXT
      CASE S.LLG: LOAD(RDN(), M.G); GOTO NEXT
      CASE S.LLL: LOAD(RDL(), M.L); GOTO NEXT

      CASE S.SP: STOREIN(RDN(), M.P); GOTO NEXT
      CASE S.SG: STOREIN(RDN(), M.G); GOTO NEXT
      CASE S.SL: STOREIN(RDL(), M.L); GOTO NEXT

      CASE S.STIND: FORCE.ACAD()
                    CODE(F.S, AD.A, AD.K)
                    SSP, STATE := SSP-2, NIL
                    GOTO NEXT

      CASE S.MULT:CASE S.DIV:CASE S.REM:
      CASE S.MINUS:CASE S.EQ:CASE S.NE:
      CASE S.LS:CASE S.GR:CASE S.LE:CASE S.GE:
      CASE S.LSHIFT:CASE S.RSHIFT:
      CASE S.LOGAND:CASE S.LOGOR:CASE S.NEQV:CASE S.EQV:
           FORCE.ACAD()
           CODE(F.L, AD.A, AD.K)
           CODE(F.X, OPCODE(OP), M.N)
           STATE, SSP := AC, SSP-1
           GOTO NEXT

      CASE S.RV:CASE S.NEG:CASE S.NOT:
           FORCE.AC()
           CODE(F.X, OPCODE(OP), M.N)
           GOTO NEXT

      CASE S.PLUS: FORCE.ACAD()
                   CODE(F.A, AD.A, AD.K)
                   STATE, SSP := AC, SSP-1
                   GOTO NEXT

      CASE S.JUMP: FORCE.NIL()
                   CODE(F.J, RDL(), M.L)
                   GOTO NEXT

      CASE S.JT:CASE S.JF:
                FORCE.AC()
                CODE(OP=S.JT->F.T,F.F, RDL(), M.L)
                SSP, STATE := SSP-1, NIL
                GOTO NEXT

      CASE S.GOTO: FORCE.AD()
                   CODE(F.J, AD.A, AD.K)
                   SSP, STATE := SSP-1, NIL
                   GOTO NEXT

      CASE S.LAB: FORCE.NIL()
                  COMPLAB(RDL())
                  GOTO NEXT

      CASE S.QUERY: FORCE.NIL(); SSP := SSP + 1; GOTO NEXT

      CASE S.STACK: FORCE.NIL(); SSP := RDN(); GOTO NEXT

      CASE S.STORE: FORCE.NIL(); GOTO NEXT

      CASE S.ENTRY: $( LET N = RDN()
                       LET L = RDL()
                       WR('*N'); WR('$')
                       FOR I = 1 TO N DO RDN()
                       WR(' ')
                       COMPLAB(L)
                       GOTO NEXT  $)

      CASE S.SAVE: SSP := RDN(); GOTO NEXT

      CASE S.ENDPROC: RDN(); GOTO NEXT
      CASE S.RTAP:
      CASE S.FNAP: $( LET K = RDN()
                      FORCE.AC()
                      CODE(F.K, K, M.N)
                      TEST OP=S.FNAP
                            THEN SSP, STATE := K+1, AC
                              OR SSP, STATE := K, NIL
                      GOTO NEXT   $)

      CASE S.FNRN: FORCE.AC()
                   SSP := SSP - 1
      CASE S.RTRN: CODE(F.X, OPCODE(S.RTRN), M.N)
                   STATE := NIL
                   GOTO NEXT

      CASE S.RES: FORCE.AC()
                  CODE(F.J, RDL(), M.L)
                  SSP, STATE := SSP-1, NIL
                  GOTO NEXT

      CASE S.RSTACK: FORCE.NIL()
                     SSP, STATE := RDN()+1, AC
                     GOTO NEXT

      CASE S.FINISH: CODE(F.X, OPCODE(OP), M.N); GOTO NEXT

      CASE S.SWITCHON:
          $( LET N = RDN()
             LET D = RDL()
             FORCE.AC()
             CODE(F.X, OPCODE(OP), M.N)
             CODE(F.D, N, M.N)
             CODE(F.D, D, M.L)
             SSP, STATE := SSP-1, NIL
             FOR I = 1 TO N DO
                  $( CODE(F.D, RDN(), M.N)
                     CODE(F.D, RDL(), M.L)  $)
             GOTO NEXT  $)


        CASE S.GLOBAL:
             WR('*N')
             FOR I = 0 TO DATAP-2 BY 2 DO WRDATA(DATAV!I, DATAV!(I+1))
             WR('*N')
             FOR I = 1 TO RDN() DO
                 $( WR('G'); WRN(RDN())
                    WR('L'); WRN(RDL()); WR('*S')  $)
             WR('*N'); WR('Z'); WR('*N')
             RETURN

      CASE S.DATALAB:
      CASE S.ITEML: DATA(OP, RDL())
                    GOTO NEXT

      CASE S.ITEMN: DATA(OP, RDN())
                    GOTO NEXT                   $)1

.

//    CG4


GET "CGHDR"

LET FORCE.NIL() BE
      SWITCHON STATE INTO
      $( CASE ACAD: CODE(F.S, SSP-2, M.P)

         CASE AD:   CODE(F.L, AD.A, AD.K)

         CASE AC:   CODE(F.S, SSP-1, M.P)
                    STATE := NIL

         CASE NIL:  $)

AND FORCE.AD() BE
      SWITCHON STATE INTO
      $( CASE ACAD: CODE(F.S, SSP-2, M.P)
                    GOTO L

         CASE AC:   CODE(F.S, SSP-1, M.P)

         CASE NIL: AD.A, AD.K := SSP-1, M.IP
         L:        STATE := AD

         CASE AD:    $)

AND FORCE.AC() BE
      SWITCHON STATE INTO
      $( CASE NIL:  CODE(F.L, SSP-1, M.IP)
                    GOTO L

         CASE ACAD: CODE(F.S, SSP-2, M.P)

         CASE AD:   CODE(F.L, AD.A, AD.K)
         L:         STATE := AC

         CASE AC:   $)

AND FORCE.ACAD() BE
      SWITCHON STATE INTO
      $( CASE AD:   CODE(F.L, SSP-2, M.IP)
                    GOTO L

         CASE AC:   CODE(F.S, SSP-1, M.P)

         CASE NIL:  CODE(F.L, SSP-2, M.IP)
                    AD.A, AD.K := SSP-1, M.IP
         L:         STATE := ACAD

         CASE ACAD:  $)

AND LOAD(A, K) BE
      SWITCHON STATE INTO
      $( CASE NIL: STATE := AD
                   GOTO M

         CASE ACAD:
         CASE AD:  FORCE.AC()
         CASE AC:  STATE := ACAD
         M:        AD.A, AD.K := A, K
                   SSP := SSP + 1  $)


AND STOREIN(A, K) BE
    $( FORCE.AC()
       CODE(F.S, A, K)
       SSP, STATE := SSP-1, NIL  $)

AND CGSTRING(N) BE
    $(1 LET L = NEXTPARAM()
        DATA(S.DATALAB, L)
        DATA(S.CHAR, N)
        FOR I = 1 TO N DO DATA(S.CHAR, RDN())
        LOAD(L, M.L)
        RETURN  $)1

AND DATA(K, V) BE
    $( LET P = DATAP
       DATAV!P, DATAV!(P+1) := K, V
       DATAP := DATAP + 2
       IF DATAP>DATAT DO
              $( SELECTOUTPUT(SYSPRINT)
                 WRITES("*NTOO MANY CONSTANTS*N")
                 SELECTOUTPUT(INTCODE)
                 DATAP := 0  $)  $)

AND NEXTPARAM() = VALOF $( PARAM := PARAM - 1
                           RESULTIS PARAM  $)

.

//    CG5


GET "CGHDR"

LET CODE(F, A, K) BE
    $( WR(F)
       SWITCHON K INTO
       $( CASE M.I: WR('I')
          CASE M.N: ENDCASE

          CASE M.IG: WR('I')
          CASE M.G:  WR('G')
                     ENDCASE

          CASE M.IP: WR('I')
          CASE M.P:  WR('P'); ENDCASE

          CASE M.IL: WR('I')
          CASE M.L:  WR('L'); ENDCASE  $)

       WRN(A)
       WR(' ')
       PROGLENGTH := PROGLENGTH + 1  $)

AND COMPLAB(N) BE $( WRN(N); WR(' ')  $)

AND WRDATA(K, N) BE SWITCHON K INTO
      $( CASE S.DATALAB: COMPLAB(N); RETURN

         CASE S.ITEMN: CODE(F.D, N, M.N); RETURN

         CASE S.ITEML: CODE(F.D, N, M.L); RETURN

         CASE S.CHAR:  CODE(F.C, N, M.N); RETURN  $)


AND OPCODE(OP) = VALOF SWITCHON OP INTO
     $( CASE S.RV: RESULTIS 1
        CASE S.NEG:RESULTIS 2
        CASE S.NOT:RESULTIS 3
        CASE S.RTRN:RESULTIS 4
        CASE S.MULT:  RESULTIS 5
        CASE S.DIV:   RESULTIS 6
        CASE S.REM:   RESULTIS 7
        CASE S.PLUS:  RESULTIS 8
        CASE S.MINUS: RESULTIS 9
        CASE S.EQ:    RESULTIS 10
        CASE S.NE:    RESULTIS 11
        CASE S.LS:    RESULTIS 12
        CASE S.GE:    RESULTIS 13
        CASE S.GR:    RESULTIS 14
        CASE S.LE:    RESULTIS 15
        CASE S.LSHIFT:RESULTIS 16
        CASE S.RSHIFT:RESULTIS 17
        CASE S.LOGAND:RESULTIS 18
        CASE S.LOGOR: RESULTIS 19
        CASE S.NEQV:  RESULTIS 20
        CASE S.EQV:   RESULTIS 21
        CASE S.FINISH:RESULTIS 22
        CASE S.SWITCHON:RESULTIS 23

        DEFAULT: SELECTOUTPUT(SYSPRINT)
                 WRITEF("*NUNKNOWN OP %N*N", OP)
                 SELECTOUTPUT(INTCODE)
                 RESULTIS 0  $)


AND WR(CH) BE
    $( IF CH='*N' DO $( WRCH('*N')
                        LINEP := 0
                        RETURN  $)

       IF LINEP=71 DO
              $( WRCH('/')
                 WRCH('*N')
                 LINEP := 0  $)
       LINEP := LINEP + 1
       WRCH(CH)  $)

AND WRN(N) BE
    $( TEST N<0 DO WR('-') OR N := -N
       IF N<-9 DO WRN(-N/10)
       WR('0' - N REM 10)  $)
