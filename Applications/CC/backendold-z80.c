#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "backend.h"

#define BYTE(x)		(((unsigned)(x)) & 0xFF)
#define WORD(x)		(((unsigned)(x)) & 0xFFFF)

/*
 *	State for the current function
 */
static unsigned frame_len;	/* Number of bytes of stack frame */
static unsigned sp;		/* Stack pointer offset tracking */

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


#define T_NREF		(T_USER)
#define T_CALLNAME	(T_USER+1)
#define T_NSTORE	(T_USER+2)
#define T_LREF		(T_USER+3)
#define T_LSTORE	(T_USER+4)

static void squash_node(struct node *n, struct node *o)
{
	n->value = o->value;
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
 *	Our chance to do tree rewriting. We don't do much for the Z80
 *	at this point, but we do rewrite name references and function calls
 *	to make them easier to process.
 */
struct node *gen_rewrite_node(struct node *n)
{
	struct node *l = n->left;
	struct node *r = n->right;
	/* Rewrite references into a load operation */
	if (n->type == CSHORT || n->type == USHORT || PTR(n->type)) {
		if (n->op == T_DEREF) {
			if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
				if (r->op == T_ARGUMENT)
					r->value += 2 + frame_len;
				squash_right(n, T_LREF);
				return n;
			}
			if (r->op == T_NAME) {
				squash_right(n, T_NREF);
				return n;
			}
		}
		if (n->op == T_EQ) {
			if (l->op == T_NAME) {
				squash_left(n, T_NSTORE);
				return n;
			}
			if (l->op == T_LOCAL || l->op == T_ARGUMENT) {
				if (l->op == T_ARGUMENT)
					l->value += 2 + frame_len;
				squash_left(n, T_LSTORE);
				return n;
			}
		}
	}
	/* Rewrite function call of a name into a new node so we can
	   turn it easily into call xyz */
	if (n->op == T_FUNCCALL && r->op == T_NAME) {
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

void gen_segment(unsigned segment)
{
	switch(segment) {
	case A_CODE:
		printf("\t.code\n");
		break;
	case A_DATA:
	case A_LITERAL:
		printf("\t.data\n");
		break;
	case A_BSS:
		printf("\t.bss\n");
		break;
	default:
		error("gseg");
	}
}

/* Generate the function prologue - may want to defer this until
   gen_frame for the most part */
void gen_prologue(const char *name)
{
	printf("_%s:\n", name);
}

/* Generate the stack frame */
/* TODO: defer this to statements so we can ld/push initializers */
void gen_frame(unsigned size)
{
	frame_len = size;
	/* So ix is always our frame pointer */
	printf("\tpush ix\n");
	printf("\tld ix,%d\n", -size);
	printf("\tadd ix,sp\n");
	printf("\tld sp,ix\n");
	sp = 0;
}

void gen_epilogue(unsigned size)
{
	if (sp != 0)
		error("sp");
	sp -= size;

	if (size > 16) {
		printf("\tex de,hl\n");
		printf("\tld hl,0x%x\n", (uint16_t) - size);
		printf("\tadd hl,sp\n");
		printf("\tld sp,hl\n");
		printf("\tpop hl\n");
		printf("\tret\n");
		printf("\tex de,hl\n");
		return;
	}
	if (size % 1) {
		printf("\tinc sp\n");
		size--;
	}
	while (size) {
		printf("\tpop bc\n");
		size -= 2;
	}
	printf("\tpop ix\n");
	printf("\tret\n");
}

void gen_label(const char *tail, unsigned n)
{
	printf("L%d%s:\n", n, tail);
}

void gen_jump(const char *tail, unsigned n)
{
	printf("\tjr L%d%s\n", n, tail);
}

void gen_jfalse(const char *tail, unsigned n)
{
	printf("\tjr z,L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	printf("\tjr nz,L%d%s\n", n, tail);
}

static void gen_cleanup(unsigned v)
{
	/* CLEANUP is special and needs to be handled directly */
	sp -= v;
	if (v > 10) {
		printf("\tex de,hl\n");
		printf("\tld hl, %d\n", -v);
		printf("\tadd hl, sp\n");
		printf("\tld sp,hl\n");
		printf("\tex de,hl\n");
	} else {
		while(v >= 2) {
			printf("\tpop hl\n");
			v -= 2;
		}
		if (v)
			printf("\tdec sp\n");
	}
	/* The call return is in DE at this point due to stack juggles
	   so put it into HL */
	printf("\tex de,hl\n");
}

/*
 *	Helper handlers. We use a tight format for integers but C
 *	style for float as we'll have C coded float support if any
 */
void gen_helpcall(struct node *n)
{
	if (n->type == FLOAT)
		gen_push(n->right);
	printf("\tcall __");
}

void gen_helpclean(struct node *n)
{
	unsigned s;

	if (n->type != FLOAT)
		return;

	s = 0;
	if (n->left) {
		s += get_size(n->left->type);
		/* gen_node already accounted for removing this thinking
		   the helper did the work, adjust it back as we didn't */
		sp += s;
	}
	s += get_size(n->right->type);
	gen_cleanup(s);
}

void gen_switch(unsigned n, unsigned type)
{
	printf("\tld de, Sw%d\n", n);
	printf("\tcall __switch");
	helper_type(type, 0);
	printf("\n");
}

void gen_switchdata(unsigned n, unsigned size)
{
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

void gen_data_label(const char *name, unsigned align)
{
	printf("_%s:\n", name);
}

void gen_space(unsigned value)
{
	printf("\t.ds %d\n", value);
}

void gen_text_data(unsigned n)
{
	printf("\t.word T%d\n", n);
}

/* The label for a literal (currently only strings)
   TODO: if we add other literals we may need alignment here */

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
	unsigned w = WORD(value);
	if (PTR(type)) {
		printf("\t.word %u\n", w);
		return;
	}
	switch (type) {
	case CCHAR:
	case UCHAR:
		printf("\t.byte %u\n", BYTE(w));
		break;
	case CSHORT:
	case USHORT:
		printf("\t.word %d\n", w);
		break;
	case CLONG:
	case ULONG:
	case FLOAT:
		/* We are little endian */
		printf("\t.word %d\n", w);
		printf("\t.word %d\n", (unsigned) ((value >> 16) & 0xFFFF));
		break;
	default:
		error("unsuported type");
	}
}

void gen_start(void)
{
	printf("\t.setcpu z%d\n", cpu);
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
 *	Try and generate shorter code for stuff we can directly access
 */

/*
 *	Return 1 if the node can be turned into direct access. The VOID check
 *	is a special case we need to handle stack clean up of void functions.
 */
static unsigned access_direct(struct node *n)
{
	int off = n->value;
	/* We can direct access integer or smaller types that are constants
	   global/static or string labels */
	if (n->op != T_CONSTANT && n->op != T_NAME && n->op != T_LABEL && n->op != T_NREF && n->op != T_LREF)
		return 0;
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;
	if (n->op == T_LREF && (off > 126 || off < -128))
		return 0;
	return 1;
}

/*
 *	Get something that passed the access_direct check into de. Could
 *	we merge this with the similar hl one in the main table ?
 */

static unsigned load_r_with(const char *r, struct node *n)
{
	unsigned v = WORD(n->value);
	int vs = v;
	const char *name;

	switch(n->op) {
	case T_NAME:
		printf("\tld %s, _%s+%d\n", r, namestr(n->snum), v);
		return 1;
	case T_LABEL:
		printf("\tld %s, T%d\n", r, v);
		return 1;
	case T_CONSTANT:
		/* We know this is not a long from the checks above */
		printf("\tld %s, %d\n", r, v);
		return 1;
	case T_NREF:
		name = namestr(n->snum);
		printf("\tld %s,(_%s+%d)\n", r, name, v);
		return 1;
	case T_LREF:
		if (vs >= -128 && vs < 126) {
			printf("\tld %c,(ix + %d)\n", *r, vs + 1);
			printf("\tld %c,(ix + %d)\n", r[1], vs);
			return 1;
		}
	default:
		return 0;
	}
	return 1;
}

static unsigned load_bc_with(struct node *n)
{
	return load_r_with("bc", n);
}

static unsigned load_de_with(struct node *n)
{
	return load_r_with("de", n);
}

static unsigned load_hl_with(struct node *n)
{
	return load_r_with("hl", n);
}

static unsigned load_a_with(struct node *n)
{
	int v = n->value;
	switch(n->op) {
	case T_CONSTANT:
		/* We know this is not a long from the checks above */
		printf("\tld a,%d\n", BYTE(n->value));
		break;
	case T_NREF:
		printf("\tld a,(_%s+%d)\n", namestr(n->snum), v);
		break;
	case T_LREF:
		if (v >= -128 && v <= 127)
			printf("\tld a,(ix+%d)\n", v);
		break;
	default:
		return 0;
	}
	return 1;
}

static unsigned gen_compc(const char *op, struct node *n, struct node *r)
{
	unsigned s = get_size(n->type);
	if (r->op == T_CONSTANT && r->value == 0) {
		char buf[10];
		strcpy(buf, op);
		strcat(buf, "0");
		helper(n, buf);
		return 1;
	}
	if (s == 2) {
		if (load_de_with(r) == 0)
			return 0;
		helper(n, op);
		return 1;
	}
	if (s == 1) {
		if (load_a_with(r) == 0)
			return 0;
		helper(n, op);
		return 1;
	}
	return 0;
}

/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 *
 *	The Z80 is pretty basic so there isn't a lot we turn around here. As
 *	proof of concept we deal with the add case. Other processors may be
 *	able to handle a lot more.
 *
 *	If your processor is good at subtracts you may also want to rewrite
 *	constant on the left subtracts in the rewrite rules into some kind of
 *	rsub operator.
 */
unsigned gen_direct(struct node *n)
{
	unsigned s = get_size(n->type);
	struct node *r = n->right;
	unsigned v;

	if (r)
		v = r->value;

	switch (n->op) {
	case T_PLUSPLUS:
		/* Left (ie HL) is address of object */
		if (s == 1) {
			printf("\tld a,(hl)\n");
			printf("\tadd a,%d\n", BYTE(v));
			printf("\tld (hl),a\n");
			if (!(n->flags & NORETURN))
				printf("\tld l,a\n");
			return 1;
		}
		if (s == 2) {
			printf("\tld de,%d\n", v);
			helper(n, "cpostinc");
			return 1;
		}
		return 0;
	case T_MINUSMINUS:
		/* Left (ie HL) is address of object */
		if (s == 1) {
			printf("\tld a,(hl)\n");
			printf("\tsub a,%d\n", BYTE(v));
			printf("\tld (hl),a\n");
			if (!(n->flags & NORETURN))
				printf("\tld l,a\n");
			return 1;
		}
		if (s == 2) {
			printf("\tld de,%d\n", -v);
			helper(n, "__cpostinc");
			return 1;
		}
		return 0;
	}

	/* We only deal with simple cases for now */
	if (r && !access_direct(r))
		return 0;

	switch (n->op) {
	case T_CLEANUP:
		gen_cleanup(v);
		return 1;
	case T_NSTORE:
		if (s == 1) {
			printf("\tld a,l\n");
			printf("\tld (_%s+%d), a\n", namestr(n->snum), WORD(n->value));
			return 1;
		}
		if (s == 2) {
			printf("\tld (_%s+%d), hl\n", namestr(n->snum), WORD(n->value));
			return 1;
		}
		/* TODO 4/8 for long etc */
		return 0;
	case T_EQ:
		/* The address is in HL at this point */
		if (s == 2 ) {
			if (load_de_with(r) == 0)
				return 0;
			printf("\tld (hl),e\n");
			printf("\tinc hl\n");
			printf("\tld (hl),e\n");
			/* FIXME: can eliminate this in many cases - need to
			   add this to 8080 tree so we pass value right */
			if (!(n->flags & NORETURN))
				printf("\tex de,hl\n");
			return 1;
		}
		if (s == 1) {
			if (load_a_with(r) == 0)
				return 0;
			printf("\tld a,(hl)\n");
			/* FIXME: add to 8080 tree, eliminate on noresult */
			if (!(n->flags & NORETURN))
				printf("\tld l,a\n");
			return 1;
		}
		return 0;
	case T_PLUS:
		if (s <= 2) {
			/* LHS is in HL at the moment, end up with the result in HL */
			if (s == 1) {
				if (load_a_with(r) == 0)
					return 0;
				printf("\tld e,a\n");
			}
			if (s > 2 || load_de_with(r) == 0)
				return 0;
			printf("\tadd hl,de\n");
			return 1;
		}
		return 0;
	case T_MINUS:
		if (s <= 2) {
			/* LHS is in HL at the moment, end up with the result in HL */
			if (s == 1) {
				if (load_a_with(r) == 0)
					return 0;
				printf("\tld e,a\n");
			}
			if (s > 2 || load_de_with(r) == 0)
				return 0;
			printf("\tor a,a\n\tsbc hl,de\n");
			return 1;
		}
		return 0;
	case T_EQEQ:
		return gen_compc("cmpeq", n, r);
	case T_GTEQ:
		return gen_compc("cmpgteq", n, r);
	case T_GT:
		return gen_compc("cmpgt", n, r);
	case T_LTEQ:
		return gen_compc("cmplteq", n, r);
	case T_LT:
		return gen_compc("cmplt", n, r);
	case T_BANGEQ:
		return gen_compc("cmpne", n, r);
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
 *	Allow the code generator to short cut any subtrees it can directly
 *	generate.
 */
unsigned gen_shortcut(struct node *n)
{
	struct node *l = n->left;
	switch(n->op) {
	case T_COMMA:
		/* The comma operator discards the result of the left side, then
		   evaluates the right. Avoid pushing/popping and generating stuff
		   that is surplus */
		n->left->flags |= NORETURN;
		codegen_lr(n->left);
		codegen_lr(n->right);
		return 1;
	case T_PLUS:
		printf(";plus direct access l says %d\n", access_direct(l));
		if (access_direct(l)) {
			codegen_lr(n->right);
			load_de_with(l);
			printf("\tadd hl,de\n");
			return 1;
		}
		printf(";plus direct access r says %d\n", access_direct(n->right));
		if (access_direct(l)) {
			codegen_lr(l);
			load_de_with(n->right);
			printf("\tadd hl,de\n");
			return 1;
		}
		break;
	}
	return 0;
}

/* Stack the node which is currently in the working register */
unsigned gen_push(struct node *n)
{
	unsigned size = get_stack_size(n->type);

	/* Our push will put the object on the stack, so account for it */
	sp += size;

	switch(size) {
	case 2:
		printf("\tpush hl\n");
		return 1;
	case 4:
		printf("ld de,(hireg)\n\tpush de\n\tpush hl\n");
		return 1;
	default:
		return 0;
	}
}

static unsigned gen_cast(struct node *n)
{
	unsigned lt = n->type;
	unsigned rt = n->right->type;
	unsigned rs;
	unsigned ls;

	if (PTR(rt))
		rt = CSHORT;
	if (PTR(lt))
		lt = CSHORT;

	/* Floats and stuff handled by helper */
	if (!IS_INTARITH(lt) || !IS_INTARITH(rt))
		return 0;

	rs = get_size(rt);
	ls = get_size(lt);

	/* Size shrink is free */
	if ((ls & ~UNSIGNED) <= (rs & ~UNSIGNED))
		return 1;
	/* Don't do the harder ones */
	if (!(rs & UNSIGNED) || rs > 2)
		return 0;
	printf("\tld h,0\n");
	return 1;
}

unsigned gen_node(struct node *n)
{
	unsigned size = get_size(n->type);
	unsigned v;
	/* We adjust sp so track the pre-adjustment one too when we need it */
	unsigned spval = sp;

	v = n->value;

	/* An operation with a left hand node will have the left stacked
	   and the operation will consume it so adjust the stack.

	   The exception to this is comma and the function call nodes
	   as we leave the arguments pushed for the function call */

	if (n->left && n->op != T_ARGCOMMA && n->op != T_CALLNAME && n->op != T_FUNCCALL)
		sp -= get_stack_size(n->left->type);

	switch (n->op) {
		/* Load from a name */
	case T_NREF: /* NREF/STORE FIXME for double */
		if (size == 1) {
			printf("\tld a,(_%s+%d)\n", namestr(n->snum), v);
			printf("\tld l,a\n");
			return 1;
		} else if (size == 2) {
			printf("\tld hl,(_%s+%d)\n", namestr(n->snum), v);
			return 1;
		} else if (size == 4) {
			printf("\tld hl,(_%s+%d)\n", namestr(n->snum), v + 2);
			printf("\tld (__hireg),hl\n");
			printf("\tld hl,(_%s+%d)\n", namestr(n->snum), v);
			return 1;
		}
		break;
	case T_LREF:
		/* TODO long/float etc */
		if (v + spval == 0 && size == 2) {
			printf("\tpop hl\n\tpush hl\n");
			return 1;
		}
		if (v < 127 && size <= 2) {
			printf("\tld l, (ix + %d)\n", v);
			if (size == 2)
				printf("\tld h, (ix + %d)\n", v + 1);
			return 1;
		}
		printf("\tcall __lrefw\n\t.word %d\n", v);
		return 1;
	case T_NSTORE:
		if (size > 2)
			return 0;
		if (size == 1)
			printf("\tld a,l\n\tld (_%s+%d),a\n", namestr(n->snum), v);
		else
			printf("\tld (_%s+%d),hl\n", namestr(n->snum), v);
		return 1;
	case T_LSTORE:
		if (v + spval == 0 && size == 2 ) {
			printf("\tpop af\n\tpush hl\n");
			return 1;
		}
		v += spval;
		if (v < 127 && size <= 2) {
			printf("\tld (ix + %d),l\n", v);
			if (size == 2)
				printf("\tld (ix + %d),h\n", v + 1);
			return 1;
		}
		/* Via helper magic for compactness */
		printf("\tcall __lstorew\n\t.word %d\n", v);
		return 1;
		/* Call a function by name */
	case T_CALLNAME:
		printf("\tcall _%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_EQ:
		if (size == 2) {
			/* FIXME: we can drop the final exchange in some cases */
			/* Ditto push into 8080 tree */
			if (cpu == 8085)
				printf("\tex de,hl\n\tpop h\n\tshlx\n");
			else
				printf("\tex de,hl\n\tpop hl\n\tld (hl),e\n\tinc hl\n\tld (hl),d\n\tex de,hl\n");
			return 1;
		}
		if (size == 1) {
			printf("\tpop de\n\tld a,l\n\tld (de),a\n");
			return 1;
		}
		break;
	case T_DEREF:
		if (size == 2) {
			printf("\tld e,(hl)\n\tinc hl\n\tld d,(hl)\n\tex de,hl\n");
			return 1;
		}
		if (size == 1) {
			printf("\tld l,(hl)\n");
			return 1;
		}
		break;
	case T_FUNCCALL:
		printf("\tcall callhl\n");
		return 1;
	case T_LABEL:
		/* Used for const strings */
		printf("\tld hl,T%d\n", v);
		return 1;
	case T_CONSTANT:
		switch(size) {
		case 4:
			printf("ld hl,%u\n", ((v >> 16) & 0xFFFF));
			printf("ld (hireg),hl\n");
		case 2:
			printf("\tld hl,%d\n", (v & 0xFFFF));
			return 1;
		case 1:
			printf("\tld l,%d\n", (v & 0xFF));
			return 1;
		}
		break;
	case T_NAME:
		printf("\tld hl,");
		printf("_%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_LOCAL:
		/* We already adjusted sp so allow for this */
		printf("\tld hl,%d\n", v + spval + size);
		printf("\tadd hl,sp\n");
		return 1;
	case T_ARGUMENT:
		/* We already adjusted sp so allow for this */
		printf("\tld hl,%d\n", v + size + frame_len + spval);
		printf("\tadd hl,sp\n");
		return 1;
	case T_CAST:
		return gen_cast(n);
	}
	return 0;
}
