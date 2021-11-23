    .text
    .align 4
    .global longjmp

longjmp:
    l32i a0, a2, 0
    l32i a12, a2, 8
    l32i a13, a2, 12
    l32i a14, a2, 16
    l32i a15, a2, 20
    l32i sp, a2, 4
    /* Return val ? val : 1.  */
    movi a2, 1
    movnez a2, a3, a3
    ret

