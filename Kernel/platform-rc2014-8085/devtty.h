extern void tty_poll(void);
extern void ttyout(uint8_t c);

extern void uart_setup(uint8_t r);

extern int rctty_ioctl(uint8_t minor, uarg_t arg, char *ptr);

extern uint_fast8_t uart_ready(void);
extern uint16_t uart_poll(void);
