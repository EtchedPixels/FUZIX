;
;*****************************************************
;                                                    *
;       runtime library for small C compiler         *
;                                                    *
;       c.s - runtime routine for basic C code       *
;                                                    *
;               Ron Cain                             *
;                                                    *
;*****************************************************
;
        cseg
;
        public  ?gchar,?gint,?pchar,?pint
        public  ?sxt
        public  ?or,?and,?xor
        public  ?eq,?ne,?gt,?le,?ge,?lt,?uge,?ult,?ugt,?ule
        public  ?asr,?asl
        public  ?sub,?neg,?com,?lneg,?bool,?mul,?div
        public  ?case,brkend,Xstktop
        public  etext
        public  edata
;
; fetch char from (HL) and sign extend into HL
?gchar: mov     a,m
?sxt:   mov     l,a
        rlc
        sbb     a
        mov     h,a
        ret
; fetch int from (HL)
?gint:  mov     a,m
        inx     h
        mov     h,m
        mov     l,a
        ret
; store char from HL into (DE)
?pchar: mov     a,l
        stax    d
        ret
; store int from HL into (DE)
?pint:  mov     a,l
        stax    d
        inx     d
        mov     a,h
        stax    d
        ret
; "or" HL and DE into HL
?or:    mov     a,l
        ora     e
        mov     l,a
        mov     a,h
        ora     d
        mov     h,a
        ret
; "xor" HL and DE into HL
?xor:   mov     a,l
        xra     e
        mov     l,a
        mov     a,h
        xra     d
        mov     h,a
        ret
; "and" HL and DE into HL
?and:   mov     a,l
        ana     e
        mov     l,a
        mov     a,h
        ana     d
        mov     h,a
        ret
;
;......logical operations: HL set to 0 (false) or 1 (true)
;
; DE == HL
?eq:    call    ?cmp
        rz
        dcx     h
        ret
; DE != HL
?ne:    call    ?cmp
        rnz
        dcx     h
        ret
; DE > HL [signed]
?gt:    xchg
        call    ?cmp
        rc
        dcx     h
        ret
; DE <= HL [signed]
?le:    call    ?cmp
        rz
        rc
        dcx     h
        ret
; DE >= HL [signed]
?ge:    call    ?cmp
        rnc
        dcx     h
        ret
; DE < HL [signed]
?lt:    call    ?cmp
        rc
        dcx     h
        ret
; DE >= HL [unsigned]
?uge:   call    ?ucmp
        rnc
        dcx     h
        ret
; DE < HL [unsigned]
?ult:   call    ?ucmp
        rc
        dcx     h
        ret
; DE > HL [unsigned]
?ugt:   xchg
        call    ?ucmp
        rc
        dcx     h
        ret
; DE <= HL [unsigned]
?ule:   call    ?ucmp
        rz
        rc
        dcx     h
        ret
; signed compare of DE and HL
;   carry is sign of difference [set => DE < HL]
;   zero is zero/non-zero
?cmp:   mov     a,e
        sub     l
        mov     e,a
        mov     a,d
        sbb     h
        lxi     h,1             ;preset true
        jm      ?cmp1
        ora     e               ;resets carry
        ret
?cmp1:  ora     e
        stc
        ret
; unsigned compare of DE and HL
;   carry is sign of difference [set => DE < HL]
;   zero is zero/non-zero
?ucmp:  mov     a,d
        cmp     h
        jnz     ?ucmp1
        mov     a,e
        cmp     l
?ucmp1: lxi     h,1             ;preset true
        ret
; shift DE right arithmetically by HL, move to HL
?asr:   xchg
?asr1:  dcr     e
        rm
        mov     a,h
        ral
        mov     a,h
        rar
        mov     h,a
        mov     a,l
        rar
        mov     l,a
        jmp     ?asr1
; shift DE left arithmetically by HL, move to HL
?asl:   xchg
?asl1:  dcr     e
        rm
        dad     h
        jmp     ?asl1
; HL = DE - HL
?sub:   mov     a,e
        sub     l
        mov     l,a
        mov     a,d
        sbb     h
        mov     h,a
        ret
; HL = -HL
?neg:   call    ?com
        inx     h
        ret
; HL = ~HL
?com:   mov     a,h
        cma
        mov     h,a
        mov     a,l
        cma
        mov     l,a
        ret
; HL = !HL
?lneg:  mov     a,h
        ora     l
        jz      ?lneg1
        lxi     h,0
        ret
?lneg1: inx     h
        ret
; HL = !!HL
?bool:  call    ?lneg
        jmp     ?lneg
;
; HL = DE * HL [signed]
?mul:   mov     b,h
        mov     c,l
        lxi     h,0
?mul1:  mov     a,c
        rrc
        jnc     ?mul2
        dad     d
?mul2:  xra     a
        mov     a,b
        rar
        mov     b,a
        mov     a,c
        rar
        mov     c,a
        ora     b
        rz
        xra     a
        mov     a,e
        ral
        mov     e,a
        mov     a,d
        ral
        mov     d,a
        ora     e
        rz
        jmp     ?mul1
; HL = DE / HL, DE = DE % HL
?div:   mov     b,h
        mov     c,l
        mov     a,d
        xra     b
        push    psw
        mov     a,d
        ora     a
        cm      ?deneg
        mov     a,b
        ora     a
        cm      ?bcneg
        mvi     a,16
        push    psw
        xchg
        lxi     d,0
?div1:  dad     h
        call    ?rdel
        jz      ?div2
        call    ?cmpbd
        jm      ?div2
        mov     a,l
        ori     1
        mov     l,a
        mov     a,e
        sub     c
        mov     e,a
        mov     a,d
        sbb     b
        mov     d,a
?div2:  pop     psw
        dcr     a
        jz      ?div3
        push    psw
        jmp     ?div1
?div3:  pop     psw
        rp
        call    ?deneg
        xchg
        call    ?deneg
        xchg
        ret
; {DE = -DE}
?deneg: mov     a,d
        cma
        mov     d,a
        mov     a,e
        cma
        mov     e,a
        inx     d
        ret
; {BC = -BC}
?bcneg: mov     a,b
        cma
        mov     b,a
        mov     a,c
        cma
        mov     c,a
        inx     b
        ret
; {DE <r<r 1}
?rdel:  mov     a,e
        ral
        mov     e,a
        mov     a,d
        ral
        mov     d,a
        ora     e
        ret
; {BC : DE}
?cmpbd: mov     a,e
        sub     c
        mov     a,d
        sbb     b
        ret
; case jump
?case:  xchg                    ;switch value to DE
        pop     h               ;get table address
?case1: call    ?case4          ;get case value
        mov     a,e
        cmp     c               ;equal to switch value ?
        jnz     ?case2          ;no
        mov     a,d
        cmp     b               ;equal to switch value ?
        jnz     ?case2          ;no
        call    ?case4          ;get case label
        jz      ?case3          ;end of table, go to default
        push    b
        ret                     ;case jump
?case2: call    ?case4          ;get case label
        jnz     ?case1          ;next case
?case3: dcx     h
        dcx     h
        dcx     h
        mov     d,m
        dcx     h
        mov     e,m
        xchg
        pchl                    ;default jump
?case4: mov     c,m
        inx     h
        mov     b,m
        inx     h
        mov     a,c
        ora     b
        ret
;
;
;
Xstktop:        lxi     h,0     ;return current stack pointer (for sbrk)
        dad     sp
        ret
        cseg
etext:
        dseg
brkend: dw      edata           ;current "break"
edata:
;
;
;
        end
