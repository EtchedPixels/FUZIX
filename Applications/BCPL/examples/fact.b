 GLOBAL $( START:1; WRITEF:76 $)
 LET START () BE $(1
 LET F(N) = N=0 -> 1, N*F(N-1)
 FOR I = 1 TO 10 DO WRITEF("F(%N), = %N*N", I, F(I))
 FINISH $)1
