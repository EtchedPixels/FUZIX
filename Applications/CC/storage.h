extern unsigned get_storage(unsigned defstorage);
extern unsigned is_storage_word(void);

extern void put_typed_data(struct node *n);
extern void put_padding_data(unsigned space);
extern void put_typed_constant(unsigned type, unsigned long value);
extern void put_typed_case(unsigned tag, unsigned entry);
