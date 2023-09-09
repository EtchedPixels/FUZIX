extern struct node *typeconv(struct node *n, unsigned type, unsigned warn);
extern unsigned bracketed_expression(unsigned mkbool);
extern void expression_or_null(unsigned mkbool, unsigned noret);
extern unsigned const_int_expression(void);
extern unsigned expression(unsigned comma, unsigned mkbool, unsigned noret);
extern struct node *expression_tree(unsigned comma);
extern struct node *hier0(unsigned comma);	/* needed for primary bracketed */
extern void expression_typed(unsigned type);
