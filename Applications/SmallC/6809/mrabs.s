|       mrabs.  converts both args to unsigned, and
|       remembers result sign as the sign of the left
|       argument.  (for signed modulo)
|       result d contains right, sign is non-zero
|       if result (from mod) should be negative.
|
|
.globl __mrabs
;       left=8.
;       right=4.
;       sign=3.
__mrabs:clr     3,s
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
        std     4,s
done:   rts
