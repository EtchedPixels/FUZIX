;       Run time start off for Small C.
        cseg
        sphl            ; save the stack pointer
        shld    ?stksav
        lhld    6       ; pick up core top
        lxi     d,-10   ; decrease by 10 for safety
        dad     d
        sphl            ; set stack pointer
        call    stdioinit       ; initialize stdio
        call    Xarglist
        lhld    Xargc
        push    h
        lxi     h,Xargv
        push    h
        call    main    ; call main program
        pop     d
        pop     d
        lhld    ?stksav ; restore stack pointer
        ret             ; go back to CCP
        dseg
?stksav ds      2
        extrn   stdioinit
        extrn   Xarglist
        extrn   Xargc
        extrn   Xargv
        extrn   main
        end
