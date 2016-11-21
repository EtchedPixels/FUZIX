GET "LIBHDR"

// THIS IS A UNIVERSAL CODE-GENERATOR TEST PROGRAM
// WRITTEN BY M. RICHARDS ORIGINALLY TO TEST THE
// CII 10070 CODE-GENERATOR.

GLOBAL $( F:100; G:101; H:102
          TESTNO:103; FAILCOUNT:104
          V:105; TESTCOUNT:106; QUIET:107; T:108  $)

STATIC $( A=10; B=11; C=12; W=0  $)

MANIFEST $( K0=0; K1=1; K2=2  $)

LET T(X, Y) = VALOF
   $( TESTNO := TESTNO + 1
      TESTCOUNT := TESTCOUNT + 1
      IF X=Y & QUIET RESULTIS Y
      WRITEF("%I3 %I5 ", TESTNO, Y)
      TEST X=Y
         THEN WRITES("OK*N")
         ELSE $( WRITEF("FAILED %X8(%N) %X8(%N)*N", X, X, Y, Y)
                 FAILCOUNT := FAILCOUNT + 1  $)
      RESULTIS Y  $)


LET T1(A,B,C,D,E,F,G) = T(A+B+C+D+E+F, G)

LET START(PARM) BE
$(1 LET V1 = VEC 200
    AND V2 = VEC 200
    TESTER(0, 1, 2, V1, V2)
$)1

AND TIMEOFDAY() = "Now"

AND DATE() = "Today"

AND TESTER(X, Y, Z, V1, V2) BE
$(1 WRITEF("*NCGTESTER ENTERED %S %S*N*N",
            TIMEOFDAY(), DATE())

//  FIRST INITIALIZE CERTAIN VARIABLES

    F, G, H := 100, 101, 102
    TESTNO, TESTCOUNT, FAILCOUNT := 0, 0, 0
    V, W := V1, V2

    FOR I = 0 TO 200 DO V!I, W!I := 1000+I, 10000+I


    QUIET := FALSE
//  QUIET := GETBYTE(PARM,0)>0 & GETBYTE(PARM,1)='Q' -> TRUE, FALSE

//  TEST SIMPLE VARIABLES AND EXPRESSIONS

    T(A+B+C, 33)
    T(F+G+H, 303)
    T(X+Y+Z, 3)

    T(123+321-400, 44)
    T(X=0, TRUE)
    T(Y=0, FALSE)
    T(!(@Y+X), 1)
    T(!(@B+X), 11)
    T(!(@G+X), 101)

    X, A, F := 5, 15, 105
    T(X, 5)
    T(A, 15)
    T(F, 105)

    V!1, V!2 := 1234, 5678
    T(V!1, 1234)
    T(V!Z, 5678)

    T(X*A, 75)
    T(1*X+2*Y+3*Z+F*4,433)
    T(X*A+A*X, 150)

    T(100/2, 50)
    T(A/X, 3)
    T(A/-X, -3)
    T((-A)/X, -3)
    T((-A)/(-X), 3)
    T((A+A)/A, 2)
    T((A*X)/(X*A), 1)
    T((A+B)*(X+Y)*123/(6*123), 26)

    T(7 REM 2, 1)
    T(F REM 100, 5)
    T(A REM X, 0)

    T(-F, -105)
    T(F=105, TRUE)
    T(F NE 105, FALSE)
    T(F<105, FALSE)
    T(F>=105, TRUE)
    T(F>105, FALSE)
    T(F<=105, TRUE)

    T(#1775<<3, #17750)
    T(#1775>>3, #177)
    T(#1775<<Z+1, #17750)
    T(#1775>>Z+1, #177)

    T(#B1100&#B1010, #B1000)
    T(#B1100 \/ #B1010, #B1110)
    T((#B1100 EQV   #B1010) & #B11111, #B11001)
    T(#B1100 NEQV  #B1010, #B0110)

    T(NOT TRUE, FALSE)
    T(NOT FALSE, TRUE)
    T(NOT(1234 EQV -4321), 1234 NEQV -4321)

    T(-F, -105)

    T(!V, 1000)
    T(V!0, 1000)
    T(V!1, 1234)
    T(V!(!V-998), 5678)
    T(!W, 10000)
    T(W!0, 10000)
    T(0!W, 10000)
    T(1!W, 10001)
    T(W!1, 10001)
    T(!(W+200), 10200)

    A := TRUE
    B := FALSE

    IF A DO X := 16
    T(X, 16)
    X := 16

    IF B DO X := 15
    T(X, 16)
    X := 15

    $( LET W = VEC 20
       GOTO L1
    L2: WRITES("GOTO ERROR*N")
        FAILCOUNT := FAILCOUNT+1  $)

L1: A := VALOF RESULTIS 11
    T(A, 11)

    TESTNO := 100  // TEST SIMULATED STACK ROUTINES

    $( LET V1 = VEC 1
       V1!0, V1!1 := -1, -2
       $( LET V2 = VEC 10
          FOR I = 0 TO 10 DO V2!I := -I
          T(V2!5, -5)  $)
       T(V1!1, -2)  $)

    X := X + T(X,15, T(F, 105), T(A, 11)) - 15
    T(X, 15)

    X := X+1
    T(X, 16)
    X := X-1
    T(X, 15)
    X := X+7
    T(X,22)
    X := X-22
    T(X, 0)
    X := X+15
    T(X, 15)
    X := X + F
    T(X, 120)
    X := 1

    TESTNO := 200  // TEST SWITCHON COMMANDS

$(SW LET S1, S1F = 0, 0
     AND S2, S2F = 0, 0

     FOR I = -200 TO 200 DO
     $( SWITCHON I INTO
         $( DEFAULT: S1 := S1+1000; ENDCASE
            CASE -1000: S1F := S1F + I; ENDCASE
            CASE -200: S1 := S1 + 1
            CASE -190: S1 := S1 + 1
            CASE -180: S1 := S1 + 1
            CASE   -5: S1 := S1 + 1
            CASE    0: S1 := S1 + 1
            CASE -145: S1 := S1 + 1
            CASE    7: S1 := S1 + 1
            CASE    8: S1 := S1 + 1
            CASE  200: S1 := S1 + 1
            CASE  190: S1 := S1 + 1
            CASE  100: S1 := S1 + 1
            CASE   90: S1 := S1 + 1
            CASE  199: S1 := S1 + 1
            CASE   95: S1 := S1 + 1
            CASE   76: S1 := S1 + 1
            CASE   88: S1 := S1 + 1
            CASE   99: S1 := S1 + 1
            CASE  -98: S1 := S1 + 1
            CASE   11: S1 := S1 + 1
            CASE   12: S1 := S1 + 1
            CASE   13: S1 := S1 + 1
            CASE   41: S1 := S1 + 1
            CASE   91: S1 := S1 + 1
            CASE   92: S1 := S1 + 1
            CASE   71: S1 := S1 + 1
            CASE   73: S1 := S1 + 1
            CASE   74: S1 := S1 + 1
            CASE   81: S1 := S1 + 1
            CASE   82: S1 := S1 + 1
            CASE   61: S1 := S1 + 1
            CASE -171: S1 := S1 + 1
            CASE -162: S1 := S1 + 1  $)

        SWITCHON I+10000 INTO
         $( DEFAULT: S2 := S2+1000; ENDCASE
            CASE 10020: S2 := S2 + 1
            CASE 10021: S2 := S2 + 1
            CASE 10022: S2 := S2 + 1
            CASE 10023: S2 := S2 + 1
            CASE 10024: S2 := S2 + 1
            CASE 10025: S2 := S2 + 1
            CASE 10026: S2 := S2 + 1
            CASE 10027: S2 := S2 + 1
            CASE 10028: S2 := S2 + 1
            CASE 10029: S2 := S2 + 1
            CASE 10010: S2 := S2 + 1
            CASE 10011: S2 := S2 + 1
            CASE 10012: S2 := S2 + 1
            CASE 10013: S2 := S2 + 1
            CASE 10014: S2 := S2 + 1
            CASE 10015: S2 := S2 + 1  $)

     $)
     T(S1F, 0)
     T(S2F, 0)
     T(S1, (401-32)*1000 + 32*(32+1)/2)
     T(S2, (401-16)*1000 + 16*(16+1)/2)
$)SW


    TESTNO := 250  // TEST FUNCTION CALLING

      T1(1,2,3,4,5,6, 21)
      T1(T(1,1), T(2,2), T(3,3), T(4,4), T(5,5), T(6,6),
         T(21,21))
      T1(VALOF RESULTIS 1,
         VALOF RESULTIS 2,
         VALOF RESULTIS 3,
         VALOF RESULTIS 4,
         VALOF RESULTIS 5,
         VALOF RESULTIS 6,
         21)
      T1(VALOF RESULTIS 1,
         T(2,2),
         VALOF RESULTIS 3,
         T(4,4),
         VALOF RESULTIS 5,
         T(6,6),
         21)
     T1( 1, T(2,2), VALOF RESULTIS 3,
         4, T(5,5), VALOF RESULTIS 6,
         21)
     T1(!V,V!0,V!200,!W,W!0,W!200, 2*1000+1200+2*10000+10200)
     (T1+(X+X)/X-2)(1,1,1,1,1,1,6)
     (!@T1)(1,2,3,4,5,6,21)

     TESTNO := 300  // TEST EXPRESSION OPERATORS

     T((2+3)+F+6,116)
     T(F+2+3+6,116)
     T(6+3+2+F, 116)
     T(F-104, 1)
     T((X+2)=(X+2)->99,98, 99)
     T(F<F+1->21,22, 21)
     T(F>F+1->31,32, 32)
     T(F<=105->41,42, 41)
     T(F>=105->51,52, 51)

    TESTNO := 400  // TEST REGISTER ALLOCATION ETC.

    X := 0
    Y := 1
    Z := 2
    T(X, 0)
    T(Y, 1)
    T(Z, 2)
    F,G,H := 101,102,103
    A,B,C := 11,12,13
    T(X+1,1)
    T(F+1, 102)
    T(A+1, 12)
    T(!(@A*2/2+F-101),11)
    A := @F
    T(!A, 101)
    B := @G
    A := @B
    T(!!A, 102)
    W!0 := @W!1
    W!1 := @H
    T(Z*Y+(W!0)!0!0-2, 103)
    T(Z*Y+W!1!0-2, 103)
    T(T(123,123),T(123,123))

    WRITEF("*N%N TESTS COMPLETED, %N FAILURE(S)*N",
            TESTCOUNT, FAILCOUNT)
$)1


