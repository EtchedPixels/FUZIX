.macro helper N, A
    .global \N
    .align 4
    \N:
        movi a6, \A
        jx a6
.endm

helper __adddf3, 0x4000c538
helper __addsf3, 0x4000c180
helper __divdf3, 0x4000cb94
helper __divdi3, 0x4000ce60
helper __divsi3, 0x4000dc88
helper __extendsfdf2, 0x4000cdfc
helper __fixdfsi, 0x4000ccb8
helper __fixunsdfsi, 0x4000cd00
helper __fixunssfsi, 0x4000c4c4
helper __floatsidf, 0x4000e2f0
helper __floatsisf, 0x4000e2ac
helper __floatunsidf, 0x4000e2e8
helper __floatunsisf, 0x4000e2a4
helper __muldf3, 0x4000c8f0
helper __muldi3, 0x40000650
helper __mulsf3, 0x4000c3dc
helper __subdf3, 0x4000c688
helper __subsf3, 0x4000c268
helper __truncdfsf2, 0x4000cd5c
helper __udivdi3, 0x4000d310
helper __udivsi3, 0x4000e21c
helper __umoddi3, 0x4000d770
helper __umodsi3, 0x4000e268
helper __umulsidi3, 0x4000dcf0

# vim: sw=4 ts=4 et
