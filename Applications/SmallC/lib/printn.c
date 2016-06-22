#include <stdio.h>
/* print a number in any radish */
#define DIGARR "0123456789ABCDEF"
printn(number, radix, file)
int number, radix; FILE *file;{
        int i;
        char *digitreps;
        if (number < 0 & radix == 10){
                fputc('-', file);
                number = -number;
                }
        if ((i = number / radix) != 0)
                printn(i, radix, file);
        digitreps=DIGARR;
        fputc(digitreps[number % radix], file);
        }
