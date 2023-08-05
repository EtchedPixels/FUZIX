extern int gm833_open(uint_fast8_t minor, uint16_t flag);
extern int gm833_read(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
extern int gm833_write(uint_fast8_t minor, uint_fast8_t rawflag, uint_fast8_t flag);
extern uint_fast8_t gm833_probe(void);
