extern uint_fast8_t ch375_probe(void);

extern uint8_t ch375_rdata(void);
extern uint8_t ch375_rstatus(void);
extern void ch375_wdata(uint_fast8_t data);
extern void ch375_wcmd(uint_fast8_t cmd);
extern void ch375_rblock(uint8_t *ptr);
extern void ch375_wblock(uint8_t *ptr);
