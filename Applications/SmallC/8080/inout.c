inp(pno) char pno; {
        pno;
#asm
        mov     a,l
        sta     ininst+1 <tel:+1>
ininst  in      0       ; self modifying code...
        mov     l,a
        xra     a
        mov     h,a
        ret
#endasm

}

outp(pno, val) char pno, val; {
        pno;
#asm
        mov     a,l
        sta     outinst+1 <tel:+1>
#endasm
        val;
#asm
        mov     a,l
outinst out     0
        ret
#endasm
}

