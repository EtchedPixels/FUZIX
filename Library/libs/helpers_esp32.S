.macro helper N, A
    .global \N
    .align 4
    \N:
        movi a6, \A
        jx a6
.endm

helper __absvdi2, 0x4006387c
helper __absvsi2, 0x40063868
helper __adddf3, 0x40002590
helper __addsf3, 0x400020e8
helper __addvdi3, 0x40002cbc
helper __addvsi3, 0x40002c98
helper __ashldi3, 0x4000c818
helper __ashrdi3, 0x4000c830
helper __bswapdi2, 0x40064b08
helper __bswapsi2, 0x40064ae0
helper __clrsbdi2, 0x40064b7c
helper __clrsbsi2, 0x40064b64
helper __clzdi2, 0x4000ca50
helper __clzsi2, 0x4000c7e8
helper __cmpdi2, 0x40063820
helper __ctzdi2, 0x4000ca64
helper __ctzsi2, 0x4000c7f0
helper __divdc3, 0x400645a4
helper __divdf3, 0x40002954
helper __divdi3, 0x4000ca84
helper __divsi3, 0x4000c7b8
helper __eqdf2, 0x400636a8
helper __eqsf2, 0x40063374
helper __extendsfdf2, 0x40002c34
helper __ffsdi2, 0x4000ca2c
helper __ffssi2, 0x4000c804
helper __fixdfdi, 0x40002ac4
helper __fixdfsi, 0x40002a78
helper __fixsfdi, 0x4000244c
helper __fixsfsi, 0x4000240c
helper __fixunsdfsi, 0x40002b30
helper __fixunssfdi, 0x40002504
helper __fixunssfsi, 0x400024ac
helper __floatdidf, 0x4000c988
helper __floatdisf, 0x4000c8c0
helper __floatsidf, 0x4000c944
helper __floatsisf, 0x4000c870
helper __floatundidf, 0x4000c978
helper __floatundisf, 0x4000c8b0
helper __floatunsidf, 0x4000c938
helper __floatunsisf, 0x4000c864
helper __gcc_bcmp, 0x40064a70
helper __gedf2, 0x40063768
helper __gesf2, 0x4006340c
helper __gtdf2, 0x400636dc
helper __gtsf2, 0x400633a0
helper __ledf2, 0x40063704
helper __lesf2, 0x400633c0
helper __lshrdi3, 0x4000c84c
helper __ltdf2, 0x40063790
helper __ltsf2, 0x4006342c
helper __moddi3, 0x4000cd4c
helper __modsi3, 0x4000c7c0
helper __muldc3, 0x40063c90
helper __muldf3, 0x4006358c
helper __muldi3, 0x4000c9fc
helper __mulsf3, 0x400632c8
helper __mulsi3, 0x4000c7b0
helper __mulvdi3, 0x40002d78
helper __mulvsi3, 0x40002d60
helper __nedf2, 0x400636a8
helper __negdf2, 0x400634a0
helper __negdi2, 0x4000ca14
helper __negsf2, 0x400020c0
helper __negvdi2, 0x40002e98
helper __negvsi2, 0x40002e78
helper __nesf2, 0x40063374
helper __nsau_data, 0x3ff96544
helper __paritysi2, 0x40002f3c
helper __popcount_tab, 0x3ff96544
helper __popcountdi2, 0x40002ef8
helper __popcountsi2, 0x40002ed0
helper __powidf2, 0x400638e4
helper __subdf3, 0x400026e4
helper __subsf3, 0x400021d0
helper __subvdi3, 0x40002d20
helper __subvsi3, 0x40002cf8
helper __truncdfsf2, 0x40002b90
helper __ucmpdi2, 0x40063840
helper __udiv_w_sdiv, 0x40064bec
helper __udivdi3, 0x4000cff8
helper __udivmoddi4, 0x40064bf4
helper __udivsi3, 0x4000c7c8
helper __umoddi3, 0x4000d280
helper __umodsi3, 0x4000c7d0
helper __umulsidi3, 0x4000c7d8
helper __unorddf2, 0x400637f4
helper __unordsf2, 0x40063478

helper setjmp, 0x40056268
helper longjmp, 0x400562cc

# vim: sw=4 ts=4 et
