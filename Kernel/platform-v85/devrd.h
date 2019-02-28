extern void rd_input(uint8_t *addr, uint16_t block, uint16_t page);
extern void rd_output(uint8_t *addr, uint16_t block, uint8_t page);
extern uint8_t rd_present(uint8_t board);

extern int devrd_open(uint8_t minor, uint16_t flag);
extern int devrd_read(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern int devrd_write(uint8_t minor, uint8_t rawflag, uint8_t flag);
extern void devrd_init(void);
