bdos (c, de) int c, de; {
#asm
;       CP/M support routine
;       bdos(C,DE);
;       char *DE; int C;
;       returns H=B,L=A per CPM standard
        pop     h       ; hold return address
        pop     d       ; get bdos function number
        pop     b       ; get DE register argument
        push    d
        push    b
        push    h
        call    5
        mov     h,b
        mov     l,a
#endasm
}

