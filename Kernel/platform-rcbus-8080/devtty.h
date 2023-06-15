extern void tty_poll(void);
extern int rctty_ioctl(uint_fast8_t minor, uarg_t arg, char *ptr);
extern int rctty_open(uint_fast8_t minor, uint16_t flag);
extern void rctty_init(void);

extern void ttyout_acia(uint8_t c);
extern void acia_setup(uint8_t r);
extern uint_fast8_t acia_ready(void);
extern uint16_t acia_poll(void);

extern void uart_setup(uint8_t r);
extern void ttyout_uart(uint8_t c);
extern uint_fast8_t uart_ready(void);
extern uint16_t uart_poll(void);
