#include "compiler.h"

/*
 *	Write a single initialization element to the stream. For auto variables
 *	we generate the assignment tree, for static or globals we generate a
 *	stream of data with types for the backend.
 */
static void initializer_single(struct symbol *sym, unsigned type, unsigned storage)
{
    struct node *n = expression_tree(0);
    n = typeconv(n, type, 1);
    if (storage == S_AUTO || storage == S_REGISTER) {
        n = tree(T_EQ, make_symbol(sym), n);
        write_tree(n);
    } else {
        put_typed_data(n);
        free_tree(n);
    }
}

/* C99 permits trailing comma and ellipsis */
/* Strictly {} is not permitted - there must be at least one value */

static unsigned initializer_string(unsigned n)
{
        /* This one is weird because the string is not literal */
        if (n)
            n = copy_string(0, n, 1, 0);
        else
            n = copy_string(0, TARGET_MAX_PTR, 0, 0);
        return n;
}

/*
 *	Array bottom level initializer: repeated runs of the same type
 *
 *	TODO: In theory we could have a platform that needs padding
 *	and we don't deal with that aspect of alignment yet
 */
static unsigned initializer_group(struct symbol *sym, unsigned type, unsigned n, unsigned storage)
{
    unsigned sized = n;
    unsigned string = 0;
    unsigned count = 0;
    /* C has a funky special case rule that you can write
       char x[16] = "foo"; which creates a copy of the string in that
       array not a literal reference. It's also got a second funky special case
       rule that you can write { "string" }. */

    if ((type_canonical(type) & ~UNSIGNED) == CCHAR)
        string = 1;

    if (token == T_STRING) {
        if (!string)
            typemismatch();
        return initializer_string(n);
    }
    require(T_LCURLY);
    if (!sized)
        n = TARGET_MAX_PTR;
    while(n && token != T_RCURLY) {
        /* Deal with the second string special case, gotta love C some days */
        if (token == T_STRING && string) {
            n = initializer_string(sized);
            require(T_RCURLY);
            return n;
        }
        string = 0;	/* Only valid first */
        if (token == T_ELLIPSIS)
            break;
        n--;
        initializers(sym, type, storage);
        count++;
        if (!match(T_COMMA))
            break;
    }
    if (n && sized) {
        unsigned s = type_sizeof(type) * n;
        put_padding_data(s);
    }
    /* Catches any excess elements */
    require(T_RCURLY);
    return count;
}

/*
 *	Struct and union initializer
 *
 *	This is similar to an array but each element has its own expected
 *	type, and some elements may themselves be structures or arrays. It's
 *	mostly recursion.
 *
 *	Remaining space in the object is padded.
 *
 *	We don't deal with auto here as with arrays because we don't support
 *	the C extensions of auto array and struct with initializers.
 */
static void initializer_struct(struct symbol *psym, unsigned type, unsigned storage)
{
    struct symbol *sym = symbol_ref(type);
    unsigned *p = sym->data.idx;
    unsigned n = *p;
    unsigned s = p[1];	/* Size of object (needed for union) */
    unsigned pos = 0;

    p += 2;
    /* We only initialize the first object */
    if (S_STORAGE(sym->infonext) == S_UNION)
        n = 1;
    require(T_LCURLY);
    while(n-- && token != T_RCURLY) {
        /* Name, type, offset tuples */
        type = p[1];

        /* Align */
        if (pos != p[2]) {
            put_padding_data(p[2] - pos);
            pos = p[2];
        }
        /* Write out field */
        initializers(psym, type, storage);
        pos += type_sizeof(type);

        /* Next field */
        p += 3;

        if (!match(T_COMMA))
            break;
    }
    if (n == -1 && token != T_RCURLY)
        error("too many initializers");
    require(T_RCURLY);
    /* For a union zerofill the slack if other elements are bigger */
    /* For a struct fill from the offset of the next field to the size of
       the base object */
    if (pos != s)
        put_padding_data(s - pos);	/* Fill remaining space */
}

/*
 *	Array initializer.
 *
 *	We recursively call down through the layers until we hit the bottom
 *	layer of the array which should be a series of values in the type
 *	of the array. The base value may be a structure.
 */
static void initializer_array(struct symbol *sym, unsigned type, unsigned depth, unsigned storage)
{
    unsigned n = array_dimension(type, depth);
    unsigned count = 0;

    if (depth < array_num_dimensions(type)) {
        type = type_deref(type);
        require(T_LCURLY);
        if (n == 0)
            n = TARGET_MAX_PTR;
        while(n--) {
            initializer_array(sym, type, depth + 1, storage);
            count++;
            /* Trailing comma is allowed so eat it before checking n */
            if (match(T_COMMA) && n)
                continue;
            break;
        }
        if (array_dimension(type, 1) == 0)
            sym->type = array_with_size(type, count);
        /* Pad the remaining pieces */
        while(n--)
            put_padding_data(type_sizeof(type));
        require(T_RCURLY);
    } else {
        n = initializer_group(sym, type_deref(type), n, storage);
        if (array_dimension(type, 1) == 0)
            sym->type = array_with_size(type, n);
    }
}

/*
 *	Initialize an object.
 */
void initializers(struct symbol *sym, unsigned type, unsigned storage)
{
    if (PTR(type) && !IS_ARRAY(type)) {
        initializer_single(sym, type, storage);
        return;
    }
    if (IS_ARITH(type)) {
        initializer_single(sym, type, storage);
        return;
    }
    /* No complex stack initializers, for now at least */
    if (storage == S_AUTO || storage == S_REGISTER) {
        error("not a valid auto initializer");
        return;
    }
    if (storage == S_EXTERN) {
        error("cannot initialize external");
        return;
    }
    if (IS_FUNCTION(type))
        error("init function");	/* Shouldn't get here, we don't use "=" for
                                   function forms even if it would be more
                                   logical than the C syntax */
    else if (IS_ARRAY(type))
        initializer_array(sym, type, 1, storage);
    else if (IS_STRUCT(type))
        initializer_struct(sym, type, storage);
    else
        error("cannot initialize this type");
}
