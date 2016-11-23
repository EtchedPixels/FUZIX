// a first program.
// bcpl hello.b
// icint hello.i

GET "LIBHDR"

LET START() BE
    $( LET A, B, C, SUM = 1, 2, 3, 0
       SUM := A + B + C
       WRITES("Sum of 1 + 2 + 3 is ")
       WRITEN(SUM)
       WRITES("*NHello, World*N")
    $)
