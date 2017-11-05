#include <stdio.h>
#include <string.h>
#include <ctype.h>

char *tokens[] = {
    "PRINT",
    "INPUT",
    "IF",
    "THEN",
    "ELSE",
    "FOR",
    "TO",
    "NEXT",
    "STEP",
    "GO",
    "SUB",
    "RETURN",
    "LET",
    "REM",
    "CLEAR",
    "LIST",
    "RUN",
    "STOP",
    "END",
    "DIM",
    "RANDOMIZE",
    "READ",
    "DATA",
    "RESTORE",
    "OPTION",
    "BASE",
    "TAB",
    "AT",
    "CLS",
    "SAVE",
    "LOAD",
    "DEF",
    "FN",
    "ON",
    "AND",
    "OR",
    "NOT",
    "<=|LE",
    ">=|GE",
    "<>|NE",
    "**|ORD",
    "ABS",
    "ATN",
    "CODE",
    "COS",
    "EXP",
    "INT",
    "LOG",
    "RND",
    "SGN",
    "SIN",
    "SQR",
    "TAN",
    
    "LEN",
    "VAL",
    "MOD",
    "LEFT$",
    "RIGHT$",
    "MID$",
    "CHR$",
    "INKEY$",
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
        char *y = strchr(x, '|');
        if (y)
            y++;
        else
            y = x;
        putchar('\t');
        while(x[1] && x[1] != '|')
            printf("'%c',", *x++);
        printf("0x%02X,\n", *x|0x80);
        printf("#define TOK_");
        while(*y) {
            if (isalnum(*y))
                putchar(*y);
            y++;
        }
        printf(" %d\n", tokbase++);
        p++;
    }
}
