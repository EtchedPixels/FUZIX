#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <stdbool.h>
#include <vt.h>
#include <tty.h>
#include "rawuart.h"
#include "picosdk.h"
#include <pico/multicore.h>
#include "core1.h"
#include "devtty.h"

uint8_t ttybuf[TTYSIZ*NUM_DEV_TTY];

int ttymap_count;
struct ttymap ttymap[NUM_DEV_TTY+1];
struct s_queue ttyinq[NUM_DEV_TTY+1];
tcflag_t termios_mask[NUM_DEV_TTY+1];

struct ttydriver ttydrivers[2] =
{
    { .putc = &rawuart_putc, .ready = &rawuart_ready, .sleeping = rawuart_sleeping, .getc = &rawuart_getc },
    { .putc = &usbconsole_putc, .ready = &usbconsole_ready, .sleeping = usbconsole_sleeping, .getc = &usbconsole_getc },
};

static void devtty_defconfig(uint8_t drv, int count, int minor)
{
    int devnum = 1;
    while(devnum <= count)
    {
        ttymap[minor].tty = devnum++;
        ttymap[minor].drv = drv;
        minor++;
    }
}

/* To be called right after startup to be able to print boot messages */
void devtty_early_init(void)
{
    rawuart_init();
    core1_init();
    devtty_init();
}

void devtty_init(void)
{
    int defconfig = 0;
    if (ttymap_count == 0)
    {
        defconfig = 1;
    }
    for (int i = 1; i <= NUM_DEV_TTY; i++)
    {
        ttyinq[i].q_base = ttyinq[i].q_head = ttyinq[i].q_tail = &ttybuf[TTYSIZ*(i-1)];
        ttyinq[i].q_size = TTYSIZ;
        ttyinq[i].q_count = 0;
        ttyinq[i].q_wakeup = TTYSIZ/2;
        termios_mask[i] = _CSYS;
    }
    
    if (defconfig)
    {
        absolute_time_t until = delayed_by_ms(get_absolute_time(), DEV_USB_DETECT_TIMEOUT);

        int usb_detected = 0;
        while (absolute_time_diff_us(get_absolute_time(), until) > 0)
        {
            if (usbconsole_is_available(1))
            {
                usb_detected = 1;
                break;
            }
        }

        if (usb_detected)
        {
            devtty_defconfig(TTYDRV_USB, NUM_DEV_TTY_USB, 1);
            devtty_defconfig(TTYDRV_UART, NUM_DEV_TTY_UART, 1+NUM_DEV_TTY_USB);
            until = delayed_by_ms(get_absolute_time(), DEV_USB_INIT_TIMEOUT);
            while (absolute_time_diff_us(get_absolute_time(), until) > 0)
            {
                tight_loop_contents();
            }
            kprintf("devtty: %s as default tty\n", "usb");
        }
        else
        {
            devtty_defconfig(TTYDRV_UART, NUM_DEV_TTY_UART, 1);
            devtty_defconfig(TTYDRV_USB, NUM_DEV_TTY_USB, 1+NUM_DEV_TTY_UART);
            kprintf("devtty: %s as default tty\n", "uart");
        }
        ttymap_count = NUM_DEV_TTY;
    }
}

/* Output for the system console (kprintf etc) */
void kputchar(uint_fast8_t c)
{
    /* If tty's were not properly initialized */
    if (ttymap_count == 0)
    {
        if (c == '\n')
            rawuart_putc(1, '\r');
        rawuart_putc(1, c);
    }
    else
    {
        if (c == '\n')
        {
            while(tty_writeready(1) != TTY_READY_NOW) {
                tight_loop_contents();
            }
            tty_putc(1, '\r');
        }
        while(tty_writeready(1) != TTY_READY_NOW) {
            tight_loop_contents();
        }
        tty_putc(1, c);
    }
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
    struct ttymap* map = &ttymap[minor];
    if (map->tty == 0)
        return;
    struct ttydriver *drv = &ttydrivers[map->drv];
    drv->putc(map->tty, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
    struct ttymap* map = &ttymap[minor];
    if (map->tty == 0)
        return TTY_READY_LATER;
    struct ttydriver *drv = &ttydrivers[map->drv];
    return drv->ready(map->tty);
}

/* For the moment */
int tty_carrier(uint_fast8_t minor)
{
    return 1;
}

void tty_sleeping(uint_fast8_t minor)
{
    struct ttymap* map = &ttymap[minor];
    if (map->tty == 0)
        return;
    struct ttydriver *drv = &ttydrivers[map->drv];
    drv->sleeping(map->tty);
}
void tty_data_consumed(uint_fast8_t minor) {}
void tty_setup(uint_fast8_t minor, uint_fast8_t flags) {}

void tty_interrupt(void)
{
    int c;
    for (int minor = 1; minor <= ttymap_count; minor++)
    {
        struct ttymap* map = &ttymap[minor];
        if (map->tty == 0)
            continue;
        struct ttydriver *drv = &ttydrivers[map->drv];
        while((c = drv->getc(map->tty)) >= 0)
        {
            tty_inproc(minor, c);
        }
    }
}
/* vim: sw=4 ts=4 et: */

