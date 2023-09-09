/*
 *	Handle all the int a, char *b[433], *x() stuff
 */

#include <stdio.h>
#include <string.h>
#include "compiler.h"

unsigned is_modifier(void)
{
	/* Track volatile hints */
	if (token == T_VOLATILE)
		voltrack++;
	return (token == T_CONST || token == T_VOLATILE);
}

unsigned is_type_word(void)
{
	return (token >= T_CHAR && token <= T_VOID);
}

/* This one is more expensive so use it last when possible */
struct symbol *is_typedef(void)
{
	return find_symbol_by_class(token, S_TYPEDEF);
}

void skip_modifiers(void)
{
	while (is_modifier())
		next_token();
}

static unsigned typeconflict(void) {
	error("type conflict");
	return CINT;
}

/* Structures */
static unsigned structured_type(unsigned sflag)
{
	struct symbol *sym;
	unsigned name = symname();

	sym = update_struct(name, sflag);
	if (sym == NULL) {
		error("not a struct");
		junk();
		return CINT;
	}
	/* Now see if we have a struct declaration here */
	if (token == T_LCURLY) {
		struct_declaration(sym);
	}
	/* Encode the struct. The caller deals with any pointer, array etc
	   notation */
	return type_of_struct(sym);
}

static unsigned once_flags;

static void set_once(unsigned bits)
{
	if (once_flags & bits)
		typeconflict();
	once_flags |= bits;
}

static unsigned base_type(void)
{
	once_flags = 0;
	unsigned type = CINT;

	while (is_type_word()) {
		switch (token) {
		case T_SHORT:
			set_once(1);
			break;
		case T_LONG:
			/* For now -- long long is for the future */
			set_once(2);
			break;
		case T_UNSIGNED:
			set_once(4);
			break;
		case T_SIGNED:
			set_once(8);
			break;
		case T_CHAR:
			set_once(16);
			type = CCHAR;
			break;
		case T_INT:
			set_once(16);
			type = CINT;
			break;
		case T_FLOAT:
			set_once(16);
			type = FLOAT;
			break;
		case T_DOUBLE:
			set_once(16);
			type = DOUBLE;
			break;
		case T_VOID:
			set_once(16);
			type = VOID;
			break;
		}
		next_token();
	}
	/* Now put it all together */

	/* No signed unsigned or short longs */
	if ((once_flags & 3) == 3 || (once_flags & 12) == 12)
		return typeconflict();
	/* No long or short char */
	if (type == CCHAR && (once_flags & 3))
		return typeconflict();
	/* No signed/unsigned/short float or double */
	if ((type == FLOAT || type == DOUBLE) && (once_flags & 13))
		return typeconflict();
	/* No void modifiers */
	if (type == VOID && (once_flags & 15))
		return typeconflict();
	/* long */
	if (type == CINT && (once_flags & 2))
		type = CLONG;
	if (type == FLOAT && (once_flags & 2))
		type = DOUBLE;
	/* short */
	if (type == CINT && (once_flags & 1))
		type = CSHORT;
	/* signed/unsigned */
#ifdef TARGET_CHAR_UNSIGNED
	if (type == CCHAR)
		type |= UNSIGNED;
#endif
	if (once_flags & 4)
		type |= UNSIGNED;
	/* We don't deal with default unsigned char yet .. */
	if (once_flags & 8)
		type &= ~UNSIGNED;
	return target_type_remap(type);
}

unsigned get_type(void)
{
	unsigned sflag = 0;
	struct symbol *sym;
	unsigned type;

	skip_modifiers();

	if (match(T_ENUM))
		type = enum_body();
	else if ((sflag = match(T_STRUCT)) || match(T_UNION))
		type = structured_type(sflag);
	else if (is_type_word())
		type = base_type();
	else {
		/* Check for typedef */
		sym = find_symbol_by_class(token, S_TYPEDEF);
		if (sym == NULL)
			return UNKNOWN;
		next_token();
		type = sym->type;
	}
	skip_modifiers();
	return type;
}

#define MAX_DECL	16
#define MAX_TMPIDX	64

struct declstack {
	unsigned ptr;
	unsigned form;
	unsigned *idx;
};

struct declstack decls[MAX_DECL];
struct declstack *decp = decls;

static void declarator_add(unsigned form, unsigned ptr, unsigned *idx)
{
	if (decp == &decls[MAX_DECL])
		fatal("type too complex");
	decp->idx = idx;
	decp->ptr = ptr;
	decp->form = form;
	decp++;
}

/*
 *	Parse an ANSI C style function (we don't do K&R at all)
 *
 *	We build a vector of type descriptors for the arguments and then
 *	build a type from that. All functions with the same argument pattern
 *	have the same type code. That saves us a ton of space and also means
 *	we can compare function pointer equivalence trivially
 *
 *	ptr tells us if this is a pointer declaration and therefore must
 *	not have a body. We will need that once we move the body parsing
 *	here.
 */


static void parse_function_arguments(unsigned *tplt)
{
	unsigned *tn = tplt + 1;
	unsigned t;
	unsigned an;
	struct symbol *sym;

	/* Parse the bracketed arguments if any and nail them to the
	   symbol. */
	while (token != T_RPAREN) {
		/* TODO: consider K&R support */
		if (tn == tplt + 33)
			fatal("too many arguments");
		if (token == T_ELLIPSIS) {
			next_token();
			*tn++ = ELLIPSIS;
			break;
		}
		/* Throw away register hint on arguments */
		match(T_REGISTER);
		t = get_type();
		if (t == UNKNOWN)
			t = CINT;
		t = type_name_parse(S_NONE, t, &an);
		if (t == VOID) {
			*tn++ = VOID;
			break;
		}
		if (!PTR(t) && (IS_STRUCT(t) || IS_FUNCTION(t))) {
			error("cannot pass objects");
			t = CINT;
		}
		t = type_canonical(t);
		if (an) {
			sym = update_symbol_by_name(an, S_ARGUMENT, t);
			sym->data.offset = assign_storage(t, S_ARGUMENT);
			*tn++ = t;
		} else {
			assign_storage(t, S_ARGUMENT);
			*tn++ = t;
		}
		if (!match(T_COMMA))
			break;
	}
	require(T_RPAREN);
	/* A zero length comes from () and means 'whatever' so semantically
	   for us at least 8) it's an ellipsis only */
	if (tn == tplt + 1)
		*tn++ = ELLIPSIS;
	*tplt = tn - tplt - 1;
}

void type_parse_function(unsigned ptr)
{
	/* Function returning the type accumulated so far */
	unsigned tplt[33];	/* max 32 typed arguments */
	unsigned argsave, locsave;
	unsigned *idx;

	mark_storage(&argsave, &locsave);
	parse_function_arguments(tplt);
	pop_storage(&argsave, &locsave);

	idx = sym_find_idx(S_FUNCDEF, tplt, *tplt + 1);
	declarator_add(T_LPAREN, ptr, idx);
}

void type_parse_array(unsigned ptr)
{
	unsigned dim[9];
	unsigned ndim = 0;
	int n;
	unsigned *idx;

	while(token == T_LSQUARE) {
		next_token();
		if (token == T_RSQUARE) {
			if (ndim)
				error("size required");
			else
				n = 0;
		} else {
			n = const_int_expression();
			if (n < 1 ) {
				error("bad size");
				n = 1;
			}
		}
		require(T_RSQUARE);
		if (ndim == 8) {
			error("too many dimensions");
			break;
		}
		dim[++ndim] = n;
	}
	dim[0] = ndim;
	idx = sym_find_idx(S_ARRAY, dim, *dim + 1);
	declarator_add(T_LSQUARE, ptr, idx);
}


static void declarator(unsigned *name, unsigned depth)
{
	unsigned ptr = 0;

	/* Count the number of indirections at this level */
	skip_modifiers();
	while (match(T_STAR)) {
		skip_modifiers();
		ptr++;
	}
	skip_modifiers();

	/* Process any bracketed declarations */
	if (token == T_LPAREN) {
		next_token();
		declarator(name, depth + 1);
		require(T_RPAREN);
	}

	/* Have we found our name. If so we know the base type and the name */
	else if (token >= T_SYMBOL) {	/* It's a name */
		*name = token;
		next_token();
	}
	/* Now work rightwards and outwards */
	/* Do we have array declarations to the right */
	if (token == T_LSQUARE) {
		/* Get the dimensions */
		type_parse_array(ptr);
	}
	else if (token == T_LPAREN) {
		next_token();
		type_parse_function(ptr);
	} else
		/* A simple object */
		declarator_add(0, ptr, NULL);
}

/*
 *	Given a base type find the full type and name
 */
static unsigned do_type_name_parse(unsigned type, unsigned *name)
{
	struct declstack *dp = decp;

	*name = 0;

	/* Walk the declaration building a type stack.*/
	declarator(name, 0);

	/* Now work back down the stack from outside inwards so we can
	   create our types cleanly */
	/* SORT OUT OFF BY ONES */
	while (--decp >= dp) {
		type = type_addpointer(type, decp->ptr);
		switch(decp->form) {
		case T_LSQUARE:
			type = make_array(type, decp->idx);
			break;
		case T_LPAREN:
			type = make_function(type, decp->idx);
			break;
		}
	}
	decp++;
	/* And return the final resulting type */
	return type;
}

static unsigned parse_depth = 0;

unsigned type_name_parse(unsigned storage, unsigned type, unsigned *name)
{
	struct symbol *ltop = mark_local_symbols();
	parse_depth++;
	if (parse_depth == 8)
		fatal("too complex");
	type = do_type_name_parse(type, name);
	if (IS_FUNCTION(type) && !PTR(type) && token == T_LCURLY) {
		struct symbol *sym = update_symbol_by_name(*name, storage, type);
		unsigned argsave, locsave;
		/* Handle the function body */
		if (sym->infonext & INITIALIZED)
			error("duplicate function");
		if (storage == S_AUTO || storage == S_NONE)
			error("function not allowed");
		if (storage == S_EXTDEF)
			header(H_EXPORT, *name, 0);
		mark_storage(&argsave, &locsave);
		init_storage();
		function_body(storage, *name, type);
		pop_storage(&argsave, &locsave);
		sym->infonext |= INITIALIZED;
		funcbody = 1;
	}
	pop_local_symbols(ltop);
	parse_depth--;
	return type;
}
