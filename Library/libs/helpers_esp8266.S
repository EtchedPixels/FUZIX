.macro helper N, A
    .global \N
    .align 4
    \N:
        movi a6, \A
        jx a6
.endm

helper __divdi3, 0x4000ce60
helper __divsi3, 0x4000dc88
helper __muldi3, 0x40000650
helper __udivdi3, 0x4000d310
helper __udivsi3, 0x4000e21c
helper __umoddi3, 0x4000d770
helper __umodsi3, 0x4000e268
helper __umulsidi3, 0x4000dcf0

# vim: sw=4 ts=4 et
