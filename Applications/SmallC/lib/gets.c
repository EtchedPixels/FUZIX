#include <stdio.h>
#define EOL     10
#define BKSP    8
#define CTRLU   0x15
gets(s) char *s; {
        char c, *ts;
        ts = s;
        while ((c = getchar()) != EOL && (c != EOF)) {
                if (c == BKSP) {
                        if (ts > s) {
                                --ts;
                                /* CPM already echoed */
                                putchar(' ');
                                putchar(BKSP);
                                }
                        }
                else if (c == CTRLU) {
                        ts = s;
                        putchar(EOL);
                        putchar('#');
                        }
                else (*ts++) = c;
                }
        if ((c == EOF) && (ts == s)) return NULL;
        (*ts) = NULL;
        return s;
}

