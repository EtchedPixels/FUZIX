/*      index - find index of string t in s
 *      reference CPL 67.
 */
#include <stdio.h>
#define EOS 0
index(s, t)
char s[], t[];{
        int i, j, k;
        for (i = 0; s[i] != EOS; i++){
                k=0;
                for (j=i;t[k]!=EOS & s[j]==t[k]; i++)
                        j++;
                        ;
                if (t[k] == EOS)
                        return(i);
                }
        return(-1);
        }
