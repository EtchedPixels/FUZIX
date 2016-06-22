/*
#include        <stdio.h>
*/
#define NULL 0
#define FILE char

fgets(s, n, iop)
int n;
char *s;
register FILE *iop;
{
        register c;
        register char *cs;

        cs = s;
        while (--n>0 && (c = fgetc(iop))>=0) {
                *cs++ = c;
                if (c=='\n')
                        break;
        }
        if (c<0 && cs==s)
                return(NULL);
        *cs++ = '\0';
        return(s);

}

