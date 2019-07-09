#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

static char linebuf[128];

int get_line(FILE *fp)
{
    char *p;
    if (fgets(linebuf, sizeof(linebuf) - 1, fp) == NULL)
        return 0;
    p = linebuf + strlen(linebuf) - 1;
    if (*p == '\n')
        *p = 0;
    return 1;
}

void output_symbol(char *name, uint16_t addr, char *type)
{
    printf("%s %04X %s\n", name, addr, type);
}

void parse_symbols(FILE *fp)
{
    char *name, *addrp, *typep;
    uint16_t addr;
    while(get_line(fp) && *linebuf) {
        name = strtok(linebuf, "\t ");
        addrp = strtok(NULL, "\t ");
        typep = strtok(NULL, "\t ");
        if (!name || !addrp || !typep) {
            fprintf(stderr, "Unable to parse symbol entry.\n");
            exit(1);
        }
        addr = strtoul(addrp, NULL, 16);
        output_symbol(name, addr, typep);
        name = strtok(NULL, "\t ");
        addrp = strtok(NULL, "\t ");
        typep = strtok(NULL, "\t ");
        if (!name || !addrp || !typep) {
            return;        }
        addr = strtoul(addrp, NULL, 16);
        output_symbol(name, addr, typep);
    }
}

void find_symbols(FILE *fp)
{
    while(get_line(fp)) {
        if (strncmp(linebuf, "Exports list by value:", 22) == 0) {
            get_line(fp);
            parse_symbols(fp);
            return;
        }
    }
    fprintf(stderr, "No valid symbol table found.\n");
    exit(1);
}

int main(int argc, char *argv[])
{
    find_symbols(stdin);
}

