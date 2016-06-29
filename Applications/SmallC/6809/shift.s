|       Shift support for Small C v1 sa09
.globl __asr
__asr:  tstb
        bge     okr
        negb
        bra     __asl
okr:    incb
        pshs    b
        ldd     3,s
asrl:   dec     ,s
        beq     return
        asra
        rorb
        bra     asrl

.globl __asl
__asl:  tstb
        bge     okl
        negb
        bra     __asr
okl:    incb
        pshs    b
        ldd     3,s
asll:   dec     ,s
        beq     return
        aslb
        rola
        bra     asll

return: ldx     1,s
        leas    5,s
        jmp     ,x
