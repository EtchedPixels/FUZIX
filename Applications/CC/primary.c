/*
 *	C primary objects
 */

#include <stddef.h>
#include <stdio.h>
#include "compiler.h"


/*
 *	The tokenizer has already done the basic conversion work for us and
 *	labelled the type produced. We just need to turn it into a name
 *	and for strings deal with the fact that a string is actually a
 *	literal holding the address of the characters.
 */
struct node *constant_node(void)
{
	struct node *n;
	unsigned label;
	unsigned t;
	int len;

	/* Strings are special */
	label = quoted_string(&len);
	if (label) {
		/* We have a temporary name */
		n = make_label(label);
		if (in_sizeof) {
			unsigned *idx = idx_get(2);
			*idx = 1;	/* 1 dimension */
			idx[1] = len;
#ifdef TARGET_CHAR_UNSIGNED
			n->type = make_array(UCHAR, idx);
#else
			n->type = make_array(CHAR, idx);
#endif
		} else {
			/* The only case the array is seen as a sized array
			   is in sizeof() so for all other cases decay it to
			   a char pointer in advance to save all the extra
			   type tracking  cost */
#ifdef TARGET_CHAR_UNSIGNED
			n->type = PTRTO | UCHAR;
#else
			n->type = PTRTO | CHAR;
#endif
		}
		return n;
	}
	/* Numeric */
	switch (token) {
	case T_INTVAL:
		t = CINT;
		break;
	case T_LONGVAL:
		t = CLONG;
		break;
	case T_UINTVAL:
		t = UINT;
		break;
	case T_ULONGVAL:
		t = ULONG;
		break;
	case T_FLOATVAL:
		t = FLOAT;
		break;
	default:
		error("invalid value");
		t = CINT;
		break;
	}
	n = make_constant(token_value, t);
	next_token();
	return n;
}

/*
 *	A C language primary. This can be one of several things
 *
 *	1.	Another expression in brackets, in which case we recurse
 *	3.	A name
 *	4.	A constant
 */
struct node *primary(void)
{
	struct node *l;
	unsigned func = 0;
	unsigned name;
	unsigned p = 0;

	/* Expression case first.. a bracketed expression is a primary */
	if (match(T_LPAREN)) {
		l = hier0(1);
		require(T_RPAREN);
		return l;
	}
	/* Names or types */
	name = symname();
	if (token == T_LPAREN)
		func = 1;
	if (name) {
		struct symbol *sym = find_symbol(name, 0);
		/* Weird case you can call a function you've not declared. This
		   makes it int f() */
		if (func && sym == NULL)
			sym = update_symbol_by_name(name, S_EXTERN, deffunctype);
		/* You can't size fields and structs by field/struct name without 
		   the type specifier */
		if (sym == NULL) {
			/* Enum... */
			if (find_constant(name, &p) == 0)
				error("unknown symbol");
			return make_constant(p, CINT);
		}
		/* Primary can be followed by operators and the caller handles those */
		return make_symbol(sym);
	}
	/* Not a name - constants or strings */
	return constant_node();
}
