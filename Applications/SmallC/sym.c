/*
 * File sym.c: 2.1 (83/03/20,16:02:19)
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"

/**
 * declare a static variable
 * @param type
 * @param storage
 * @param mtag tag of struct whose members are being declared, or zero
 * @param otag tag of struct object being declared. only matters if mtag is non-zero
 * @param is_struct struct or union or no meaning
 * @return 1 if a function was parsed
 */
int declare_global(int type, int storage, TAG_SYMBOL *mtag, int otag, int is_struct) {
    int     dim, identity;
    char    sname[NAMESIZE];

    FOREVER {
        FOREVER {
            if (endst ())
                return 0;
            dim = 1;
            if (match ("*")) {
                identity = POINTER;
            } else {
                identity = VARIABLE;
            }
            if (!symname (sname))
                illname ();
            if (match ("(")) {
                /* FIXME: We need to deal with pointer types properly here */
                if (identity == POINTER)
                    type = CINT;
                newfunc_typed(storage, sname, type);
                /* Can't int foo(x){blah),a=4; */
                return 1;
            }
            /* FIXME: we need to deal with extern properly here */
            if (find_global (sname) > -1)
                multidef (sname);
            if (identity == VARIABLE)
                notvoid(type);
            if (match ("[")) {
                dim = needsub ();
                //if (dim || storage == EXTERN) {
                    identity = ARRAY;
                //} else {
                //    identity = POINTER;
                //}
            }
            // add symbol
            if (mtag == 0) { // real variable, not a struct/union member
                identity = initials(sname, type, identity, dim, otag);
                add_global (sname, identity, type, (dim == 0 ? -1 : dim), storage);
                if (type == STRUCT) {
                    symbol_table[current_symbol_table_idx].tagidx = otag;
                }
                break;
            } else if (is_struct) {
                // structure member, mtag->size is offset
                add_member(sname, identity, type, mtag->size, storage);
                // store (correctly scaled) size of member in tag table entry
                if (identity == POINTER)
                    type = CINT;
                scale_const(type, otag, &dim);
                mtag->size += dim;
            }
            else {
                // union member, offset is always zero
                add_member(sname, identity, type, 0, storage);
                // store maximum member size in tag table entry
                if (identity == POINTER)
                    type = CINT;
                scale_const(type, otag, &dim);
                if (mtag->size < dim)
                    mtag->size = dim;
            }
        }
        if (!match (","))
            return 0;
    }
}

/**
 * initialize global objects
 * @param symbol_name
 * @param type char or integer or struct
 * @param identity
 * @param dim
 * @return 1 if variable is initialized
 */
int initials(char *symbol_name, int type, int identity, int dim, int otag) {
    int dim_unknown = 0;
    int n;

    if(dim == 0) { // allow for xx[] = {..}; declaration
        dim_unknown = 1;
    }
    if (!(type & CCHAR) && !(type & CINT) && !(type == STRUCT)) {
        error("unsupported storage size");
    }
    data_segment_gdata();
    glabel(symbol_name);
    if(match("=")) {
        // an array or struct
        if(match("{")) {
            // aggregate initialiser
            if ((identity == POINTER || identity == VARIABLE) && type == STRUCT) {
                // aggregate is structure or pointer to structure
                dim = 0;
                struct_init(&tag_table[otag], symbol_name);
            }
            else {
                while((dim > 0) || (dim_unknown)) {
                    if (identity == ARRAY && type == STRUCT) {
                        // array of struct
                        needbrack("{");
                        struct_init(&tag_table[otag], symbol_name);
                        --dim;
                        needbrack("}");
                    }
                    else {
                        if (init(symbol_name, type, identity, &dim, 0)) {
                            dim_unknown++;
                        }
                    }
                    if(match(",") == 0) {
                        break;
                    }
                }
                if(--dim_unknown == 0)
                    identity = POINTER;
                else {
                    /* Pad any missing objects */
                    n = dim;
                    gen_def_storage();
                    if (identity != ARRAY && type != STRUCT) {
                        if (!(type & CCHAR))
                           n *= 2;
                    } else
                        n = tag_table[otag].size;

                    output_number(n);
                    newline();
                }
            }
            needbrack("}");
        // single constant
        } else {
            init(symbol_name, type, identity, &dim, 0);
        }
    }
    code_segment_gtext();
    return identity;
}

/**
 * initialise structure
 * @param tag
 */
void struct_init(TAG_SYMBOL *tag, char *symbol_name) {
	int dim ;
	int member_idx;
	int size = tag->size;

	member_idx = tag->member_idx;
	while (member_idx < tag->member_idx + tag->number_of_members) {
		size -= init(symbol_name, member_table[tag->member_idx + member_idx].type,
                        member_table[tag->member_idx + member_idx].identity, &dim, tag);
		++member_idx;
		/* FIXME:  not an error - zero rest */
		if ((match(",") == 0) && (member_idx != (tag->member_idx + tag->number_of_members))) {
		    gen_def_storage();
		    output_number(size);
		    newline();
		    break;
		}
	}
}

/**
 * evaluate one initializer, add data to table
 * @param symbol_name
 * @param type
 * @param identity
 * @param dim
 * @param tag
 * @return
 *	returns size of initializer, or 0 for none (a null string is size 1)
 *
 */
int init(char *symbol_name, int type, int identity, int *dim, TAG_SYMBOL *tag) {
    int value, n;

    /* A pointer is initialized as a word holding the address of the struct
       or string etc that directly follows */
    if(identity == POINTER) {
        int x = getlabel();
        gen_def_word();
        print_label(x);
        newline();
        print_label(x);
        output_label_terminator();
        newline();
    }
    /* FIXME: may need to distinguish const string v data in future */
    if(quoted_string(&n, NULL)) {
        if((identity == VARIABLE) || !(type & CCHAR))
            error("found string: must assign to char pointer or array");
        *dim = *dim - n; /* ??? FIXME arrays of char only */
        return n;
    }

    if (type & CCHAR)
        gen_def_byte();
    else
        gen_def_word();
    if (!number(&value) && !quoted_char(&value))
        return 0;
    *dim = *dim - 1;
    output_number(value);
    newline();
    return (type & CCHAR) ? 1 : 2;
}

/**
 * declare local variables
 * works just like "declglb", but modifies machine stack and adds
 * symbol table entry with appropriate stack offset to find it again
 * @param typ
 * @param stclass
 * @param otag index of tag in tag_table
 */
void declare_local(int typ, int stclass, int otag) {
    int     k, j;
    char    sname[NAMESIZE];

    FOREVER {
        FOREVER {
            if (endst())
                return;
            if (match("*"))
                j = POINTER;
            else
                j = VARIABLE;
            if (!symname(sname))
                illname();
            if (-1 != find_locale(sname))
                multidef (sname);
            if (match("[")) {
                k = needsub();
                if (k) {
                    j = ARRAY;
                    if (typ & CINT) {
                        k = k * INTSIZE;
                    } else if (typ == STRUCT) {
                        k = k * tag_table[otag].size;
                    }
                } else {
                    j = POINTER;
                    k = INTSIZE;
                }
            } else {
                if (j == POINTER) {
                    k = INTSIZE;
                } else {
                    switch (typ) {
                        case CCHAR:
                        case UCHAR:
                            k = 1;
                            break;
                        case STRUCT:
                            k = tag_table[otag].size;
                            break;
                        default:
                            k = INTSIZE;
                    }
                }
            }
            if (stclass == LSTATIC) {
                add_local(sname, j, typ, k, LSTATIC);
                break;
            }
            if (stclass == REGISTER) {
                int r = gen_register(j, k, typ);
                if (r != -1) {
                    add_local(sname, j, typ, r, REGISTER);
                    break;
                }
            }
            if (match("=")) {
                gen_modify_stack(stkp);
                expression(0);
                gen_push(0);
            } else
                stkp = gen_defer_modify_stack(stkp - k);
            add_local(sname, j, typ, stkp, AUTO);
            break;
        }
        if (!match(","))
            return;
    }
}

/**
 * get required array size. [xx]
 * @return array size
 */
int needsub(void) {
    int num[1];

    if (match ("]"))
        return (0);
    if (!number (num)) {
        error ("must be constant");
        num[0] = 1;
    }
    if (num[0] < 0) {
        error ("negative size illegal");
        num[0] = (-num[0]);
    }
    needbrack ("]");
    return (num[0]);
}

/**
 * search global table for given symbol name
 * @param sname
 * @return table index
 */
int find_global (char *sname) {
    int idx;

    idx = 0;
    while (idx < global_table_index) {
        if (astreq (sname, symbol_table[idx].name, NAMEMAX))
            return (idx);
        idx++;
    }
    return (-1);
}

/**
 * search local table for given symbol name
 * @param sname
 * @return table index
 */
int find_locale (char *sname) {
    int idx;

    idx = local_table_index;
    while (idx >= NUMBER_OF_GLOBALS) {
        idx--;
        if (astreq (sname, symbol_table[idx].name, NAMEMAX))
            return (idx);
    }
    return (-1);
}

/**
 * add new symbol to global table
 * @param sname
 * @param identity
 * @param type
 * @param offset size in bytes
 * @param storage
 * @return new index
 */
int add_global (char *sname, int identity, int type, int offset, int storage) {
    SYMBOL *symbol;
    char *buffer_ptr;
    if ((current_symbol_table_idx = find_global(sname)) > -1) {
        return (current_symbol_table_idx);
    }
    if (global_table_index >= NUMBER_OF_GLOBALS) {
        error ("global symbol table overflow");
        return (0);
    }
    current_symbol_table_idx = global_table_index;
    symbol = &symbol_table[current_symbol_table_idx];
    buffer_ptr = symbol->name;
    /* FIXME: only copy so many bytes */
    while (alphanumeric(*buffer_ptr++ = *sname++));
    symbol->identity = identity;
    symbol->type = type;
    symbol->storage = storage;
    symbol->offset = offset;
    global_table_index++;
    return (current_symbol_table_idx);
}

/**
 * add new symbol to local table
 * @param sname
 * @param identity
 * @param type
 * @param offset size in bytes
 * @param storage_class
 * @return 
 */
int add_local (char *sname, int identity, int type, int offset, int storage_class) {
    int k;
    SYMBOL *symbol;
    char *buffer_ptr;

    if ((current_symbol_table_idx = find_locale (sname)) > -1) {
        return (current_symbol_table_idx);
    }
    if (local_table_index >= NUMBER_OF_GLOBALS + NUMBER_OF_LOCALS) {
        error ("local symbol table overflow");
        return (0);
    }
    current_symbol_table_idx = local_table_index;
    symbol = &symbol_table[current_symbol_table_idx];
    buffer_ptr = symbol->name;
    /* FIXME: only copy so many bytes */
    while (alphanumeric(*buffer_ptr++ = *sname++));
    symbol->identity = identity;
    symbol->type = type;
    symbol->storage = storage_class;
    if (storage_class == LSTATIC) {
        data_segment_gdata();
        print_label(k = getlabel());
        output_label_terminator();
        gen_def_storage();
        output_number(offset);
        newline();
        code_segment_gtext();
        offset = k;
    }
    symbol->offset = offset;
    local_table_index++;
    return (current_symbol_table_idx);
}

/**
 * test if next input string is legal symbol name
 */
int symname(char *sname) {
    int k;

    blanks();
    if (!alpha (ch ()))
        return (0);
    k = 0;
    while (alphanumeric(ch ()))
        if (k < NAMEMAX)
            sname[k++] = gch ();
        else
            gch();

    sname[k] = 0;
    return (1);
}

/**
 * print error message
 */
void illname(void) {
    error ("illegal symbol name");
}

/**
 * print error message
 * @param symbol_name
 * @return 
 */
void multidef(char *symbol_name) {
    error ("already defined");
    gen_comment ();
    output_string (symbol_name);
    newline ();
}


void notvoid(int type)
{
    if (type == VOID)
        error("cannot be void type");
}
