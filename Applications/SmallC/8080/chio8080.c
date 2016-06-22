#define EOL 10
getchar() {
        return (bdos(1,1));

}

putchar (c) char c; {
        if (c == EOL)   bdos(2,13);
        bdos(2,c);
        return c;
}

