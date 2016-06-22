#include <stdio.h>

fputs(str, fp) FILE *fp; char *str; {
        while(*str) fputc(*str++, fp);

}

