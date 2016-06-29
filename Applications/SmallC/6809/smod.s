|       signed mod
|       calling: (left / right)
|               push left
|               ldd right
|               jsr smod
|       result in d, arg popped.
|
|       left=6
|       right=2
|       sign=1
|       count=0
|       return=4
|       CARRY=1
.globl __smod,__mod,ASSERTFAIL
.globl __mrabs
__smod: leas    -4,s
        std     2,s
        bne     nozero
	ldd	#8	; SIGFPE
	pshs	d,x	; need a dummy
	ldd	#39	; sigkill
	swi
	puls	d,x
	rts
nozero: jsr     __mrabs
__mod:  clr     0,s        ; prescale divisor
        inc     0,s
mscl:   inc     0,s
        aslb
        rola
        bpl     mscl
        std     2,s
        ldd     6,s
        clr     6,s
        clr     6+1,s
mod1:   subd    2,s        ; check subtract
        bcc     mod2
        addd    2,s
        andcc   #~1
        bra     mod3
mod2:   orcc    #1
mod3:   rol     6+1,s       ; roll in carry
        rol     6,s
        lsr     2,s
        ror     2+1,s
        dec     0,s
        bne     mod1
        tst     1,s         ; sign fiddle
        beq     nochg
        nega
        negb
        sbca    #0
nochg:  std     2,s        ; move return addr
        ldd     4,s
        std     6,s
        ldd     2,s
        leas    6,s
        rts
