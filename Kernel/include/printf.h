extern void kprintf(const char *, ...);
extern void kputs(const char *);
extern void kputnum(int v);
extern void kuputunum(unsigned int v);
extern void kputhex(unsigned int v);
extern void kputhexbyte(unsigned int b);

/* The platform must provide this method */
extern void kputchar(uint_fast8_t c);

/* Provided by lib/hexdump.c if you need it */
extern void hexdump(const void* ptr, size_t size);

