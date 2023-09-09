#include <stdio.h>
#include "compiler.h"


unsigned type_deref(unsigned t)
{
	unsigned p = PTR(t);
	if (p == 0) {
		/* If we have a dereference of the base of an array object it's a dereference
		   of the type of the array - which may be a pointer type */
		if (!IS_ARRAY(t) || !PTR(array_type(t))) {
			error("cannot dereference");
			return CINT;
		}
		return array_type(t) - 1;
	}
	/* Turns into the base type */
	if (p == 1 && IS_ARRAY(t))
		return array_type(t);
	return --t;
}

unsigned type_ptr(unsigned t)
{
	if (PTR(t) == 7)
		indirections();
	else
		t++;
	return t;
}

unsigned type_addpointer(unsigned t, unsigned ptr)
{
	if (PTR(t) + ptr > 7)
		indirections();
	else
		t += ptr;
	return t;
}

/*
 *	Handle array v type pointers.
 */
unsigned type_canonical(unsigned t)
{
	/* An array is pointer to the base type of the array */
	if (IS_ARRAY(t)) {
		struct symbol *s = symbol_ref(t);
		/* Shouldn't be possible */
		if (PTR(s->type) + PTR(t) > 7)
			indirections();
		t = s->type + PTR(t);
	}
	/* Functions automagically turn into pointers to themselves */
	if (IS_FUNCTION(t) && !PTR(t))
		return PTRTO | t;
	return t;
}

/*
 *	It is possible to have more depths of pointer than the
 *	array, because an array may itself be of pointer types. In that
 *	case we behave like a conventional object for the extra depths.
 */
unsigned type_arraysize(unsigned t)
{
	struct symbol *sym = symbol_ref(t);
	unsigned *p = sym->data.idx;
	unsigned n = *p;
	unsigned d = PTR(t);
	unsigned s = type_sizeof(sym->type);

	p += n;

	while(n--) {
		s *= *p--;
		if (--d == 0)
			return s;
	}
	/* We are some depth of pointer to an array object so our size
	   goes back to the size of the pointer */
	return type_sizeof(sym->type + d + 1);
}

unsigned type_sizeof(unsigned t)
{
	/* Handle array first, because it is also pointer but doesn't
	   behave like a normal pointer */
	if (IS_ARRAY(t))
		return type_arraysize(t);
	if (PTR(t) || IS_SIMPLE(t))
		return target_sizeof(t);
	if (IS_STRUCT(t)) {
		struct symbol *s = symbol_ref(t);
		unsigned *p = s->data.idx;
		if (s->infonext & INITIALIZED)
			return p[1];
		error("struct/union not declared");
		return 1;
	}
	error("can't size type");
	return 1;
}

unsigned type_ptrscale(unsigned t) {
	if (!PTR(t) && !PTR(type_canonical(t))) {
		error("not a pointer");
		return 1;
	}
	/* void * is special */
	if (t == PTRTO + VOID)
		return 1;
	return target_scale_ptr(t, type_sizeof(type_deref(t)));
}

unsigned type_scale(unsigned t) {
	/* We can scale pointers to complex objects */
	if (PTR(t))
		return type_ptrscale(t);
	/* But you can't do maths on array struct and function objects */
	if (!IS_SIMPLE(t) && !IS_ARRAY(t)) {
		badtype();
		return 1;
	}
	/* Numeric objects don't scale */
	return 1;
}

/* lvalue conversion is handled by caller */
unsigned type_addrof(unsigned t) {
	if (PTR(t))
		return t - 1;
	error("cannot take address");
	return VOID + 1;
}

/*
 *	Can we turn the right hand object into the left hand type
 *	for pointers.
 *
 *	TODO: Array
 */
int type_pointerconv(struct node *r, unsigned lt, unsigned warn)
{
    unsigned rt = type_canonical(r->type);
    /* The C zero case */
    if (is_constant_zero(r) && PTR(lt))
        return 1;
    /* Not pointers */
    if (!PTR(lt) || !PTR(rt))
        return 0;
    /* Same depth and type */
    if (lt == rt)
        return 1;
    /* void * is fine */
    if (BASE_TYPE(lt) == VOID)
        return 1;
    if (BASE_TYPE(rt) == VOID)
        return 1;
    /* sign errors */
    if (BASE_TYPE(lt) < FLOAT && (lt ^ rt) == UNSIGNED) {
        if (warn)
	    warning("sign mismatch");
        return 1;
    }
    return !warn;
}

/*
 *	Do two pointers match for conversion purposes
 */
int type_pointermatch(struct node *l, struct node *r)
{
    unsigned lt = type_canonical(l->type);
    unsigned rt = type_canonical(r->type);
    /* The C zero case */
    if (is_constant_zero(l) && PTR(rt))
        return 1;
    return type_pointerconv(r, lt, 1);
}

unsigned type_ptrscale_binop(unsigned op, struct node *l, struct node *r,
			     unsigned *type) {
	unsigned lt = l->type;
	unsigned rt = r->type;

	/* Get the target type required to hold this kind of pointer
	   difference */
	*type = target_ptr_arith(lt);

	/* Pointermatch rules apply, but for pointer, arithmetic cases we
	   want to do arithmetic, so don't treat 0 as a pointer type */
	if (PTR(lt) && PTR(rt) && type_pointermatch(l, r)) {
		if (op == T_MINUS)
			return -type_ptrscale(rt);
		else {
			error("invalid pointer difference");
			return 1;
		}
	}
	if (PTR(lt) && IS_ARITH(rt)) {
		*type = lt;
		return type_ptrscale(lt);
	}
	if (PTR(rt) && IS_ARITH(lt)) {
		*type = rt;
		return type_ptrscale(rt);
	}
	invalidtype();
	return 1;
}
