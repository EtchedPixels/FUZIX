/*
 *	For now this only works on object files. It's mostly here so I can
 *	check the assembler output looks valid.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <ctype.h>

#include "obj.h"

static char *arg0;
static int err;
static int show_debug = 1;
static int show_name;
static int show_undef;

static char segname[] = "ATDB???????????U";

static int do_nm(FILE *fp, const char *name)
{
    static struct objhdr oh;
    off_t base;
    uint8_t type;
    int c;
    uint16_t addr;
    char symname[NAMELEN + 1];

    if (show_name)
        printf("%s:\n", name);
    if (fread(&oh, sizeof(oh), 1, fp) != 1 ||
        oh.o_magic != MAGIC_OBJ) {
            fprintf(stderr, "%s: %s: not a valid object file.\n", arg0, name);
            return 1;
    }
    base = oh.o_symbase;
    if (base == 0) {
        fprintf(stderr, "%s: %s: no symbols.\n", arg0, name);
        return 0;
    }
    if (fseek(fp, base, SEEK_SET)) {
        fprintf(stderr, "%s: %s: truncated file ?\n", arg0, name);
        return 1;
    }
    while (1) {
        if (base >= oh.o_dbgbase && show_debug == 0)
            break;
        c = fgetc(fp);
        if (c == EOF)
            return 0;
        type = (uint8_t)c;
        base++;
        fread(symname, NAMELEN, 1, fp);
        base += NAMELEN;
        symname[NAMELEN] = 0;
        /* Address if defined */
        addr = fgetc(fp);
        addr |= fgetc(fp) << 8;
        base += 2;
        c = segname[type & S_SEGMENT];
        if (!(type & S_UNKNOWN)) {
            /* Showing undefined only */
            if (show_undef)
                continue;
        } else {
            if (c != 'U')
                c = tolower(c);
            addr = 0;
        }
        printf("%04X %c %s\n", addr, c, symname);
    }
    return 0;
}            

int main(int argc, char *argv[])
{
    int opt;
    arg0 = argv[0];

    while ((opt = getopt(argc,argv,"oAug")) != -1) {
        switch(opt) {
            case 'o':
            case 'A':
                show_name = 1;
                break;
            case 'u':
                show_undef = 1;
                break;
            case 'g':
                show_debug = 0;
                break;
            default:
                fprintf(stderr, "%s: name ...\n", argv[0]);
                exit(1);
        }
    }

    /* Show names if multiple arguments */
    if (optind - argc > 1)
        show_name = 1;

    if (optind >= argc)
        do_nm(stdin, "-");
    else while (optind < argc) {
        FILE *fp = fopen(argv[optind], "r");
        if (fp == NULL) {
            perror(argv[optind]);
            err |= 1;
        } else {
            err |= do_nm(fp, argv[optind]);
            fclose(fp);
        }
        optind++;
    }
    exit(err);
}

