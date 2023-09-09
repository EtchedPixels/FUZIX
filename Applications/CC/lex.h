extern unsigned token;
extern unsigned line_num;
extern unsigned long token_value;
extern char filename[33];

extern unsigned label_tag;

extern void next_token(void);
extern void push_token(unsigned);
extern unsigned match(unsigned);
extern void require(unsigned);
extern void need_semicolon(void);
extern void junk(void);
extern unsigned symname(void);

extern unsigned quoted_string(int *len);
extern unsigned copy_string(unsigned label, unsigned maxlen, unsigned pad,
                                unsigned literal);

extern void out_write(void);
extern void out_flush(void);
extern unsigned long out_tell(void);
extern void out_seek(unsigned long pos);
extern void out_byte(unsigned char c);
extern void out_block(void *pv, unsigned len);
