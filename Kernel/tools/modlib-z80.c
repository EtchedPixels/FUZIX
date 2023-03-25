/*
 *	Generate the asm file for symbol exports
 *
 *	FIXME: we need to add some defence against mismatches
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

char buf[512];

#define NHASH	26

struct export {
    struct export *next;
    char data[];
};

struct export *names[NHASH];

unsigned hash(const char *p)
{
    if (!isalpha(*p))
        return 0;
    return toupper(*p) - 'A';
}

static struct export *name_insert(const char *p)
{
    unsigned n = hash(p);
    struct export *e = malloc(strlen(p) + 1 + sizeof(struct export));
    if (e == NULL) {
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }
    strcpy(e->data, p);
    e->next = names[n];
    names[n] = e;
    return e;
}

static unsigned exported(const char *p)
{
    unsigned n = hash(p);
    struct export *e = names[n];
    while(e) {
        if (strcmp(e->data, p) == 0)
            return 1;
        e = e->next;
    }
    return 0;
}

static void load_exports(const char *p, int opt)
{
    FILE *f = fopen(p, "r");
    if (f == NULL) {
        if (!opt) {
            perror(p);
            exit(1);
        }
        return;
    }
    while(fgets(buf, 511, f) != NULL) {
        if (*buf == '#' || *buf == ';')
            continue;
        buf[strlen(buf)-1] = 0;
        name_insert(buf);
    }
    fclose(f);
}

int main(int argc, char *argv[])
{
    FILE *m = fopen("fuzix.map", "r");

    char *p1, *p2;

    printf("\t.area EXPORT (ABS)\n\n\t.module exports\n\n");

    if (m == NULL) {
        perror("fuzix.map");
        exit(1);
    }

    load_exports("fuzix.export", 0);
    load_exports("platform/fuzix.export", 1);

    while(fgets(buf, 511, m) != NULL) {
        p1 = strtok(buf, " \t\n");
        if (p1) {
            p2 = strtok(NULL, " \t\n");
            if (p2 && exported(p2)) {
                printf("\t.globl %s\n", p2);
                printf("%s	.equ	0x%s\n",
                    p2, p1);
            }
        }
    }
    fclose(m);
    printf("\n\n");
}
