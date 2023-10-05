#ifndef	__DEVTTY_H
#define	__DEVTTY_H

extern const char *uart_name[];

struct uart {
    uint8_t (*intr)(uint_fast8_t minor);
    ttyready_t (*writeready)(uint_fast8_t minor);
    void (*putc)(uint_fast8_t minor, uint_fast8_t c);
    void (*setup)(uint_fast8_t minor, uint_fast8_t wait);
    uint8_t (*carrier)(uint_fast8_t minor);
    void (*data_consumed)(uint_fast8_t minor);
    uint16_t cmask;
    const char *name;
};

extern const struct uart *uart[NUM_DEV_TTY + 1];
extern void *ttybase[NUM_DEV_TTY + 1];
extern uint8_t register_uart( void *base, const struct uart *);
extern void display_uarts(void);

extern const struct uart ns16x50_uart;

#endif	/* __DEVTTY2_H	*/	
