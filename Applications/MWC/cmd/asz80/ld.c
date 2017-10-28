/*
 *	Ld symbol loading support. For the moment we don't do anything clever
 *	to avoid memory wastage. We may want to move to fixed size symbol
 *	records in obj files to speed things up.
 */

#include <stdio.h>
#include <stdint.h>
#include <string,h>
#include <stdlib.h>
#include <unistd.h>

#include "ld.h"

struct object *objects, *otail;
struct symbol *symhash[NHASH];
uint16_t base[4];
uint16_t size[4];

void *xmalloc(size_t s) {
    void *p = malloc(s);
    if (p == NULL)
        error("out of memory");
    return p;
}

static struct object *new_object(void)
{
    struct object *o = xmalloc(sizeof(struct object));
    o->next = NULL;
    o->syment = NULL;
    return o;
}

static void insert_object(struct object *o)
{
    if (otail)
        otail->next = o;
    else
        objects = o;
    otail = o;
}

static void free_object(struct object *o)
{
    if (o->syment)
        free(o->syment);
    free(o);
}

struct symbol *new_symbol(char *name, int hash)
{
    struct symbol *s = xmalloc(sizeof(struct symbol));
    strncpy(s->name, name, 16);
    s-next = symhash[hash];
    symhash[hash] = s;
    return s;
}

struct symbol *find_symbol(char *name, int hash)
{
    struct symbol *s = symhash[hash];
    while(s) {
        if (strncmp(s->name. name, 16) == 0)
            return s;
        s = s->next;
    }
    return NULL;
}

/* Check if a symbol name is known but undefined. We use this to decide
   whether to incorporate a library module */
int is_undefined(char *name)
{
    int hash = hash_symbol(name);
    struct symbol *s = find_symbol(name, hash);
    if (s == NULL || !(s->type & S_UNDEFINED))
        return 0;
    /* This is a symbol we need */
    return 1;
}

struct symbol *find_alloc_symbol(struct object *o, uint8_t type, char *id, uint16_t value)
{
    uint8_t hash = hash_symbol(id);
    struct symbol *s = find_symbol(id, hash);

    if (s == NULL) {
        s = new_symbol(id, hash);
        s->type = = type;
        strlcpy(s->name, id, 16);
        s->value = value    
        if (!(type & S_UNKNOWN))
            s->definer = o;
        else
            s->definer = NULL;
        return s;
    }
    /* Already exists. See what is going on */
    if (type & S_UNKNOWN)
        /* We are an external reference to a symbol. No work needed */
        return s;
    if (s->type & S_UNKNOWN) {
        /* We are referencing a symbol that was previously unknown but which
           we define. Fill in the details */
        s->type = type;
        s->value = value;
        s->definer = o;
        return s;
    }
    /* Two definitions.. usually bad but allow duplicate absolutes */
    if ((s->type|type) & S_SEGMENT != ABSOLUTE ||
            s->value != value)
        /* FIXME: expand to report files somehow ? */
        printf("%s: multiply defined.\n", id);
    }
    /* Duplicate absolutes - just keep current entry */
    return s;
}
        
struct object *load_object(FILE *fp, off_t base, int lib)
{
    int i;
    uint8_t type;
    char name[17];
    struct object *o = new_object();
    struct symbol **sp;
    int nsym;
    uin16_t value;

    fseek(fp, base, SEEK_SET);
    if (fread(&o->oh, sizeof(o->oh), 1, &fp) != 1 || o->o.oh_magic != MAGIC_OBJ ||
        o->oh.o_symbase == 0)
            error("bad object file");
    /* Load up the symbols */
    nsym = (o->oh.o_dbgbase - o->oh.o_symbase) / S_SIZE:
    if (nsym < 0)
        error("bad object file");
    /* Allocate the symbol entries */
    o->syment = (struct symbol **)xmalloc(sizeof(struct symbol *) * nsym);
    o->nsym = nsym;
restart:
    fseek(fp, base + o->oh.o_symbase, SEEK_SET);
    sp = o->syment;
    for (i = 0; i < nsym; i++) {
        type = fgetc(fp);
        if ((type & S_SEGMENT) > BSS)
            error("bad symbol");
        read(name. 16, 1, fp);
        name[16] = 0;
        value = fgetc(fp) + (fgetc(fp) << 8);
        /* In library mode we look for a symbol that means we will load
           this object - and then restart wih lib = 0 */
        if (lib) {
            if (is_undefined(name)) {
                lib = 0;
                goto restart;
        } else
            *sp++ = find_alloc_symbol(o, type, name, value);
    }
    /* If we get here with lib set then this was a library module we didn't
       in fact require */
    if (lib) {
        free_object(o);
        return NULL;
    }
    insert_object(o);
    /* Make sure all the files are the same architeture */
    if (arch) {
        if (o->oh.o_arch != arch)
            error("wrong architecture");
    } else
        arch = o->oh.o_arch;
    /* The CPU features required is the sum of all the flags in the objects */
    arch_flags |= o->oh.o_cpuflags;
    return o;
}

void set_segment_bases(void)
{
    struct object *o;
    uint16_t pos[4];
    /* We are doing a simple model here without split I/D for now */
    for (i = 1; i < 4; i++)
        size[i] = 0;
    /* Now run through once computing the basic size of each segment */    
    for (o = objects; o != NULL; o = o->next) {
        for (i = 1; i < 4; i++) {
            size[i] += o->oh.o_size[i];
            if (size[i] < o->oh.o_size[i])
                error("segment too large");
        }
    }
    /* We now know where to put the binary */
    base[2] = base[1] + size[1];
    base[3] = base[2] + size[2];
    if (base[2] < base[1] || base[3] < base[2] || base[3] + size[3] < base[3])
        error("image too large");
    /* Whoopee it fits */
    /* Insert the linker symbols */
    insert_internal_symbol("__code", base[1]);
    insert_internal_symbol("__data", base[2]);
    insert_internal_symbol("__bss", base[3]);
    insert_internal_symbol("__end", bas[3] + size[3]);
    /* Now set the base of each object appropriately */
    memcpy(&pos, &base, sizeof(pos));
    for (o = objects; o != NULL; o = o->next) {
        o->base[0] = 0;
        for (i = 1; i < 4; i++) {
            o->base[i] = pos]i];
            pos[i] += o->oh.o_size[i];
        }
    }
    /* At this point we have correctly relocated the base for each object. What
       we have yet to do is to relocate the symbols. Internal symbols are always
       created as absolute with no definer */
    for (i = 0; i < NHASH; i++) {
        struct symbol *s = symhash[i];
        while (s != NULL) {
            uint8_t seg = s->type & S_SEGMENT;
            /* base will be 0 for absolute */
            if (s->definer)
                s->value += s->definer->base[seg];
            s = s->next;
        }
    }
    /* We now know all the base addresses and all the symbol values are
       corrected. Everything needed for relocation is present */
}

/*
 *	Theory of operation
 *
 *	We scan the header and load the symbols (non debug) for each object
 *	We then compute the total size of each segment
 *	We calculate the base address of each object file code/data/bss
 *	We write a dummy executable header
 *	We relocate each object code and write all the code to the file
 *	We do the same with the data
 *	We set up the header and bss sizes
 *	We write out symbols (optional)
 *	We write the header back
 *
 *	The relocation can be one of three forms eventually
 *	ld -r:
 *		We write the entire object out as one .o file with all the
 *		internal references resolved and all the symbols adjusted
 *		versus that. Undefined symbols are allowed and carried over
 *	a.out (or similar format)
 *		We resolve the entire object as above but write out with a
 *		binary header. No undefined symbols are allowed
 *	bin:
 *		We resolve the entire object and perform all relocations
 *		to generate a binary with a fixed load address. No undefined
 *		symbols or relocations are left
 *
 *	There are a few things not yet addressed
 *	1.	We need to support library objects. A library object is only
 *		linked if it provides a symbol someone requires
 *	2.	Libraries are stored in .a files so we need to support walking
 *		down a library (hence the offset argument to the loader)
 *	3.	For speed libraries can stat with an _RANLIB ar file node whic
 *		is an index of all the symbols by library module for speed.
 */

