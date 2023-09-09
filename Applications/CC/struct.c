#include "compiler.h"

/*
 *	Struct declarations
 */

static void struct_add_field(struct symbol *sym, unsigned name, unsigned type)
{
    unsigned *p = sym->data.idx;
    unsigned n = 0;
    unsigned *t = p + 2;

    while(n < *p) {
        if (*t == name)
            error("duplicate field");
        t += 3;
        n++;
    }
    *t++ = name;
    *t++ = type;
    (*p)++;
    if (S_STORAGE(sym->infonext) == S_UNION) {
        /* For a union track the largest element size */
        unsigned s = type_sizeof(type);
        *t = 0;
        if (p[1] < s)
            p[1] = s;
    } else {
        /* For a struct allocate fields and track offset */
        *t = alloc_room(p + 1, type, S_STRUCT);
    }
}

static unsigned structdepth;

void struct_declaration(struct symbol *sym)
{
    unsigned name;
    unsigned t;
    unsigned tags[3 * NUM_STRUCT_FIELD + 2];	/* max 30 fields per struct for the moment */
    unsigned nfield = 0;
    unsigned err = 0;
    unsigned ut;

    if (sym->infonext & INITIALIZED)
        error("struct declared twice");
    sym->infonext |= INITIALIZED;

    /* Temporarily, as this can recurse */
    sym->data.idx = tags;

    *tags = 0;		/* No elements */
    tags[1] = 0;	/* Zero space */

    structdepth++;

    if (structdepth == 4)	/* Avoid stack overflows */
        fatal("struct too complex");

    require(T_LCURLY);
    while(token != T_RCURLY) {
        t = get_type();
        if (t == UNKNOWN) {
            badtype();
            junk();
            continue;
        }
        do {
            ut = type_name_parse(S_NONE, t, &name);
            if (nfield == NUM_STRUCT_FIELD) {
                if (err == 0)
                    error("too many struct/union fields");
                err = 1;
            } else {
                struct_add_field(sym, name, ut);
                nfield++;
            }
        } while (match(T_COMMA));
        require(T_SEMICOLON);
    }
    require(T_RCURLY);

    sym->data.idx = idx_copy(tags, 2 + 3 * *tags);

    structdepth--;
}
