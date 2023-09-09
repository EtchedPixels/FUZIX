#include "compiler.h"

struct constant {
    unsigned name;
    unsigned value;
};

/*
 *	Enum is really about tables of constant values to substitute for
 *	integer types.
 *	TODO: track the type of the constant ?
 */

static struct constant constant_tab[NUM_CONSTANT];
static struct constant *ctnext = constant_tab;

unsigned find_constant(unsigned name, unsigned *v)
{
    struct constant *ct = constant_tab;
    while(ct < ctnext) {
        if (ct->name == name) {
            *v = ct->value;
            return 1;
        }
        ct++;
    }
    return 0;
}

static void insert_constant(unsigned n, unsigned nv)
{
    unsigned v;
    if (find_constant(n, &v)) {
        if (v != nv)
            error("duplicate constant");
        return;
    }
    if (ctnext == &constant_tab[NUM_CONSTANT])
        fatal("too many constants");
    ctnext->name = n;
    ctnext->value = nv;
    ctnext++;
}
    
/*
 *	Enumerated types. These turn out to be quite simple to parse but
 *	do cause a certain amount of nuisance when type handling elsewhere
 *	because an enum is a complex type but also a simple arithmetic thing.
 */

static unsigned enum_set_base(void)
{
    require(T_EQ);    
    return const_int_expression();
}

/* An enum */
unsigned enum_body(void)
{
    unsigned enum_base = 0;
    unsigned type = CINT;
    unsigned ename;
    unsigned name;
    struct symbol *sym = NULL;

    /* Optional */
    ename = symname();

    if (ename)
        sym = find_symbol_by_class(ename, S_ENUM);
        
    /* TODO: we should if possible pick a range to cover the type. We take
       a rather simplistc approach */
    if (!match(T_LCURLY)) {
        if (sym == NULL) {
            error("unknown enum");
            return CINT;
        }
        return sym->type;
    }
    if (sym) {
        error("duplicate enum");
        junk();
        return CINT;
    }
    while (token != T_RCURLY) {
        name = symname();
        if (name == 0) {
            error("bad enum");
            junk();
            return CINT;
        }
        if (token == T_EQ)
            enum_base = enum_set_base();
        if (enum_base > TARGET_MAX_INT)
            type = UINT;
        /* Enum constants have global scope and are not tied to the enum type. It's
           also possible to refer to a previous one in the next so we must assign as
           we go (ie enum foo { A, B = A + 4 }; is legal */
        insert_constant(name, enum_base++);            
        if (match(T_COMMA))
            continue;
    }
    if (ename)
        update_symbol_by_name(ename, S_ENUM, type);
    /* An enum takes the type needed to hold the things in it. Thus we can
       shortcut all the type stuff and report the underlying type */
    require(T_RCURLY);
    return type;
}

