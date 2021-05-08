#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

/* Not sure this belongs here FIXME */
extern uint8_t timer_source;
#define TIMER_NONE		0
#define TIMER_TMS9918A		1
#define TIMER_QUART		2
#define TIMER_SC26C92		3

extern uint8_t sc26c92_present;
extern uint8_t tms9918a_present;
extern uint8_t shadowcon;
extern uint8_t inputtty;

void do_timer_interrupt(void);
void tty_putc(uint_fast8_t minor, uint_fast8_t c);
void tty_pollirq(void);

void set_console(void);
void do_conswitch(uint8_t c);

uint8_t *init_alloc(unsigned int);

extern uint8_t nuart;

extern const char *uart_name[];

struct uart {
    uint8_t (*intr)(uint_fast8_t minor);
    uint8_t (*writeready)(uint_fast8_t minor);
    void (*putc)(uint_fast8_t minor, uint_fast8_t c);
    void (*setup)(uint_fast8_t minor);
    uint8_t (*carrier)(uint_fast8_t minor);
    uint16_t cmask;
    const char *name;
};

extern struct uart *uart[NUM_DEV_TTY + 1];
extern uint16_t ttyport[NUM_DEV_TTY + 1];
extern uint8_t register_uart( uint16_t port, struct uart *);
extern void insert_uart(uint16_t port, struct uart *);
extern void display_uarts(void);

extern struct uart acia_uart;
extern struct uart ns16x50_uart;
extern struct uart tms_uart;
extern struct uart quart_uart;
extern struct uart sc26c92_uart;
extern struct uart xr88c681_uart;

#define ACIA_C		0xA0
#define ACIA_D		0xA1

#endif
