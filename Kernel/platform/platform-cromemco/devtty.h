extern void tty_drain(void);

extern void tuart0_txqueue(uint8_t c) __z88dk_fastcall;
extern uint8_t tuart0_rx_get(void);
extern void tuart0_error_get(void);
extern void tuart0_txd(uint8_t unused);
extern void tuart0_rx_ring(uint8_t unused);

extern uint8_t tuart0_txl, tuart0_rxl;

extern void tuart1_txqueue(uint8_t c) __z88dk_fastcall;
extern uint8_t tuart1_rx_get(void);
extern void tuart1_error_get(void);
extern void tuart1_txd(uint8_t unused);
extern void tuart1_rx_ring(uint8_t unused);

extern uint8_t tuart1_txl, tuart1_rxl;

extern void tuart2_txqueue(uint8_t c) __z88dk_fastcall;
extern uint8_t tuart2_rx_get(void);
extern void tuart2_error_get(void);
extern void tuart2_txd(uint8_t unused);
extern void tuart2_rx_ring(uint8_t unused);

extern uint8_t tuart2_txl, tuart2_rxl;

extern uint8_t tty_irqmode;
