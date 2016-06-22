/*
 * Concatenate s2 on the end of s1.  S1's space must be large enough.
 * Return s1.
 */

strcat(s1, s2)
char *s1, *s2;
{
        char *os1;

        os1 = s1;
        while (*s1++)
                ;
        *--s1;
        while (*s1++ = *s2++)
                ;
        return(os1);

}

