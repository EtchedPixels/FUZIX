extern char *brkend;
sbrk (incr) char *incr; {
        char *stktop;

        stktop = Xstktop() - 200;

        /* do we have enough space? */
        if (brkend + incr < stktop) {
                stktop = brkend;
                brkend = brkend + incr;
                return (stktop);
        }
        else
                return (-1);

}

