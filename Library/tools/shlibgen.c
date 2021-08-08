#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

/*
 *	Generate the pieces to build a shared library header and pull it together
 */

#define MAX_CALLS 512

char *call_name[MAX_CALLS];
unsigned int num_calls = 0;
unsigned int lib_major = 0;
unsigned int lib_minor = 0;

/* Using the CPU specific instruction template generate a branch for each call in the call list */
void generate_jumps(char *template)
{
    unsigned int i;
    printf(template, "__libopen");
    printf(template, "__libclose");
    for (i = 0; i < num_calls; i++) {
        printf(template, call_name[i]);
    }
}

void do_substitutions(const char *p)
{
    const char *bp = p;
    while(*p) {
        if (*p == '%') {
            p++;
            if (*p == '%') {
                putchar('%');
                p++;
                continue;
            }
            if (*p == 'M') {
                printf("%d", lib_major);
                p++;
                continue;
            }
            if (*p == 'm') {
                printf("%d", lib_minor);
                p++;
                continue;
            }
            if (*p == 'n') {
                printf("%d", num_calls);
                p++;
                continue;
            }
            fprintf(stderr, "unknown substitution in %s", bp);
            exit(1);
        } else {
            putchar(*p);
            p++;
        }
    }
}

void generate(void)
{
    char buf[512];
    while(fgets(buf, 512, stdin) != NULL) {
        if (memcmp(buf, "%jump", 5) == 0 &&  isspace(buf[5])) {
            generate_jumps(buf + 6);
        } else
            do_substitutions(buf);
    }
}

void add_function(const char *n)
{
    if (num_calls == MAX_CALLS) {
        fprintf(stderr, "Too many function names at '%s'.\n", n);
        exit(1);
    }
    call_name[num_calls++] = strdup(n);
    if (n == NULL) {
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }
}

void load_functions(const char *p)
{
    FILE *fp = fopen(p, "r");
    char buf[512];
    unsigned int vh, vl;
    if (fp == NULL) {
        perror(p);
        exit(1);
    }
    while(fgets(buf, 511, fp) != NULL) {
        if (*buf == '#')
            continue;
        if (sscanf(buf, "%%version %u.%u\n", &vh, &vl) == 2) {
            lib_major = vh;
            lib_minor = vl;
            continue;
        } else if (*buf == '%') {
            fprintf(stderr, "%s: unknown directive - %s", p, buf);
            exit(1);
        }
        buf[strlen(buf) - 1] = 0;
        add_function(buf);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "%s func_map <template >asmfile.\n", argv[0]);
        exit(1);
    }
    load_functions(argv[1]);    
    generate();
    return 0;
}
