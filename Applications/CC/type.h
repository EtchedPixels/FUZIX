/*
 *	Types. Reorganized so we can do some vaguely proper type management
 *
 */
#define PTR(x)		((x) & 7)		/* 7 deep should be loads */
/* These are orderd for promotion rules */
#define UNSIGNED	0x08
#define CCHAR		0x00			/* 00-7F - integer */
#define CSHORT		0x10
#define	CLONG		0x20
#define CLONGLONG	0x30
#define UCHAR		0x08
#define USHORT		0x18
#define ULONG		0x28
#define ULONGLONG	0x38
#define	FLOAT		0x80
#define DOUBLE		0x90
#define VOID		0xA0
#define ELLIPSIS	0xB0

#define UNKNOWN		0xFE
#define ANY		0xFF

#define PTRTO		1			/* Use with care for comparisons */

/* For non simple types the information index */
#define INFO(x)		(((x) >> 3) & 0x3FF)

#define CLASS(x)	((x) & 0xC000)
#define IS_SIMPLE(x)	(CLASS(x) == C_SIMPLE)
#define IS_STRUCT(x)	(CLASS(x) == C_STRUCT)
#define IS_FUNCTION(x)	(CLASS(x) == C_FUNCTION)
#define IS_ARRAY(x)	(CLASS(x) == C_ARRAY)

#define BASE_TYPE(x)	((x) & ~7)
#define IS_ARITH(x)	(!PTR(x) && (x) < VOID)
#define IS_INTARITH(x)	(!PTR(x) && (x) < FLOAT)
#define IS_INTORPTR(x)	((x) < (FLOAT | 7))

#define C_SIMPLE	0x0000
#define C_STRUCT	0x4000
#define C_FUNCTION	0x8000
#define C_ARRAY		0xC000		/* and other special later ? */

#define C_ANY		0xFFFF		/* Special compiler internal */

extern unsigned type_deref(unsigned t);
extern unsigned type_ptr(unsigned t);
extern unsigned type_addpointer(unsigned t, unsigned ptr);
extern unsigned type_canonical(unsigned t);
extern unsigned type_sizeof(unsigned t);
extern unsigned type_alignof(unsigned t);
extern unsigned type_ptrscale(unsigned t);
extern unsigned type_scale(unsigned t);
extern unsigned type_addrof(unsigned t);
extern unsigned type_ptrscale_binop(unsigned op, struct node *l, struct node *r, unsigned *rtype);
extern int type_pointermatch(struct node *l, struct node *r);
extern int type_pointerconv(struct node *r, unsigned lt, unsigned warn);

extern unsigned deffunctype;	/* Type number for int foo(); */
extern unsigned voltrack;	/* Track volatile possibility */
