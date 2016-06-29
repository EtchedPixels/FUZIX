|       prabs.  converts both args to unsigned, and
|       remembers result sign as sign a eor sign b
|       used only by divide support
|       result d contains right, sign is non-zero
|       if result (from divide) should be negative.
|
|
.globl __prabs
|       left=8.
|       right=4.
|       sign=3.
__prabs:clr     3,s
        ldd     8,s
        bge     tryr
        nega
        negb
        sbca    #0
        std     8,s
        inc     3,s
tryr:   ldd     4,s
        bge     done
        nega
        negb
        sbca    #0
        dec     3,s
        std     4,s
done:   rts
