
extern void skip_modifiers(void);
extern unsigned type_name_parse(unsigned storage, unsigned type, unsigned *name);
extern unsigned get_type(void);

extern unsigned is_modifier(void);
extern unsigned is_type_word(void);
extern struct symbol *is_typedef(void);

extern unsigned funcbody;
