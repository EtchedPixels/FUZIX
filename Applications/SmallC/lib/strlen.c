#include <stdio.h>
/* return length of string, reference CPL p 36 */
strlen(s) char *s;{
        int i;
        i = 0;
        while (*s++) i++;
        return (i);
        }
