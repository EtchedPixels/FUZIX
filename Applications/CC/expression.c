#include <stddef.h>
#include "compiler.h"

static void unexarg(void)
{
	error("unexpected argument");
}

static void missedarg(unsigned narg, unsigned ti)
{
	/* Make sure no arguments is acceptable */
	if (!(narg == 0 || ti == ELLIPSIS || ti == VOID))
		error("missing argument");
}

/*
 *	At the moment this is used for functions, but it will be used for
 *	casting, hence all the sanity checks.
 */
struct node *typeconv(struct node *n, unsigned type, unsigned warn)
{
	unsigned nt = type_canonical(n->type);

	/* Weirdness with functions. Properly you should write
	         funcptr = &func,
	   but compilers allow funcptr = func even though this is
	   by strict interpretation nonsense */
	if (PTR(type) == 1 && IS_FUNCTION(n->type)) {
		/* A function type can only be a name, you can't do maths
		   on them or dereference them */
		n->type++;
	}
	/* Handle the various cases where we are working with complex types
	   and they already match */
	if (n->type == type) {
		return n;
	}

	if (!PTR(nt)) {
		/* Casting an arithmetic type to pointer */
		if (PTR(type) && IS_INTARITH(nt)) {
			if (warn && !is_constant_zero(n))
				typemismatch();
			return make_cast(n, type);
		}
		/* You can cast pointers to things but not actual block
		   classes */
		if (!IS_SIMPLE(nt) || !IS_ARITH(nt) ||
			!IS_SIMPLE(type) || !IS_ARITH(type)) {
			error("invalid type conversion");
			return n;
		}
	} else {
		if (type_pointerconv(n, type, warn))
			return make_cast(n, type);
	}
	if (nt == type || (IS_ARITH(nt) && IS_ARITH(type)))
		return make_cast(n, type);
	if ((IS_ARITH(nt) && PTR(type)) || (IS_ARITH(type) && PTR(nt))) {
		if (!warn)
			return make_cast(n, type);
	}
	typemismatch();
	n->type = nt;
	return n;
}

/*
 *	Perform the implicit legacy type conversions C specifies for
 *	unprototyped arguments
 */
struct node *typeconv_implicit(struct node *n)
{
	unsigned t = n->type;
	if (t == CCHAR || t == UCHAR)
		return typeconv(n, CINT, 0);
	if (t == FLOAT)
		return typeconv(n, DOUBLE, 0);
	n->type = type_canonical(t);
	return n;
}

/*
 *	Build an argument tree for right to left stacking
 *
 *	TODO: both here and in the space allocation we need to
 *	do type / size fixes for argument spacing. For example on an 8080
 *	we always push 2 bytes so char as arg takes 2 and we need to do
 *	the right thing.
 */
struct node *call_args(unsigned *narg, unsigned *argt, unsigned *argsize)
{
	struct node *n = expression_tree(0);
	unsigned t;

	/* See what argument type handling is needed */
	if (*argt == VOID)
		unexarg();
	/* Implicit */
	else if (*argt == ELLIPSIS)
		n = typeconv_implicit(n);
	else {
		/* Explicit prototyped argument */
		if (*narg) {
			n = typeconv(n, type_canonical(*argt++), 1);
			(*narg)--;
			/* Once we hit ellipsis we can accept any number
			   of arguments including none */
			if (*argt == ELLIPSIS)
				*narg = 0;
		} else
			unexarg();
	}
	*argsize += target_argsize(n->type);
	t = n->type;
	if (match(T_COMMA)) {
		/* Switch around for calling order */
		n = tree(T_ARGCOMMA, call_args(narg, argt, argsize), n);
		n->type = t;
		return n;
	}
	require(T_RPAREN);
	return n;
}

/*
 *	Generate a function call tree - no type checking arg counts etc
 *	yet. Take any arguments for a function we've not seen a prototype for.
 */

static unsigned dummy_argp = ELLIPSIS;

struct node *function_call(struct node *n)
{
	unsigned type;
	unsigned *argt, *argp;
	unsigned argsize = 0;
	unsigned narg;

	/* Must be a function or pointer to function */
	if (!IS_FUNCTION(n->type)) {
		error("not a function");
		return n;
	}
	type = func_return(n->type);
	argt = func_args(n->type);

	if (!argt)
		fatal("narg");
	narg = *argt;

	if (narg == 0)
		argp = &dummy_argp;
	else
		argp = argt + 1;

	/* A function without arguments */
	if (match(T_RPAREN)) {
		/* Make sure no arguments is acceptable */
		n  = sf_tree(T_FUNCCALL, NULL, n);
		missedarg(narg, argp[0]);
	} else {
		n = sf_tree(T_FUNCCALL, call_args(&narg, argp, &argsize), n);
		missedarg(narg, argp[0]);
	}
	/* Always emit this - some targets have other uses for knowing
	   the boundary of a function call return */
	n->type = type;
	n = tree(T_CLEANUP, n, make_constant(argsize, UINT));
	return n;
}

static struct node *badsizeof(void)
{
	error("bad sizeof");
	return make_constant(1, UINT);
}

/*
 *	sizeof() is a strange C thing that is sort of
 *	a function call but magic.
 */
struct node *get_sizeof(void)
{
	unsigned name;
	unsigned type;
	struct node *n, *r;
	unsigned want_paren = 0;

	if (match(T_LPAREN))
		want_paren = 1;

	/* We will eventually need to count typedefs as type_word */
	if (is_type_word() || is_typedef()) {
		type = type_name_parse(S_NONE, get_type(), &name);
		if (type == UNKNOWN || name)
			return badsizeof();
		require(T_RPAREN);
		return make_constant(type_sizeof(type), UINT);
	}
	/* Sizeof an expression. This is one case that does not degrade to a pointer
	   if the result is an array. We track whether we are in sizeof so that
	   we can optimize some of the symbol table tracking for constanrt strings
	   to keep memory usage a bit more controlled. See primary.c */
	in_sizeof++;
	n = hier0(0);
	r = make_constant(type_sizeof(n->type), UINT);
	free_tree(n);
	if (want_paren)
		require(T_RPAREN);
	in_sizeof--;
	return r;
}

/*
 *	Postfixed array and structure dereferences (basically the same but
 *	one is named and re-typed), and function calls.
 *
 *	left to right
 */
static struct node *hier11(void)
{
	int direct;
	struct node *l, *r;
	unsigned ptr;
	unsigned scale;
	unsigned *tag;
	unsigned lt;

	l = primary();
	lt = l->type;

	if (token == T_LSQUARE || token == T_LPAREN || token == T_DOT
	    || token == T_POINTSTO) {
		for (;;) {
			lt = l->type;
			ptr = PTR(type_canonical(lt));
			if (match(T_LSQUARE)) {
				if (ptr == 0) {
					error("can't subscript");
					junk();
					return l;
				}
				/* TODO: assumes ptrdiff is an integer sized type */
				r = typeconv(expression_tree(1), UINT, 0);
				require(T_RSQUARE);
				scale = type_ptrscale(lt);
				l = tree(T_PLUS, make_rval(l),
					 tree(T_STAR, r,
					      make_constant(scale, UINT)));
				l->flags |= LVAL;
				/* Force the type back correct */
				lt = type_deref(lt);
				l->type = lt;
			} else if (match(T_LPAREN)) {
				l = function_call(make_rval(l));
			} else if ((direct = match(T_DOT))
				   || match(T_POINTSTO)) {
				if (direct == 0) {
					/* The pointer we have holds the address of the
					   struct which is thus an lval */
					l = make_rval(l);
					l->flags |= LVAL;
					lt = type_deref(lt);
				}
				if (PTR(lt)
				    || !IS_STRUCT(lt)) {
					error("can't take member");
					junk();
					l->type = CINT;
					return l;
				}
				tag = struct_find_member(lt, symname());
				if (tag == NULL) {
					error("unknown member");
					/* So we don't internal error later */
					l->type = CINT;
					return l;
				}
				/* Type is tricky here. It's a pointer to the
				   type of the field, sort of - except for the
				   maths. TODO This will need work for things like
				   bytepointer and word addressed machines */
				l->type = PTRTO | tag[1];
				l = tree(T_PLUS, l,
					 make_constant(tag[2], UINT));
				l->flags |= LVAL;
				l->type = tag[1];
			} else
				return l;
		}
	}
	return l;
}

/*
 *	Unary operators
 *
 *	type_scale() typechecks the increment/decrement operators
 *
 *	These all associate right to left
 *
 *	FIXME: sizeof() belongs here not primary
 */
static struct node *hier10(void)
{
	struct node *l, *r;
	unsigned op;
	unsigned name;
	unsigned t;
	unsigned is_tcast = 0;
	unsigned s;

	/* C syntax fun. The grammar has two cases here for (, the first
	   is a primary (a bracketed expression) the second is a typecast
	   which has a *different* priority */

	op = token;
	if (op == T_LPAREN) {
		next_token();
		if (is_modifier() || is_type_word() || is_typedef())
			is_tcast = 1;
		push_token(T_LPAREN);
	}
	if (token != T_PLUSPLUS
	    && token != T_SIZEOF
	    && token != T_MINUSMINUS
	    && token != T_MINUS
	    && token != T_TILDE
	    && is_tcast == 0
	    && token != T_BANG && token != T_STAR && token != T_AND) {
		/* Check for trailing forms */
		l = hier11();
		if (token == T_PLUSPLUS || token == T_MINUSMINUS) {
			if (!(l->flags & LVAL)) {
				needlval();
				return l;
			}
			op = token;
			/* It's an lval so we want the pointer form */
			s = type_scale(l->type);
			next_token();
			/* Put the constant on the right for convenience */
			/* We can know the constant will fit a UINT for 16bit boxes
			   but 32bit ptr 16bit int this is borked FIXME */
			if (PTR(l->type))
				r = sf_tree(op, l, make_constant(s, UINT));
			else
				r = sf_tree(op, l, make_constant(s, l->type));
			return r;
		}
		return l;
	}

	next_token();
	switch (op) {
	case T_PLUSPLUS:
	case T_MINUSMINUS:
		r = hier10();
		if (!(r->flags & LVAL)) {
			needlval();
			return r;
		}
		if (op == T_PLUSPLUS)
			op = T_PLUSEQ;
		else
			op = T_MINUSEQ;
		/* FIXME: turning it into a PLUSEQ/MINUSEQ implies the right side
		   type needs to be ptr size not UINT ?? */
		if (PTR(r->type))
			return sf_tree(op, r, make_constant(type_scale(r->type), UINT));
		/* We should probably keep an optimized ++/-- FIXME */
		return sf_tree(op, r, make_constant(1, r->type));
	case T_TILDE:
		/* Floating point bit ops are not allowed */
		r = make_rval(hier10());
		if (!IS_INTARITH(r->type))
			badtype();
		return tree(op, NULL, r);
	case T_MINUS:
		/* Floating point allowed */
		r = make_rval(hier10());
		if (!IS_ARITH(r->type) && !PTR(r->type))
			badtype();
		return tree(T_NEGATE, NULL, r);
	case T_BANG:
		/* Floating point allowed */
		r = make_rval(hier10());
		if (!IS_ARITH(r->type) && !PTR(r->type))
			badtype();
		return bool_tree(tree(op, NULL, r));
	case T_STAR:
		r = make_rval(hier10());
		if (!PTR(r->type))
			badtype();
		r->flags |= LVAL;
		r->type = type_deref(r->type);
		return r;
	case T_AND:
		r = hier10();
		/* If it's an lvalue then just stop being an lvalue */
		if (r->flags & LVAL) {
			r->flags &= ~LVAL;
			/* We are now a pointer to */
			r->type = type_ptr(r->type);
			return r;
		}
		r = tree(T_ADDROF, NULL, r);
		r->type = type_addrof(r->type);
		return r;
	case T_LPAREN:
		/* Should be a type without a name */
		t = type_name_parse(S_NONE, get_type(), &name);
		require(T_RPAREN);
		if (t == UNKNOWN || name)
			badtype();
		return typeconv(make_rval(hier10()), t, 0);
	case T_SIZEOF:
		return get_sizeof();

	}
	fatal("h10");
}

/*
 *	Multiplication, division and remainder
 *	The '%' operator does not apply to floating point.
 *
 *	As usual left associative
 */
static struct node *hier9(void)
{
	struct node *l;
	struct node *r;
	unsigned op;
	l = hier10();
	while (token == T_STAR || token == T_PERCENT || token == T_SLASH) {
		op = token;
		next_token();
		l = make_rval(l);
		r = make_rval(hier10());
		if (op == T_PERCENT)
			l = intarith_tree(op, l, r);
		else
			l = arith_tree(op, l, r);
	}
	return l;
}

/*
 *	Addition and subtraction. Messy because of the pointer scaling
 *	rules and even more so because of arrays.
 *
 *	As usual left associative
 */

static struct node *hier8(void)
{
	struct node *l, *r;
	unsigned op;
	int scale = 1;
	unsigned rt;

	l = hier9();

	while (token == T_PLUS || token == T_MINUS) {
		op = token;
		next_token();

		l = make_rval(l);
		r = make_rval(hier9());

		/* Deal with the non pointer case firt */
		if (IS_ARITH(l->type) && IS_ARITH(r->type))
			l = arith_tree(op, l, r);
		else {
			scale = type_ptrscale_binop(op, l, r, &rt);
			/* The type checking was done in type_ptrscale_binop */
			if (scale < 0)
				l = tree(T_SLASH, tree(op, l, r), make_constant(-scale, UINT));
			/* TODO: these two assume ptrdiff is an int sized type */
			else if (PTR(l->type)) {
				r = typeconv(r, UINT, 0);
				if (scale)
					l = tree(op, l, tree(T_STAR, r, make_constant(scale, UINT)));
				else
					l = tree(op, l, r);
			} else {
				l = typeconv(l, UINT, 0);
				if (scale)
					l = tree(op, tree(T_STAR, l, make_constant(scale, UINT)), r);
				else
					l = tree(op, l, r);
			}
			l->type = rt;
		}
	}
	return l;
}


/*
 *	Shifts
 */
static struct node *hier7(void)
{
	struct node *l;
	unsigned op;
	l = hier8();
	while(token == T_GTGT || token == T_LTLT) {
		op = token;
		next_token();
		/* The tree code knows about the shift rule being different for types */
		l = intarith_tree(op, make_rval(l), make_rval(hier8()));
	}
	return l;
}

/*
 *	Relational comparison operators
 *
 *	Left to right
 */
static struct node *hier6(void)
{
	struct node *l;
	unsigned op;
	l = hier7();
	while(token == T_LT || token == T_GT
	    || token == T_LTEQ || token == T_GTEQ) {
		op = token;
		next_token();
		l = ordercomp_tree(op, make_rval(l), make_rval(hier7()));
	}
	return l;
}

/*
 *	Equality and not equal operators
 *
 *	Left to right
 */
static struct node *hier5(void)
{
	struct node *l;
	unsigned op;
	l = hier6();
	while (token == T_EQEQ || token == T_BANGEQ) {
		op = token;
		next_token();
		l = ordercomp_tree(op, make_rval(l), make_rval(hier6()));
	}
	return l;
}

/*
 *	Bitwise and
 */
static struct node *hier4(void)
{
	struct node *l;
	l = hier5();
	while(match(T_AND))
		l = intarith_tree(T_AND, make_rval(l), make_rval(hier5()));
	return l;
}

/*
 *	Bitwise xor
 */
static struct node *hier3(void)
{
	struct node *l;
	l = hier4();
	while(match(T_HAT))
		l = intarith_tree(T_HAT, make_rval(l), make_rval(hier4()));
	return l;
}

/*
 *	Bitwise or
 *
 *	Bitwise operators also associate left to right
 */
static struct node *hier2(void)
{
	struct node *l;
	l = hier3();
	while(match(T_OR))
		l = intarith_tree(T_OR, make_rval(l), make_rval(hier3()));
	return l;
}

/*
 *	logical and
 *
 *	Evaulates left to right, may shortcut evaulation
 */
static struct node *hier1c(void)
{
	struct node *l;
	l = hier2();
	while(match(T_ANDAND))
		l = logic_tree(T_ANDAND, make_rval(l), make_rval(hier2()));
	return l;
}

/*
 *	logical or
 *
 *	Evaulates left to right, may shortcut evaulation
 */
static struct node *hier1b(void)
{
	struct node *l;
	l = hier1c();
	while(match(T_OROR))
		l = logic_tree(T_OROR, make_rval(l), make_rval(hier1c()));
	return l;
}

/*
 *	The ?: operator. We turn this into trees, the backend turns it into
 *	bramches/
 *
 *	Type rules are bool for ? and both sides matching for :
 *
 *	: is very unrestricted, you can do things like
 *	(a?b:c).x  or (a?b:c)(foo);
 *
 *	?: associates right to left.
 */
static struct node *hier1a(void)
{
	struct node *l;
	struct node *a1, *a2;
	unsigned lt;
	unsigned a1t, a2t;

	l = hier1b();
	if (!match(T_QUESTION))
		return l;

	l = make_rval(l);
	lt = l->type;

	/* Must be convertible to a boolean != 0 test */
	/* TODO: is float ? valid */
	if (!PTR(lt) && !IS_ARITH(lt))
		badtype();
	/* Now do the left of the colon */
	a1 = make_rval(hier1a());
	if (!match(T_COLON)) {
		error("missing colon");
		return l;
	}
	/* We can have a ? a ? b : c : d ? e : f .. */
	a2 = make_rval(hier1a());

	a1t = type_canonical(a1->type);
	a2t = type_canonical(a2->type);

	/* Check the two sides of colon are compatible */
	if (a1t == a2t || type_pointermatch(a1, a2) || (IS_ARITH(a1t) && IS_ARITH(a2t))) {
		a2 = tree(T_QUESTION, bool_tree(l), tree(T_COLON, a1, typeconv(a2, a1t, 1)));
		/* Takes the type of the : arguments not the ? */
		a2->type = a1t;
	}
	else
		badtype();
	return a2;
}

/*
 *	Assignment between an lval on the left and an rval on the right
 *
 *	Handle pointer scaling on += and -= by emitting the maths into the
 *	tree.
 *
 *	Assignment associates right to left
 */
static struct node *hier1(void)
{
	struct node *l, *r;
	unsigned fc;
	unsigned scale = 1;
	l = hier1a();
	if (match(T_EQ)) {
		if ((l->flags & LVAL) == 0)
			needlval();
		r = make_rval(hier1());
		/* You can't assign to an array/offset, you assign to
		   the underlying type */
		l->type = type_canonical(l->type);
		if (!IS_SIMPLE(l->type) && !PTR(l->type)) {
			badtype();
			return l;
		}
		return assign_tree(l, r);	/* Assignment */
	} else {
		fc = token;
		if (match(T_MINUSEQ) ||
		    match(T_PLUSEQ) ||
		    match(T_STAREQ) ||
		    match(T_SLASHEQ) ||
		    match(T_PERCENTEQ) ||
		    match(T_SHREQ) ||
		    match(T_SHLEQ) ||
		    match(T_ANDEQ) || match(T_HATEQ) || match(T_OREQ)) {
			if ((l->flags & LVAL) == 0) {
				needlval();
				return l;
			}
			/* TODO: review - fix things like float ^= and fold
			   these rules and the non eq versions together somehow */
			r = make_rval(hier1());
			switch (fc) {
			case T_MINUSEQ:
			case T_PLUSEQ:
				scale = type_scale(l->type);
			case T_STAREQ:
			case T_SLASHEQ:
				if (!IS_ARITH(r->type))
					badtype();
				break;
			default:
				if (!IS_INTARITH(r->type))
					badtype();
			}
			/* Get the type converted to the bit width of the maths */
			r = make_cast(r, l->type);
			if (scale)
				return sf_tree(fc, l,
					    tree(T_STAR, r,
						 make_constant(scale, UINT)));
			return sf_tree(fc, l, r);
		} else
			return l;
	}
	/* gcc */
	return NULL;
}

/*  Comma: left to right which means the final type is the right hand type of the final
    expression */
struct node *hier0(unsigned comma)
{
	struct node *l = hier1();
	struct node *r;
	while (comma && match(T_COMMA)) {
		l->flags |= NORETURN;
		r = hier0(comma);
		/* The return of a comma operator is never an lval */
		l = tree(T_COMMA, make_rval(l), make_rval(r));
		l->type = r->type;
	}
	return l;
}
/*
 *	Top level of the expression tree. Make the tree an rval in case
 *	we need the result. Allow for both the expr,expr,expr format and
 *	the cases where C doesnt allow it (expr, expr in function calls
 *	or initializers is not the same
 */

struct node *expression_tree(unsigned comma)
{
	return make_rval(hier0(comma));
}


/* Generate an expression and write it the output */
unsigned expression(unsigned comma, unsigned mkbool, unsigned flags)
{
	struct node *n;
	unsigned t;
	if (token == T_SEMICOLON)
		return VOID;
	n = expression_tree(comma);
	if (mkbool && !(flags & NORETURN)) {
		/* Float and double are valid */
		if (!IS_ARITH(n->type) && !PTR(n->type))
			typemismatch();
		/* NORETURN CCONLY etc also apply both to the bool node and the original */
		n->flags |= flags;
		n = bool_tree(n);
	}
	n->flags |= flags;
	t = n->type;
	write_tree(n);
	return t;
}

/* We need a another version of this for initializers that allows global or
   static names (and string labels) too */

unsigned const_int_expression(void)
{
	unsigned v = 1;
	struct node *n = expression_tree(0);

	if (n->op == T_CONSTANT)
		v = n->value;
	else
		notconst();
	free_tree(n);
	return v;
}

/* This is used for bracketed expressions following keywords such as if. These are
   normally boolean/condition code except switch */
unsigned bracketed_expression(unsigned mkbool)
{
	unsigned t;
	require(T_LPAREN);
	t = expression(1, mkbool, mkbool ? CCONLY : 0);
	require(T_RPAREN);
	return t;
}

void expression_or_null(unsigned mkbool, unsigned flags)
{
	struct node *n;
	if (token == T_SEMICOLON || token == T_RPAREN) {
		/* A null tree - force the type to void so we can spot it in the backend */
		n = tree(T_NULL, NULL, NULL);
		n->type = VOID;
		write_tree(n);
	} else
		expression(1, mkbool, flags);
}

void expression_typed(unsigned type)
{
	if (type == VOID && token == T_SEMICOLON) {
		write_tree(tree(T_NULL, NULL, NULL));
		return;
	}
	write_tree(typeconv(expression_tree(0), type, 0));
}
