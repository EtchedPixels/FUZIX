|       signed divide
|       calling: (left / right)
|               push left
|               ldd right
|               jsr sdiv
|       result in d, arg popped.
|
|       left=6
|       right=2
|       sign=1
|       count=0
|       return=4
|       CARRY=1
.globl __sdiv,__div
.globl __prabs
__sdiv: leas    -4,s
        std     2,s
        bne     nozero
	ldd	#8	; SIGFPE
	pshs	d,x	; need a dummy
	ldx	#0
	ldd	#39	; kill
	swi		; kill(0,SIGFPE)
	puls	d,x
	rts
;
;	FIXME - self signal
;

nozero: jsr     __prabs
__div:  clr     0,s        ; prescale divisor
        inc     0,s
mscl:   inc     0,s
        aslb
        rola
        bpl     mscl
        std     2,s
        ldd     6,s
        clr     6,s
        clr     6+1,s
div1:   subd    2,s        ; check subtract
        bcc     div2
        addd    2,s
        andcc   #~1
        bra     div3
div2:   orcc    #1
div3:   rol     6+1,s       ; roll in carry
        rol     6,s
        lsr     2,s
        ror     2+1,s
        dec     0,s
        bne     div1
        ldd     6,s
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
