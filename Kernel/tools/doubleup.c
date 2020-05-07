/*
 *	Turn 8bit wide input into 16bit wide for 16 bit read on 8bit CF 
 */

#include <stdio.h>

int main(int argc, char *argv[])
{
    int c;
    while((c = getchar()) != EOF) {
        putchar(c);
        putchar(c);
    }
    return 0;
}
