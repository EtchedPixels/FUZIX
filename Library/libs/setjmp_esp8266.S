    .text
    .align 4
    .global setjmp

setjmp:
    s32i a0, a2, 0
    s32i sp, a2, 4
    s32i a12, a2, 8
    s32i a13, a2, 12
    s32i a14, a2, 16
    s32i a15, a2, 20
    movi a2, 0
    ret

