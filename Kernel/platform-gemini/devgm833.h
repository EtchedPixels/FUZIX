
extern void gm833_in(uint8_t *addr) __z88dk_fastcall;
extern void gm833_out(uint8_t *addr) __z88dk_fastcall;

extern int gm833_open(uint8_t minor, uint16_t flag);
extern int gm833_close(uint8_t minor);
extern int gm833_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int gm833_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern void gm833_init(void);
