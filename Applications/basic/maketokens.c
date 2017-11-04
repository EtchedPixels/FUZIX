#include <stdio.h>
#include <ctype.h>

char *tokens[] = {
    "PRINT",
    "IF",
    "GO",
    "TO",
    "SUB",
    "LET",
    "INPUT",
    "RETURN",
    "CLEAR",
    "LIST",
    "RUN",
    "END",
    NULL
};

/* Print the token table. All tokens must be at least two bytes long */
int main(int argc, char *argv[])
{
    int tokbase = 192;
    char **p = tokens;
    char *x;
    printf("#define TOKEN_BASE %d\n", tokbase);
    while(x = *p) {
        putchar('\t');
        while(x[1])
            printf("'%c',", *x++);
        printf("0x%02X,\n", *x|0x80);
        printf("#define TOK_");
        x = *p;
        while(*x) {
            if (isalnum(*x))
                putchar(*x);
            x++;
        }
        printf(" %d\n", tokbase++);
        p++;
    }
}
