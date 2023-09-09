#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "backend.h"

/* For now assume 8/16bit */
#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */

#define T_NREF		(T_USER)		/* Load of C global/static */
#define T_CALLNAME	(T_USER+1)		/* Function call by name */
#define T_NSTORE	(T_USER+2)		/* Store to a C global/static */
#define T_LREF		(T_USER+3)		/* Ditto for local */
#define T_LSTORE	(T_USER+4)
#define T_LBREF		(T_USER+5)		/* Ditto for labelled strings or local static */
#define T_LBSTORE	(T_USER+6)

static void squash_node(struct node *n, struct node *o)
{
	n->value = o->value;
	n->val2 = o->val2;
	n->snum = o->snum;
	free_node(o);
}

static void squash_left(struct node *n, unsigned op)
{
	struct node *l = n->left;
	n->op = op;
	squash_node(n, l);
	n->left = NULL;
}

static void squash_right(struct node *n, unsigned op)
{
	struct node *r = n->right;
	n->op = op;
	squash_node(n, r);
	n->right = NULL;
}

/*
 *	Our chance to do tree rewriting. We don't do much for the 8080
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 */
struct node *gen_rewrite_node(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	unsigned op = n->op;
	unsigned nt = n->type;
	/* Rewrite references into a load operation */
	if (nt == CCHAR || nt == UCHAR || nt == CSHORT || nt == USHORT || PTR(nt)) {
		if (op == T_DEREF) {
			if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
				if (r->op == T_ARGUMENT)
					r->value += frame_len;
				squash_right(n, T_LREF);
				return n;
			}
			if (r->op == T_NAME) {
				squash_right(n, T_NREF);
				return n;
			}
			if (r->op == T_LABEL) {
				squash_right(n, T_LBREF);
				return n;
			}
		}
		if (op == T_EQ) {
			if (l->op == T_NAME) {
				squash_left(n, T_NSTORE);
				return n;
			}
			if (l->op == T_LABEL) {
				squash_left(n, T_LBSTORE);
				return n;
			}
			if (l->op == T_LOCAL || l->op == T_ARGUMENT) {
				if (l->op == T_ARGUMENT)
					l->value += frame_len;
				squash_left(n, T_LSTORE);
				return n;
			}
		}
	}
	/* Eliminate casts for sign, pointer conversion or same */
	if (op == T_CAST) {
		if (nt == r->type || (nt ^ r->type) == UNSIGNED ||
		 (PTR(nt) && PTR(r->type))) {
			free_node(n);
			return r;
		}
	}
	/* Rewrite function call of a name into a new node so we can
	   turn it easily into call xyz */
	if (op == T_FUNCCALL && r->op == T_NAME && PTR(r->type) == 1) {
		n->op = T_CALLNAME;
		n->snum = r->snum;
		n->value = r->value;
		free_node(r);
		n->right = NULL;
	}
	return n;
}

/* Export the C symbol */
void gen_export(const char *name)
{
	printf("	.export _%s\n", name);
}

void gen_segment(unsigned s)
{
	switch(s) {
	case A_CODE:
		printf("\t.code\n");
		break;
	case A_DATA:
		printf("\t.data\n");
		break;
	case A_LITERAL:
		printf("\t.literal\n");
		break;
	case A_BSS:
		printf("\t.bss\n");
		break;
	default:
		error("gseg");
	}
}

void gen_prologue(const char *name)
{
	printf("_%s:\n", name);
	printf("\tz80call __bytecode\n");
}

/* Generate the stack frame */
void gen_frame(unsigned size)
{
	frame_len = size + 2;	/* 2 for the return addr */
	if (size) {
		sp += size;
		if (size < 256) {
			printf("\tspmod8 %d\n", -size);
		} else {
			printf("\tspmod %d\n", -size);
		}
	}
}

void gen_epilogue(unsigned size)
{
	if (sp != size) {
		error("sp");
	}
	if (size) {
		sp -= size;
		if (size < 256) {
			printf("\tspmod8 %d\n", size);
		} else {
			printf("\tspmod %d\n", size);
		}
	}
	printf("\texit\n");
}

void gen_label(const char *tail, unsigned n)
{
	printf("L%d%s:\n", n, tail);
}

void gen_jump(const char *tail, unsigned n)
{
	printf("\tjump L%d%s\n", n, tail);
}

void gen_jfalse(const char *tail, unsigned n)
{
	printf("\tjfalse L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	printf("\tjtrue L%d%s\n", n, tail);
}

void gen_switch(unsigned n, unsigned type)
{
	gen_helpcall(NULL);
	printf("switch");
	helper_type(type, 0);
	printf(" Sw%d\n", n);
}

void gen_switchdata(unsigned n, unsigned size)
{
	printf("Sw%d:\n", n);
	printf("\t.word %d\n", size);
}

void gen_case(unsigned tag, unsigned entry)
{
	printf("Sw%d_%d:\n", tag, entry);
}

void gen_case_label(unsigned tag, unsigned entry)
{
	printf("Sw%d_%d:\n", tag, entry);
}

void gen_case_data(unsigned tag, unsigned entry)
{
	printf("\t.word Sw%d_%d\n", tag, entry);
}

void gen_helpcall(struct node *n)
{
	printf("\t");
}

void gen_helpclean(struct node *n)
{
}

void gen_data_label(const char *name, unsigned align)
{
	printf("_%s:\n", name);
}

void gen_space(unsigned value)
{
	printf("\tds %d\n", value);
}

void gen_text_data(unsigned n)
{
	printf("\t.word T%d\n", n);
}

void gen_literal(unsigned n)
{
	if (n)
		printf("T%d:\n", n);
}

void gen_name(struct node *n)
{
	printf("\t.word _%s+%d\n", namestr(n->snum), WORD(n->value));
}

void gen_value(unsigned type, unsigned long value)
{
	if (PTR(type)) {
		printf("\t.word %u\n", (unsigned) value);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		printf("\t.byte %u\n", (unsigned) value & 0xFF);
		break;
	case CSHORT:
	case USHORT:
		printf("\t.word %d\n", (unsigned) value & 0xFFFF);
		break;
	case CLONG:
	case ULONG:
	case FLOAT:
		/* We are little endian */
		printf("\t.word %d\n", (unsigned) (value & 0xFFFF));
		printf("\t.word %d\n", (unsigned) ((value >> 16) & 0xFFFF));
		break;
	default:
		error("unsuported type");
	}
}

void gen_start(void)
{
	printf("\t.code\n");
}

void gen_end(void)
{
}

void gen_tree(struct node *n)
{
	codegen_lr(n);
	printf(";\n");
}

/*
 *	Example size handling. In this case for a system that always
 *	pushes words.
 */
static unsigned get_size(unsigned t)
{
	if (PTR(t))
		return 2;
	if (t == CSHORT || t == USHORT)
		return 2;
	if (t == CCHAR || t == UCHAR)
		return 1;
	if (t == CLONG || t == ULONG || t == FLOAT)
		return 4;
	if (t == CLONGLONG || t == ULONGLONG || t == DOUBLE)
		return 8;
	if (t == VOID)
		return 0;
	error("gs");
	return 0;
}

static unsigned get_stack_size(unsigned t)
{
	unsigned n = get_size(t);
	if (n == 1)
		return 2;
	return n;
}

unsigned gen_push(struct node *n)
{
	/* Our push will put the object on the stack, so account for it */
	/* Can't use helper as we might be doing a push of the result of
	   a cast */
	printf("\tpush");
	helper_type(n->type, 0);
	printf("\n");
	sp += get_stack_size(n->type);
	return 1;
}

/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 */
unsigned gen_direct(struct node *n)
{
	unsigned v;
	switch(n->op) {
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	case T_CLEANUP:
		v = n->right->value;
		if (v < 256)
			printf("\tspmod8 %d\n", v);
		else
			printf("\tspmod %d\n", v);
		sp -= v;
		return 1;
	}
	return 0;
}

/*
 *	Allow the code generator to shortcut the generation of the argument
 *	of a single argument operator (for example to shortcut constant cases
 *	or simple name loads that can be done better directly)
 */
unsigned gen_uni_direct(struct node *n)
{
	return 0;
}

/*
 *	Allow the code generator to shortcut trees it knows
 */
unsigned gen_shortcut(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	/* The comma operator discards the result of the left side, then
	   evaluates the right. Avoid pushing/popping and generating stuff
	   that is surplus */
	if (n->op == T_COMMA) {
		l->flags |= NORETURN;
		codegen_lr(l);
		/* Parent determines child node requirements */
		r->flags |= (n->flags & NORETURN);
		codegen_lr(r);
		return 1;
	}
	return 0;
}

/* TOS is the lval ptr a the amount */

/* eqop preserves a whilst stacking the value pointed to by tos (without
   a pull.. So we end up with
              lval, existing value, a = modifier
   after the evaluation we get
              lval,  a = result
   and are ready for the eq op */
static void gen_eqop(struct node *n, const char *op)
{
	printf("\tdup\n");
	printf("\tswap\n");	/* A is now lval copy, TOS is modifier */
	helper_s(n, "deref");		/* A is now val, TOS is modifier */
	printf("\tswap\n");	/* A is back as it should be */
	helper(n, op);
	helper(n, "assign");
}

static void gen_eqop_s(struct node *n, const char *op)
{
	printf("\tdup");
	printf("\tswap");	/* A is now lval copy, TOS is modifier */
	helper_s(n, "deref");		/* A is now val, TOS is modifier */
	printf("\tswap");	/* A is back as it should be */
	helper_s(n, op);
	helper(n, "assign");
}

static unsigned gen_cast(struct node *n)
{
	unsigned lt = n->type;
	unsigned rt = n->right->type;
	unsigned ls;
	unsigned rs;

	if (PTR(rt))
		rt = USHORT;
	if (PTR(lt))
		lt = USHORT;

	/* Floats and stuff handled by helper */
	if (!IS_INTARITH(lt) || !IS_INTARITH(rt))
		return 0;

	ls = get_size(lt);
	rs = get_size(lt);

	/* Size shrink is free */
	if (ls < rs)
		return 1;
	if (rt & UNSIGNED) {
		/* Expanding to 4 from 2 or 1 */
		if (ls == 4) {
			printf("\tpushl\n");
			if (rs == 2)
				printf("\tconstl 0xFFFF,0\n");
			else
				printf("\tconstl 0xFF,0\n");
			printf("\tbandl\n");
		} else {
			printf("\tbyteu\n");
		}
		return 1;
	}
	/* Signed has actual helper ops */
	if (rs == 1)
		printf("\tsex\n");
	if (ls == 4)
		printf("\tsexl\n");
	return 1;
}

/* rightop - similar but for ++ and -- forms, alwaus constant right */
/* These are common so are probably good bytecode candidates, ditto the
   constant other forms */
static void gen_rightop(struct node *n, const char *op, const char *revop)
{
	struct node *r = n->right;
	unsigned sz = get_size(n->type);
	/* Generate the op */
	/* Right now TOS is lval ptr, a is amount */
	/* TODO */
}

unsigned gen_node(struct node *n)
{
	unsigned v = n->value;
	unsigned nr = n->flags & NORETURN;

	/* Function call arguments are special - they are removed by the
	   act of call/return and reported via T_CLEANUP */
	if (n->left && n->op != T_ARGCOMMA && n->op != T_FUNCCALL && n->op != T_CALLNAME)
		sp -= get_stack_size(n->left->type);

	switch(n->op) {
	case T_NREF:
		printf("\tcpush _%s+%d\n", namestr(n->snum), v);
		helper_s(n, "deref");
		return 1;
	case T_LBREF:
		printf("\tcpush T%d+%d\n", n->val2, v);
		helper_s(n, "deref");
		return 1;
	case T_LREF:
		if (nr)
			return 1;
		if (v < 256)
			printf("\tloadl8 %d\n", v);
		else
			printf("\tload %d\n", v);
		helper_s(n, "deref");
		return 1;
	case T_NSTORE:
		printf("\tcpush _%s+%d\n", namestr(n->snum), v);
		helper(n, "assign");
		return 1;
	case T_LBSTORE:
		printf("\tcpush T%ds+%d\n", n->val2, v);
		helper(n, "assign");
		return 1;
	case T_LSTORE:
		if (nr)
			return 1;
		if (v < 256)
			printf("\tloadl8 %d\n", v);
		else
			printf("\tloadl %d\n", v);
		helper(n, "assign");
		return 1;
	case T_CALLNAME:
		printf("\tcall _%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_ARGUMENT:
		/* Turn argument into local effectively */
		v = WORD(n->value + frame_len + sp);
		if (v < 256)
			printf("\tloadl8 %d\n", v);
		else
			printf("\tloadl %d\n", v);
		return 1;
	case T_LOCAL:
		/* Adjust offsets of convenience */
		v = WORD(n->value + sp);
		if (v < 256)
			printf("\tloadl8 %d\n", v);
		else
			printf("\tloadl %d\n", v);
		return 1;
	/* Ones we don't implement directly for cleanness of vm */
	/* LVAL on tos, A is amount */
	case T_SHLEQ:
		gen_eqop(n, "shl");
		return 1;
	case T_SHREQ:
		gen_eqop_s(n, "shr");
		return 1;
	case T_PLUSEQ:
		gen_eqop(n, "plus");
		return 1;
	case T_MINUSEQ:
		gen_eqop(n, "minus");
		return 1;
	case T_SLASHEQ:
		gen_eqop_s(n, "div");
		return 1;
	case T_STAREQ:
		gen_eqop(n, "mul");
		return 1;
	case T_HATEQ:
		gen_eqop(n, "xor");
		return 1;
	case T_OREQ:
		gen_eqop(n, "or");
		return 1;
	case T_ANDEQ:
		gen_eqop(n, "band");
		return 1;
	case T_PERCENTEQ:
		gen_eqop_s(n, "mod");
		return 1;
	/* These two are a bit special as the old value is left in a */
	/* TODO: generate *eq ops if noreturn */
	case T_PLUSPLUS:
		if (n->flags & NORETURN) {
			gen_eqop(n, "plus");
			return 1;
		}
		gen_rightop(n, "plus", "minus");
		return 1;
	case T_MINUSMINUS:
		if (n->flags & NORETURN) {
			gen_eqop(n, "minus");
			return 1;
		}
		gen_rightop(n, "minus", "plus");
		return 1;
	case T_NAME:
		/* Turn name into const */
		printf("\tconst _%s+%d\n",
			namestr(n->snum), v);
		return 1;
	case T_LABEL:
		/* Turn name into const */
		printf("\tconst T%d+%d\n",
			n->val2, v);
		return 1;
	case T_DEREF:
		/* Due to the way our ops work deref is signed */
		helper_s(n, "deref");
		return 1;
	case T_CAST:
		gen_cast(n);
		return 1;
	case T_CONSTANT:
		printf("\tconst");
		helper_type(n->type, 1);
		if (get_size(n->type) == 4)
			printf(" %d,%d\n", WORD(v), WORD(n->value >> 16));
		else
			printf(" %d\n", v);
		return 1;
	case T_LTEQ:
		printf("\tccgt\nnot\n");
		return 1;
	case T_GTEQ:
		printf("\tcclt\nnot\n");
		return 1;
	case T_BANGEQ:
		printf("\tcceq\nnot\n");
		return 1;
	case T_TILDE:
		if (get_size(n->type) == 4)
			printf("\tcpush 0xFFFF\n\tdup\n\txorl\n");
		else
			printf("\tcpush 0xFFFF\n\txor\n");
		return 1;
	case T_FUNCCALL:
		printf("\tcallfunc\n");
		return 1;
	}
	return 0;
}
