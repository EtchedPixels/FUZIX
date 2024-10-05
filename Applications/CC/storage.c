#include <unistd.h>
#include "compiler.h"

unsigned is_storage_word(void)
{
	return (token >= T_AUTO && token <= T_STATIC);
}

unsigned get_storage(unsigned dflt)
{
	unsigned t;
	skip_modifiers();
	if (!is_storage_word())
		return dflt;
	t = token;
	next_token();
	switch (t) {
	case T_AUTO:
		return S_AUTO;
	case T_REGISTER:
		return S_REGISTER;
	case T_STATIC:
		if (dflt == S_AUTO)
			return S_LSTATIC;
		else
			return S_STATIC;
	case T_EXTERN:
		return S_EXTERN;
	}
	/* gcc */
	return 0;
}

/*
 *	Storage I/O - hacks for now
 */

void put_typed_data(struct node *n)
{
	unsigned op = n->op;
	out_block("%[", 2);
	if (op != T_CASELABEL && op != T_PAD && op != T_LABEL &&
		op != T_NAME && op != T_CONSTANT)
		notconst();
	out_block(n, sizeof(struct node));
}

void put_padding_data(unsigned space)
{
	struct node *n = make_constant(space, UINT);
	n->op = T_PAD;
	put_typed_data(n);
	free_node(n);
}

void put_typed_constant(unsigned type, unsigned long value)
{
	struct node *n = make_constant(value, type);
	put_typed_data(n);
	free_node(n);
}

void put_typed_case(unsigned tag, unsigned entry)
{
	struct node *n = make_constant(tag, PTRTO|VOID);
	n->op = T_CASELABEL;
	n->val2 = entry;
	put_typed_data(n);
	free_node(n);
}
