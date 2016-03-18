// $Id: blib.bcpl,v 1.3 2004/12/21 13:08:58 rn Exp $

//   BLIB

GET "LIBHDR"

LET WRITES(S) BE  FOR I = 1 TO GETBYTE(S, 0) DO WRCH(GETBYTE(S, I))

AND UNPACKSTRING(S, V) BE
         FOR I = 0 TO GETBYTE(S, 0) DO V!I := GETBYTE(S, I)

AND PACKSTRING(V, S) = VALOF
    $( LET N = V!0 & 255
       LET I = N/2
       FOR P = 0 TO N DO PUTBYTE(S, P, V!P)
       SWITCHON N&1 INTO
       $(
          CASE 0: PUTBYTE(S, N+1, 0)
          CASE 1: $)
       RESULTIS I  $)

// THE DEFINITIONS THAT FOLLOW ARE MACHINE INDEPENDENT

AND WRITED(N, D) BE

$(1 LET T = VEC 20
    AND I, K = 0, N
    TEST N<0 THEN D := D-1 ELSE K := -N
    T!I, K, I := K REM 10, K/10, I+1 REPEATUNTIL K=0
    FOR J = I+1 TO D DO WRCH('*S')
    IF N<0 DO WRCH('-')
    FOR J = I-1 TO 0 BY -1 DO WRCH('0'-T!J)  $)1

AND WRITEN(N) BE WRITED(N, 0)


AND NEWLINE() BE WRCH('*N')

AND NEWPAGE() BE WRCH('*P')

AND READN() = VALOF

$(1 LET SUM = 0
    AND NEG = FALSE

L: TERMINATOR := RDCH()
    SWITCHON TERMINATOR INTO
    $(  CASE '*S':
        CASE '*T':
        CASE '*N':    GOTO L

        CASE '-':     NEG := TRUE
        CASE '+':     TERMINATOR := RDCH()   $)
    WHILE '0'<=TERMINATOR<='9' DO
                 $( SUM := 10*SUM + TERMINATOR - '0'
                    TERMINATOR := RDCH()  $)
    IF NEG DO SUM := -SUM
    RESULTIS SUM   $)1

AND WRITEOCT(N, D) BE
    $( IF D>1 DO WRITEOCT(N>>3, D-1)
       WRCH((N/\7)+'0')  $)

AND WRITEHEX(N, D) BE
    $( IF D>1 DO WRITEHEX(N>>4, D-1)
       WRCH((N&15)!TABLE
            '0','1','2','3','4','5','6','7',
            '8','9','A','B','C','D','E','F')  $)


AND WRITEF(FORMAT, A, B, C, D, E, F, G, H, I, J, K) BE

$(1 LET T = @A

    FOR P = 1 TO GETBYTE(FORMAT, 0) DO
    $(2 LET K = GETBYTE(FORMAT, P)

        TEST K='%'

          THEN $(3 LET F, Q, N = 0, T!0, 0
                   AND TYPE = GETBYTE(FORMAT, P+1)
                   P := P + 1
                   SWITCHON TYPE INTO
                $( DEFAULT: WRCH(TYPE); ENDCASE

                   CASE 'S': F := WRITES; GOTO L
                   CASE 'C': F := WRCH; GOTO L
                   CASE 'O': F := WRITEOCT; GOTO M
                   CASE 'X': F := WRITEHEX; GOTO M
                   CASE 'I': F := WRITED; GOTO M
                   CASE 'N': F := WRITED; GOTO L

                M: P := P + 1
                   N := GETBYTE(FORMAT, P)
                   N := '0'<=N<='9' -> N-'0', N-'A'+10

                L: F(Q, N); T := T + 1  $)3

            OR WRCH(K)  $)2  $)1


AND MAPSTORE() BE WRITES("*NMAPSTORE NOT IMPLEMENTED*N")
