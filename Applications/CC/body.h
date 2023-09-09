extern void statement_block(unsigned need_brack);
extern void function_body(unsigned st, unsigned name, unsigned type);

extern unsigned func_flags;

#define F_VOIDRET		1
#define F_VOID			2
#define F_VARARG		4

/* Registers start at 1 and bit 8 to 15 */
#define F_REG(n)		(1 << (n + 7))
