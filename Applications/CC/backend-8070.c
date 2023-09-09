#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "compiler.h"
#include "backend.h"

#define ARGBASE		2

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
	fprintf(stderr, "type %x\n", t);
	error("gs");
	return 0;
}

/* We work in words on stack not because of alignment or sizing but
   because we have no easy "throw a byte of stack without mashing EA */
static unsigned get_stack_size(unsigned t)
{
	unsigned sz = get_size(t);
	if (sz == 1)
		return 2;
	return sz;
}

/*
 *	Register tracking (not yet done)
 */
 
void invalidate_ea(void)
{
}

void set_ea(unsigned n)
{
}

void set_a(uint8_t n)
{
}

void set_ea_node(struct node *n)
{
}

void adjust_a(unsigned n)
{
}

void adjust_ea(unsigned n)
{
}

void set_t(unsigned n)
{
}

void invalidate_t(void)
{
}

void flush_writeback(void)
{
}

void invalidate_all(void)
{
}

unsigned free_pointer_notwith(unsigned p)
{
	/* Dummy up for now */
	if (p == 3)
		return 2;
	return 3;
}

unsigned free_pointer(void)
{
	return 3;
}

unsigned find_ref(struct node *n, unsigned notwith, int *offset)
{
	return 0;
}

void set_ptr_ref(unsigned p, struct node *n)
{
}

/*
 *	Rewriting
 */
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
 *	Heuristic for guessing what to put on the right. This is very
 *	processor dependent. For 8070 we are quite limited especially
 *	with static/global.
 */

static unsigned is_simple(struct node *n)
{
	unsigned op = n->op;

	/* Multi-word objects are never simple */
	if (!PTR(n->type) && (n->type & ~UNSIGNED) > CSHORT)
		return 0;

	/* We can load these directly into a register */
	if (op == T_CONSTANT || op == T_LABEL || op == T_NAME)
		return 10;
	/* We can load this directly into a register but may need a
	   pointer register */
	if (op == T_NREF || op == T_LBREF)
		return 1;
	return 0;
}

/*
 *	Our chance to do tree rewriting. We don't do much for the 8070
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
	if (nt == CSHORT || nt == USHORT || PTR(nt)) {
		if (op == T_DEREF) {
			if (r->op == T_LOCAL || r->op == T_ARGUMENT) {
				if (r->op == T_ARGUMENT)
					r->value += ARGBASE + frame_len;
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
					l->value += ARGBASE + frame_len;
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
	/* Commutive operations. We can swap the sides over on these */
	if (op == T_AND || op == T_OR || op == T_HAT || op == T_STAR || op == T_PLUS) {
/*		printf(";left %d right %d\n", is_simple(n->left), is_simple(n->right)); */
		if (is_simple(n->left) > is_simple(n->right)) {
			n->right = l;
			n->left = r;
		}
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
		printf("\t.text\n");
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
}

/* Generate the stack frame */
void gen_frame(unsigned size)
{
	/* Annoyingly we can't use autoindex on SP */
	frame_len = size;
	sp = 0;

	if (size > 10) {
		printf("\tld ea,sp\n\tsub ea,=%d\n\tld sp,ea\n", size);
		return;
	}
	while(size) {
		puts("\tpush ea\n");
		size -= 2;
	}
}

void gen_epilogue(unsigned size)
{
	if (sp != size) {
		error("sp");
	}
	sp -= size;
	if (size > 10)
		printf("\tld ea,sp\n\tadd ea,=%d\n\tld sp,ea\n", size);
	else while(size) {
		puts("\tpop ea\n");
		size -= 2;
	}
	printf("\tret\n");
}

void gen_label(const char *tail, unsigned n)
{
	invalidate_all();
	printf("L%d%s:\n", n, tail);
}

void gen_jump(const char *tail, unsigned n)
{
	flush_writeback();
	printf("\tjmp L%d%s\n", n, tail);
}

void gen_jfalse(const char *tail, unsigned n)
{
	flush_writeback();
	printf("\tjz L%d%s\n", n, tail);
}

void gen_jtrue(const char *tail, unsigned n)
{
	flush_writeback();
	printf("\tjnz L%d%s\n", n, tail);
}

void gen_switch(unsigned n, unsigned type)
{
	flush_writeback();
	invalidate_all();
	printf("\tld p3, Sw%d\n", n);
	printf("\tjmp __switch");
	helper_type(type, 0);
	putchar('\n');
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
	flush_writeback();
	invalidate_all();
	printf("\tjsr __");
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
	printf("\t.ds %d\n", value);
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

unsigned gen_push(struct node *n)
{
	/* Our push will put the object on the stack, so account for it */
	unsigned size = get_stack_size(n->type);
	sp += size;
	switch(size) {
	case 2:
		puts("\tpush ea\n");
		break;
	case 4:
		puts("\tld t,ea\n\tld ea,@__high\n\tpush ea\n\tld ea,t\n\tpush ea\n");
		break;
	default:
		return 0;
	}
	return 1;
}


unsigned load_ptr_ea(void)
{
	/* FIXME: set ptr up according to ea name value in future */
	unsigned ptr = free_pointer_notwith(0);
	printf("\tld p%d, ea\n", ptr);
	return ptr;
}

void load_ea(unsigned sz, unsigned long v)
{
	if (sz == 1) {
		printf("\tld a,=%ld\n", v & 0xFF);
		set_a(v & 0xFF);
	}
	else if (sz == 2) {
		printf("\tld ea,=%ld\n", v & 0xFFFF);
		set_ea(v & 0xFFFF);
	} else {
		printf("\tld ea,=%ld\n", v >> 16);
		puts("\tst ea,@__high");
		printf("\tld ea,=%ld\n", v & 0xFFFF);
		set_ea(v & 0xFFFF);
	}
}

void load_t(unsigned v)
{
	printf("\tld t,=%d\n", v & 0xFFFF);
	set_t(v & 0xFFFF);
}


void repeated_op(const char *op, unsigned n)
{
	while(n--)
		puts(op);
}

void discard_word(void)
{
	unsigned ptr = free_pointer();
	printf("\tpop p%d\n", ptr);
}

/* FIXME: pass sz */
/*
 *	Make a reference to an object. We are passed the object node
 *	an optional 1 or 2 to indicate a pointer to avoid, and an
 *	optional offset return.
 */
unsigned gen_ref_notwith(struct node *n, unsigned notwith, int *off)
{
	unsigned sz = get_size(n->type);
	unsigned ptr;
	unsigned v = n->value;

	if (off)
		*off = 0;
	if (n->op == T_LREF || n->op == T_LSTORE) {
		int r = stack_offset(v);
		/* Slightly pessimal for word ops */
		if (r >= -128 && r <= 128 - sz && off) {
			*off = r;
			return 3;
		}
		/* Need to generate a ref. TODO pass whether EA can be mushed */
		ptr = free_pointer_notwith(notwith);
		printf("\txch ea,p%d\n", ptr);
		puts("\tld ea,sp");
		printf("\tadd ea,%d\n", r);
		printf("\txch ea,p%d\n", ptr);
		return 3;
	}
	/* See if it is already accessible, often the case */
	ptr = find_ref(n, notwith, off);
	if (ptr)
		return ptr;
	/* Make a reference */
	ptr = free_pointer_notwith(notwith);
	set_ptr_ref(ptr, n);
	if (n->op == T_NREF || n->op == T_NSTORE) { 
		printf("\tld p%d,=_%s+%d\n", ptr, namestr(n->snum), v);
		return ptr;
	}
	if (n->op == T_LBREF || n->op == T_LBSTORE) {
		printf("\tld p%d,T%d+%d\n", ptr, n->val2, v);
		return ptr;
	}
	fprintf(stderr, "Don't know how to gen_ref a %x\n",
		n->op);
	error("bad gen_ref");
	return 0;
}

unsigned gen_load_notwith(struct node *n, unsigned notwith)
{
	int off;
	unsigned sz = get_size(n->type);
	unsigned ptr;
	if (n->op == T_CONSTANT) {
		load_ea(sz, n->value);
		return 1;
	}
	ptr = gen_ref_notwith(n, notwith, &off);
	if (sz == 1)
		printf("\tld a,%d,p%d\n", off, ptr);
	else if (sz == 2)
		printf("\tld ea,%d,p%d\n", off, ptr);
	else {
		printf("\tld ea,%d,p%d\n", off + 2, ptr);
		puts("\tst ea,@__high");
		printf("\tld ea,%d,p%d\n", off, ptr);
	}
	set_ea_node(n);
	return 1;
}

unsigned gen_load(struct node *n)
{
	return gen_load_notwith(n, 0);
}

unsigned gen_load_notwith_t(struct node *n, unsigned notwith)
{
	int off;
	unsigned sz = get_size(n->type);
	unsigned ptr;
	if (n->op == T_CONSTANT) {
		load_t(n->value);
		return 1;
	}
	ptr = gen_ref_notwith(n, notwith, &off);
	/* We have to ref an extra byte */
	if (sz <= 2)
		printf("\tld t,%d,p%d\n", off, ptr);
	else	
		error("loadt4");
	return 1;
}

unsigned gen_load_t(struct node *n)
{
	return gen_load_notwith_t(n, 0);
}

unsigned gen_ref(struct node *n, int *off)
{
	return gen_ref_notwith(n, 0, off);
}


/* EA holds the left side (ptr) r is the value to handle. Do not use
   T as T is used by caller in postinc/dec usage */
unsigned do_preincdec(unsigned sz, struct node *n)
{
	struct node *r = n->right;
	unsigned ptr = load_ptr_ea();

	/* For now at least */
	if (sz > 2)
		return 0;

	gen_load_notwith(r, ptr);


	if (n->op == T_PLUSPLUS || n->op == T_PLUSEQ) {
		/* A or EA or high:EA now holds the data */
		/* Now add to 0,ptr */	
		if (sz == 1) {
			printf("\tadd a,0,p%d\nst a,0,p%d\n",
				ptr, ptr);
			return 1;
		}
		printf("\tadd ea,0,p%d\nst ea,0,p%d\n",
			ptr, ptr);
		set_ea_node(n);
		return 1;
	}
	/* Tricker as order matters */
	/* No load ptr,n,ptr */
	/* At this point ptr holds our address, ea is undefined */
	if (r->op == T_CONSTANT) {
		/* Negate first */
		load_ea(2, -r->value);
		/* Now add to 0,ptr */	
		if (sz == 1) {
			printf("\tadd a,0,p%d\nst a,0,p%d\n",
				ptr, ptr);
			return 1;
		}
		printf("\tadd ea,0,p%d\nst ea,0,p%d\n",
			ptr, ptr);
		set_ea_node(n);
		return 1;
	}
	set_ea_node(n);
	/* A -= B */
	if (sz == 1) {
		/* 8bit case is easier */
		printf("\txch a,e\n\tld a,0,p%d\n\tsub a,e\nst a,0,p%d\n",
			ptr, ptr);
		return 1;
	}
	printf("\tpush ea\n\tld ea,0,p%d\n\tsub ea,2(sp)\n\tst ea,0,p%d\n",
		ptr, ptr);
	discard_word();
	return 1;
}

/* Only 8 and 16 bit as 32 is complicated by the lack of adc/sbc stuff */
static void gen_op(unsigned sz, const char *op, struct node *r)
{
	unsigned ptr;
	int off;
	unsigned v = r->value;
	if (r->op == T_CONSTANT) {
		if (sz == 1)  {
			printf("\t%s a,=%d\n", op, v & 0xFF);
			set_a(v & 0xFF);
		} else {
			printf("\t%s ea,=%d\n", op, v & 0xFFFF);
			set_ea(v & 0xFFFF);
		}
		return;
	}
	ptr = gen_ref(r, &off);
	invalidate_ea();
	if (sz == 1)
		printf("\t%s a,%d,p%d\n", op, off, ptr);
	else
		printf("\t%s ea,%d,p%d\n", op, off, ptr);
}

/* Generate a JSR to one of the ,T helpers */
static unsigned gen_t_op(unsigned sz, struct node *n, const char *fn)
{
	flush_writeback();
	invalidate_all();
	printf("\tjsr %s%c", fn, "XbwXl"[sz]);
	if (n->type & UNSIGNED)
		putchar('u');
	putchar('\n');
	return 1;
}

/* TODO:
   For 8bit:
   	shifts if fastest, MPY if not
   For 16bit signed/unsigned
   	shifts if fastest, MPY if constant positive
   	left shift and MPY by constant >> 1 if constant even
   else helper
 */
   	
   	
static unsigned gen_fast_mult(unsigned sz, unsigned value)
{
	... // shifts

	invalidate_ea();
	if (sz == 1) {
		load_t(value);
		puts("\tmpy");
		return 1;
	}
	if (sz > 2)
		return 0;
	/* Constant on right is positive - can use mpy */
	if (!(value & 0x8000)) {
		load_t(value);
		puts("\tmpy");
		return 1;
	}
	/* Can't shift to avoid problem - use helper */
	if (value & 1)
		return 0;
	/* Shift to keep right side positive */
	puts("\tsl ea\n");
	load_t(value >> 1);
	puts("\tmpy");
	return 0;	
}

/*
 *	If possible turn this node into a direct access. We've already checked
 *	that the right hand side is suitable. If this returns 0 it will instead
 *	fall back to doing it stack based.
 *
 *	We can do an awful lot of things this way and also for the stuff
 *	we cannot do this way shortcut a bunch of stuff via T to avoid stack
 *	traffic.
 */
unsigned gen_direct(struct node *n)
{
	unsigned s = get_size(n->type);
	struct node *r = n->right;
	unsigned v;
	unsigned ptr, ptr2;
	int off;

	/* We only deal with simple cases for now */
	if (r) {
		if (!access_direct(r))
			return 0;
		v = r->value;
	}
	
	switch(n->op) {
	/* Clean up is special and must be handled directly. It also has the
	   type of the function return so don't use that for the cleanup value
	   in n->right */
	case T_CLEANUP:
		gen_cleanup(v);
		return 1;
	case T_NSTORE:
	case T_LBSTORE:
		if (s <= 2) {
			gen_op(s, "st", r);
			set_ea_node(r);
			return 1;
		}
		/* TODO: can do 4 sanely */
		break;
	case T_EQ:
		/* Nothing to do with writing back yet but this is a write
		   to an unknown object so we must kill any possible aliases */
		flush_writeback();
		/* Store right hand op in EA */
		ptr = load_ptr_ea();	/* Turn EA into a pointer */
		/* Generate the load without using that ptr */
		gen_load_notwith(r, ptr);
		/* EA now holds the data */
		if (s == 4) {
			if (!(n->flags & NORETURN))
				printf("\tld t,ea");
			printf("\tst ea,0,p%d\n", ptr);
			printf("\tld ea,@__high\n\tst ea,2,p%d\n", ptr);
			if (!(n->flags & NORETURN))
				printf("\tld ea,t\n");
		}
		if (s == 2)
			printf("\tst ea,0,p%d\n", ptr);
		else
			printf("\tst a,0,p%d\n", ptr);
		set_ea_node(r);
		return 1;
	case T_PLUS:
		/* TODO: can inline long I think */
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT) {
			if (v == 0)
				return 1;
			if (s == 1) { 
				printf("\tadd a,=%d\n", v & 0xFF);
				adjust_a(v);
			} else {
				printf("\tadd ea,=%d\n", v);
				adjust_ea(v);
			}
			return 1;
		}
		invalidate_ea();
		gen_op(s, "add", r);
		break;
	case T_MINUS:
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT) {
			if (v == 0)
				return 1;
			if (s == 1) {
				printf("\tsub a,=%d\n", v & 0xFF);
				adjust_a(v);
			} else { 
				printf("\tsub ea,=%d\n", v);
				adjust_ea(v);
			}
			return 1;
		}
		invalidate_ea();
		gen_op(s, "sub", r);
		break;
	case T_STAR:	/* Multiply is a complicated mess */
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT) {
			/* Try shifts and MUL op inlined */
			if (s <= 2 && gen_fast_mul(s, v))
				return 1;
		}
		puts("\tld t,ea");
		gen_load(r);
		puts("\tjsr __mul1616");
		invalidate_ea();
		break;
	case T_SLASH:
		if (s > 2)
			return 0;
		if (r->op == T_CONSTANT) {
			if (gen_fast_div(s, r))
				return 1;
		}
		invalidate_ea();
		puts("\tld t,ea");
		gen_load(r);
		if (n->type & UNSIGNED)
			puts("\tjsr __udiv1616");
		else
			puts("\tjsr __div1616");
		break;
	/* Should do T_PERCENT fast remainder cases */
	case T_AND:
		return gen_logic(r, s, "and", 1);
	case T_OR:
		return gen_logic(r, s, "or", 2);
	case T_HAT:
		return gen_logic(r, s, "xor", 3);
	case T_EQEQ:
	case T_GTEQ:
	case T_GT:
	case T_LTEQ:
	case T_LT:
	case T_BANGEQ:
		return gen_compare(n);
	case T_LTLT:
		if (s > 2)
			return 0;
		/* TODO track shift result */
		invalidate_ea();
		if (r->op == T_CONSTANT) {
			if (v > 15)
				return 1;
			if (v >= 8) {
				if (s == 1)
					return 1;
				v -= 8;
				puts("\tld e,a\n\tld a,=0");
			}
			if (s == 1)
				repeated_op("\tsl a", v);
			else
				repeated_op("\tsl ea", v);
			return 1;
		}
		/* Helper time */
		gen_load_t(r);
		return gen_t_op(s, n, "shl_t");
	case T_GTGT:
		if (s > 2)
			return 0;
		/* TODO track shift result */
		invalidate_ea();
		if ((n->type & UNSIGNED) && s<= 2 && r->op == T_CONSTANT) {
			if (v > 15)
				return 1;
			if (v >= 8) {
				if (s == 1)
					return 1;
				v -= 8;
				puts("\tld a,e\n\tld e,=0\n");
			}
			if (s == 1)
				repeated_op("\tsr a", v);
			else
				repeated_op("\tsr ea", v);
			return 1;
		}
		gen_load_t(r);
		return gen_t_op(s, n, "shr_t");
	case T_PLUSPLUS:
	case T_MINUSMINUS:
		/* Need to look at being smarter here if we know what
		   ea points to */
		flush_writeback();
		/* TODO: spot bytewide and dec/ld inc/ld ops */
		if (s <= 2 && !(n->flags & NORETURN)) {
			/* FIXME: wrong - result is old value not old ptr ?? */
			puts("ld t,ea");
			do_preincdec(s, n);
			puts("ld ea,t");
			return 1;
		}
		if (s > 2)
			return 0;
		/* Fall through */
	case T_PLUSEQ:
	case T_MINUSEQ:
		return do_preincdec(s, n);
	case T_ANDEQ:
	case T_OREQ:
	case T_HATEQ:
		/* EA holds the pointer */
		/* FIXME: constant case ? */
		/* FIXME; need to have an offs for sp relative */
		ptr = load_ptr_ea();
		ptr2 = gen_ref_notwith(r, ptr, &off);
		return generate_logic(n, ptr, ptr2, off);
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
	unsigned s = get_size(n->type);
	unsigned ptr;
	int off;

	/* The comma operator discards the result of the left side, then
	   evaluates the right. Avoid pushing/popping and generating stuff
	   that is surplus */
	if (n->op == T_COMMA) {
		n->left->flags |= NORETURN;
		codegen_lr(n->left);
		codegen_lr(n->right);
		return 1;
	}
	if ((n->op == T_NSTORE || n->op == T_LSTORE) && s < 2) {
		codegen_lr(n->right);
		/* Result is now in EA */
		ptr = gen_ref(n, &off);
		/* Ptr loaded */
		if (s == 2)
			printf("st ea,%d,p%d\n", off, ptr);
		else
			printf("st a,%d,p%da\n", off, ptr);
		set_ea_node(n);
		return 1;
	}
	return 0;
}

static unsigned gen_cast(struct node *n)
{
	unsigned lt = n->type;
	unsigned rt = n->right->type;
	unsigned ls;

	if (PTR(rt))
		rt = USHORT;
	if (PTR(lt))
		lt = USHORT;

	/* Floats and stuff handled by helper */
	if (!IS_INTARITH(lt) || !IS_INTARITH(rt))
		return 0;

	ls = get_size(lt);

	/* Size shrink is free */
	if ((lt & ~UNSIGNED) <= (rt & ~UNSIGNED))
		return 1;
	/* Don't do the harder ones */
	if (!(rt & UNSIGNED) || ls > 2)
		return 0;
	puts("\tld e,0");
	/* FIXME: really turn EA to A */
	invalidate_ea();
	return 1;
}

static unsigned pop_t_op(struct node *n, const char *op, unsigned sz)
{
	if (sz > 2)
		return 0;
	puts("\txch ea,t\n\tpop ea");
	return gen_t_op(sz, n, op);
}

static unsigned pop_ptr(void)
{
	unsigned ptr = free_pointer();
	printf("\tpop p%d\n", ptr);
	return ptr;
}

static unsigned logic_sp_op(struct node *n, const char *op)
{
	unsigned sz = get_size(n->type);
	invalidate_ea();
	if (sz == 1) {
		printf("\t%s a,2,sp\n", op);
		discard_word();
		return 1;
	}
	if (sz == 2) {
		printf("\t%s a,2,sp\n", op);
		printf("\txch a,e\n\t%s a,3,sp\n\txch a,e\n", op);
		discard_word();
		return 1;
	}
	/* For now don't inline dword ops */
	return 0;
}

unsigned gen_node(struct node *n)
{
	unsigned sz = get_size(n->type);
	unsigned v;
	unsigned ptr, ptr2;
	int off;
	unsigned noret = n->flags & NORETURN;
	struct node *r = n->right;

	/* We adjust sp so track the pre-adjustment one too when we need it */

	v = n->value;

	/* An operation with a left hand node will have the left stacked
	   and the operation will consume it so adjust the stack.

	   The exception to this is comma and the function call nodes
	   as we leave the arguments pushed for the function call */

	if (n->left && n->op != T_ARGCOMMA && n->op != T_CALLNAME && n->op != T_FUNCCALL)
		sp -= get_stack_size(n->left->type);

	switch(n->op) {
		/* We need a pointer to all objects so these come out
		   the same */
	case T_LREF:
		/* Kill unused local ref */
		if (noret)
			return 1;
		/* TODO: once volatile is cleaned up a bit more kill these
		   too */
	case T_NREF:
	case T_LBREF:
		ptr = gen_ref(n, &off);
		if (sz == 1)
			printf("\tld a,%d,p%d\n", off, ptr);
		if (sz == 2)
			printf("\tld ea,%d,p%d\n", off, ptr);
		if (sz == 4) {
			printf("\tld ea,%d+2,p%d\n",off, ptr);
			puts("\tst ea,@__high");
			printf("\tld ea,%d,p%d\n", off, ptr);
		}
		set_ea_node(n);
		return 1;
	case T_LSTORE:
	case T_NSTORE:
	case T_LBSTORE:
		ptr = gen_ref(n, &off);
		if (sz == 1)
			printf("\tst a,%d,p%d\n", off, ptr);
		if (sz == 2)
			printf("\tst ea,%d,p%d\n", off, ptr);
		if (sz == 4) {
			puts("\tld t,ea");
			puts("\tld ea,@__high");
			printf("\tst ea,%d+2,p%d\n",off, ptr);
			puts("\tld ea,t");
			printf("\tst ea,%d,p%d\n", off, ptr);
		}
		set_ea_node(n);
		return 1;
	case T_CALLNAME:
		flush_writeback();
		invalidate_all();
		printf("\tcall _%s+%d\n", namestr(n->snum), v);
		return 1;
	case T_EQ:
		/* *EAX = TOS */
		ptr = load_ptr_ea();
		puts("\tpop ea\n");
		invalidate_ea();
		flush_writeback();
		if (sz == 1)
			printf("\tst a,0,p%d\n", ptr);
		else {
			printf("\tst ea,0,p%d\n", ptr);
			if (sz == 4) {
				if (!noret)
					puts("\tld t,ea\n");
				puts("\tpop ea\n");
				printf("\tst ea,2,p%d\n", ptr);
				if (!noret) {
					puts("\tst ea,@__high");
					puts("\tld ea,t\n");
				}
			}
		}
		return 1;
	case T_DEREF:
		/* Might be able to be smarter here */
		flush_writeback();
		/* Could noret away once volatile cleaned */
		/* EAX = *EAX */
		ptr = load_ptr_ea();
		if (sz == 1)
			printf("\tld a,0,p%d\n", ptr);
		else if (sz == 2)
			printf("\tld ea,0,p%d\n", ptr);
		else {
			printf("\tld ea,2,p%d\n", ptr);
			puts("\tst ea,@__high");
			printf("\tld ea,0,p%d\n", ptr);
		}
		return 1;
	case T_FUNCCALL:
		flush_writeback();
		invalidate_all();
		/* EA holds the function ptr */
		puts("\tjsr __callea\n");
		return 1;
	case T_LABEL:
		printf("\tld ea,T%d+%d\n", n->val2, v);
		set_ea_node(n);
		return 1;
	case T_CONSTANT:
		if (sz == 4) {
			printf("\tld ea,=%d\n", v >> 16);
			puts("\tst ea,@__high");
		}
		if (sz > 1) {
			printf("\tld ea,=%d\n", v & 0xFFFF);
			set_ea(v & 0xFFFF);
		} else	{
			printf("\tld a,=%d\n", v & 0xFF);
			set_a(v & 0xFF);
		}
		return 1;
	case T_NAME:
		printf("\tld ea,=_%s+%d\n", namestr(n->snum), v);
		set_ea_node(n);
		return 1;
	case T_LOCAL:
		v += sp;
		puts("\tld ea,sp");
		printf("\tadd ea,=%d\n", v);
		set_ea_node(n);
		return 1;
	case T_ARGUMENT:
		v += frame_len + ARGBASE + sp;
		puts("\tld ea,sp");
		printf("\tadd ea,=%d\n", v);
		set_ea_node(n);
		return 1;
	case T_CAST:
		return gen_cast(n);
	case T_PLUS:
		invalidate_ea();
		if (sz == 1)
			printf("\tadd a,2,sp\n");
		if (sz == 2)
			printf("\tadd ea,2,sp\n");
		if (sz == 4) {
			puts("\tld t,ea");
			puts("\tld ea,4,sp");
			puts("\tadd ea,@__high");
			puts("\tst ea,@__high");
			puts("\tld ea,t");
			puts("\tadd ea,2,sp");
			discard_word();
		}
		discard_word();
		return 1;
	case T_MINUS:
		invalidate_ea();
		/* Check which way around.. */
		if (sz == 1)
			printf("\tsub a,2,sp\n");
		if (sz == 2)
			printf("\tsub ea,2,sp\n");
		if (sz == 4) {
			puts("\tld t,ea");
			puts("\tld ea,4,sp");
			puts("\tsub ea,@__high");
			puts("\tst ea,@__high");
			puts("\tld ea,t");
			puts("\tsub ea,2,sp");
			discard_word();
		}
		discard_word();
		return 1;
	case T_STAR:
		return pop_t_op(n, "mul_t", sz);
	case T_SLASH:
		return pop_t_op(n, "div_t", sz);
	case T_AND:
		/* EA &= TOS */
		return logic_sp_op(n, "and");
	case T_OR:
		return logic_sp_op(n, "or");
	case T_HAT:
		return logic_sp_op(n, "xor");
	/* Shift TOS by EA */
	case T_LTLT:
 		return pop_t_op(n, "sl_t", sz);
	case T_GTGT:
 		return pop_t_op(n, "sr_t", sz);
	/* TODO Comparisons, EQ ops */
	case T_PLUSEQ:
		flush_writeback();
		invalidate_ea();
		/* EA holds the value, TOS the ptr */
		if (sz > 2)
			return 0;
		ptr = pop_ptr();
		if (sz == 1) {
			printf("\tadd a,0,p%d", ptr);
			printf("\tst a,0,p%d\n", ptr);
		} else if (sz == 2) {
			printf("\tadd ea,0,p%d\n", ptr);
			printf("\tst ea,0,p%d\n", ptr);
		}
		return 1;
	case T_MINUSEQ:
		flush_writeback();
		invalidate_ea();
		if (sz > 2)
			return 0;
		/* TODO - awkward as it is *TOS - EA */
		ptr = pop_ptr();
		puts("\tpush ea");
		gen_load_ptr(ptr);
		if (sz == 1) {
			puts("\tadd a,2,sp");
			printf("\tst a,0,p%d\n", ptr);
		} else {
			puts("\tadd ea,2,sp");
			printf("\tst ea,0,p%d\n", ptr);
		}
		return 1;
	/* Q: can we get consts for these bits .. */
	/* FIXME: need to have a returned offset as likely be be %d,sp */
	case T_ANDEQ:
		ptr = pop_ptr();
		/* FIXME: generate_logic must invalidate etc */
		ptr2 = gen_ref_notwith(r, ptr, &off);
		return generate_logic(n, ptr, ptr2, off);
	case T_OREQ:
		ptr = pop_ptr();
		ptr2 = gen_ref_notwith(r, ptr, &off);
		return generate_logic(n, ptr, ptr2, off);
	case T_HATEQ:
		ptr = pop_ptr();
		ptr2 = gen_ref_notwith(r, ptr, &off);
		return generate_logic(n, ptr, ptr2, off);
	}
	return 0;
}
