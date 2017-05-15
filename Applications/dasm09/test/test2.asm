;
; 6309 assembler/disassembler test file (12 Aug 1999)
;

     adcd #$0101
     adcd <$01
     adcd ,x
     adcd $0101

     adde #$01
     adde <$01
     adde ,x
     adde $0101

     addf #$01
     addf <$01
     addf ,x
     addf $0101

     addw #$0101
     addw <$01
     addw ,x
     addw $0101

     aim #$01,<$01
     aim #$01,$0101
     aim #$01,,x

     addd #$0101
     addd <$01
     addd ,x
     addd $0101

     asld

     asrd

     bitmd #$01

     clrd
     clre
     clrf
     clrw

     cmpe #$01
     cmpe <$01
     cmpe ,x
     cmpe $0101

     cmpf #$01
     cmpf <$01
     cmpf ,x
     cmpf $0101

     cmpw #$0101
     cmpw <$01
     cmpw ,x
     cmpw $0101

     comd
     come
     comf
     comw

     decd
     dece
     decf
     decw

     divd #$01
     divd <$01
     divd ,x
     divd $0101

     divq #$0101
     divq <$01
     divq ,x
     divq $0101

     eim #$01,<$01
     eim #$01,$0101
     eim #$01,,x

     eord #$0101
     eord <$01
     eord ,x
     eord $0101

;tfr exg

     incd
     ince
     incf
     incw

     lde #$01
     lde <$01
     lde ,x
     lde $0101

     ldf #$01
     ldf <$01
     ldf ,x
     ldf $0101

     ldq #$01010101
     ldq <$01
     ldq ,x
     ldq $0101

     ldw #$0101
     ldw <$01
     ldw ,x
     ldw $0101

     ldmd #$01

     lsrd
     lsrw

     muld #$0101
     muld <$01
     muld ,x
     muld $0101

     negd

     oim #$01,<$01
     oim #$01,$0101
     oim #$01,,x

     ord #$0101
     ord <$01
     ord ,x
     ord $0101

     pshsw
     pshuw

     pulsw
     puluw

     rold
     rolw

     rord
     rorw

     sbcd #$0101
     sbcd <$01
     sbcd ,x
     sbcd $0101

     sexw

     ste <$01
     ste $0101
     ste ,x

     stf <$01
     stf $0101
     stf ,x

     stq <$01
     stq $0101
     stq ,x

     stw <$01
     stw $0101
     stw ,x

     sube #$01
     sube <$01
     sube ,x
     sube $0101

     subf #$01
     subf <$01
     subf ,x
     subf $0101

     subw #$0101
     subw <$01
     subw ,x
     subw $0101

     tim #$01,<$01
     tim #$01,$0101
     tim #$01,,x

     tstd
     tste
     tstf
     tstw

     band cc.0,<$01.0
     band cc.1,<$01.1
     band cc.2,<$01.2
     band cc.3,<$01.3
     band cc.4,<$01.4
     band cc.5,<$01.5
     band cc.6,<$01.6
     band cc.7,<$01.7
     band cc.7,<$01.0
     band cc.0,<$01.7

     band a.0,<$01.0
     band a.1,<$01.1
     band a.2,<$01.2
     band a.3,<$01.3
     band a.4,<$01.4
     band a.5,<$01.5
     band a.6,<$01.6
     band a.7,<$01.7
     band a.7,<$01.0
     band a.0,<$01.7

     band b.0,<$01.0
     band b.1,<$01.1
     band b.2,<$01.2
     band b.3,<$01.3
     band b.4,<$01.4
     band b.5,<$01.5
     band b.6,<$01.6
     band b.7,<$01.7
     band b.7,<$01.0
     band b.0,<$01.7

     biand a.0,<$01.7
     bor a.0,<$01.7
     bior a.0,<$01.7
     beor a.0,<$01.7
     bieor a.0,<$01.7
     ldbt a.0,<$01.7
     stbt a.0,<$01.7

     adcr a,b
     addr a,b
     andr a,b
     cmpr a,b
     eorr a,b
     orr  a,b
     sbcr a,b
     subr a,b

     tfm x+,y+
     tfm y-,x-
     tfm s+,u
     tfm y,u+

     leax e,x
     leax f,x
     leax e,y
     leax f,y
     leax e,s
     leax f,s
     leax e,u
     leax f,u

     leax [e,x]
     leax [f,x]
     leax [e,y]
     leax [f,y]
     leax [e,s]
     leax [f,s]
     leax [e,u]
     leax [f,u]

     leax w,x
     leax w,y
     leax w,s
     leax w,u

     leax [w,x]
     leax [w,y]
     leax [w,s]
     leax [w,u]

     leax ,w
     leax $8000,w
     leax ,w++
     leax ,--w

     leax [,w]
     leax [$8000,w]
     leax [,w++]
     leax [,--w]


