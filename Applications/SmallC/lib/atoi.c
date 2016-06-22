#include <stdio.h>
#define EOL 10
atoi(s) char s[];{
        int i,n,sign;
        for (i=0;
                (s[i] == ' ') | (s[i] == EOL) | (s[i] == '\t');
                ++i) ;
        sign = 1;
        switch(s[i]){
        case '-': sign = -1; /* and fall through */
        case '+': ++i;
                break;
        }
        for(n = 0;
                isdigit(s[i]);
                ++i)
                n = 10 * n + s[i] - '0';
        return (sign * n);

}

