|       csa09 Small C v1 comparison support
|       All are dyadic except for lneg.
.globl  __eq
.globl  __ne
.globl  __lt
.globl  __le
.globl  __gt
.globl  __ge
.globl  __ult
.globl  __ule
.globl  __ugt
.globl  __uge
.globl  __lneg
.globl  __bool

__eq:   cmpd 2,s
        lbeq true
        lbra false

__ne:   cmpd 2,s
        lbne true
        lbra false

__lt:   cmpd 2,s
        bgt true
        bra false

__le:   cmpd 2,s
        bge true
        bra false

__gt:   cmpd 2,s
        blt true
        bra false

__ge:   cmpd 2,s
        ble true
        bra false

__ult:  cmpd 2,s
        bhi true
        bra false

__ule:  cmpd 2,s
        bhs true
        bra false

__ugt:  cmpd 2,s
        blo true
        bra false

__uge:  cmpd 2,s
        bls true
        bra false

__lneg: cmpd #0
        beq ltrue
        ldd #0
        rts
ltrue:  ldd #1
        rts

__bool: bsr     __lneg
        bra     __lneg

true:   ldd #1
        ldx ,s
        leas 4,s
        jmp ,x

false:  clra
        clrb
        ldx ,s
        leas 4,s
        jmp  ,x
