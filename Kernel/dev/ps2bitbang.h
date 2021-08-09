extern uint8_t kbsave;
extern uint16_t kbdelay;
extern uint8_t kbport;
extern uint8_t kbwait;

/* The C version uses this structure internally and for helpers */

struct ps2op {
    uint16_t port;
    
    uint_fast8_t base;

    uint_fast8_t clockdata;
    uint_fast8_t clockin;
    uint_fast8_t clockonly;

    uint_fast8_t databit;    
    uint_fast8_t floatboth;
    uint_fast8_t clockrel;
    uint_fast8_t clocklow;
    uint_fast8_t clockmask;

    uint_fast8_t (*bit)(struct ps2op *p);
    void	 (*sendbit)(struct ps2op *p, uint_fast8_t bit);

    /* Timeout bases */
    uint16_t long_timeout;
    uint16_t poll_timeout;
    uint16_t reply_timeout;

    /* Actual running timeout */
    uint16_t timeout;
};

extern void napus(uint16_t us);
extern uint16_t ps2_get(struct ps2op *p);
extern uint16_t ps2_put(struct ps2op *p, uint_fast8_t c);