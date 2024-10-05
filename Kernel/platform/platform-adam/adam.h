extern uint8_t *getpcb(void);
extern uint8_t *getdcb(unsigned dev) __z88dk_fastcall;

extern uint8_t lptout(unsigned ch) __z88dk_fastcall;
extern uint8_t lptcheck(void);

extern uint8_t keypoll(void);
extern uint8_t keycheck(void);

extern uint8_t readbegin(uint8_t *addr) __z88dk_fastcall;
extern uint8_t readdone(void);

extern uint8_t writebegin(uint8_t *addr) __z88dk_fastcall;
extern uint8_t writedone(void);

extern uint16_t lbahi, lbalo;
extern uint8_t andev;

extern void adamnet_probe(void);
