#include <stdio.h>
#define EOS 0
itoa(n,s) char s[];int n;{
        int i,sign;
        if((sign = n) < 0) n = -n;
        i = 0;
        do {
                s[i++] = n % 10 + '0';
         }while ((n = n/10) > 0);
        if (sign < 0) s[i++] = '-';
        s[i] = EOS;
        reverse(s);
}

