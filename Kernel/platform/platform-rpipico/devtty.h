#ifndef __DEVTTY_DOT_H__
#define __DEVTTY_DOT_H__

#include <stdint.h>
#include <tty.h>

struct ttydriver
{
    void (*putc)(uint8_t devn, uint8_t c);
    ttyready_t (*ready)(uint8_t devn);
    void (*sleeping)(uint8_t devn);
    int (*getc)(uint8_t devn);
    void (*setup)(uint_fast8_t minor, uint_fast8_t devn, uint_fast8_t flags);
};

struct ttymap
{
    uint8_t tty;
    uint8_t drv;
};

#define TTYDRV_UART 0
#define TTYDRV_USB 1

extern int ttymap_count;
extern struct ttymap ttymap[NUM_DEV_TTY+1];
extern void tty_interrupt(void);
extern void devtty_early_init(void);
extern void devtty_init(void);

#endif
