#include <stdio.h>
#define EOL 10
puts(str) char *str;{
        while (*str) putchar(*str++);
        putchar(EOL);
        }
