bdos1(c, de) int c, de; {
        /* returns only single byte (top half is 0) */
        return (255 & bdos(c, de));
}

