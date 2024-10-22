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

uint8_t ttybuf[TTYSIZ * NUM_DEV_TTY];

int ttymap_count;
struct ttymap ttymap[NUM_DEV_TTY + 1];
struct s_queue ttyinq[NUM_DEV_TTY + 1];
tcflag_t termios_mask[NUM_DEV_TTY + 1];

void no_setup(uint_fast8_t minor, uint_fast8_t devn, uint_fast8_t flags)
{
    used(minor);
    used(devn);
    used(flags);
}

struct ttydriver ttydrivers[2] =
    {
        {rawuart_putc, rawuart_ready, rawuart_sleeping, rawuart_getc, rawuart_setup},
        {usbconsole_putc, usbconsole_ready, usbconsole_sleeping, usbconsole_getc, no_setup},
};

static void devtty_defconfig(uint8_t drv, int count, int minor)
{
    int devnum = 1;
    while (devnum <= count)
    {
        ttymap[minor].tty = devnum++;
        ttymap[minor].drv = drv;
        if (drv == TTYDRV_USB)
        {
            termios_mask[minor] = _CSYS;
        }
        else
        {
            termios_mask[minor] = CSIZE | CBAUD | PARENB | PARODD | _CSYS;
        }
        minor++;
    }
}

/* To be called right after startup to be able to print boot messages */
void devtty_early_init(void)
{
    rawuart_early_init();
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
        ttyinq[i].q_base = ttyinq[i].q_head = ttyinq[i].q_tail = &ttybuf[TTYSIZ * (i - 1)];
        ttyinq[i].q_size = TTYSIZ;
        ttyinq[i].q_count = 0;
        ttyinq[i].q_wakeup = TTYSIZ / 2;
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
            devtty_defconfig(TTYDRV_UART, NUM_DEV_TTY_UART, 1 + NUM_DEV_TTY_USB);
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
            devtty_defconfig(TTYDRV_USB, NUM_DEV_TTY_USB, 1 + NUM_DEV_TTY_UART);
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
            while (tty_writeready(1) != TTY_READY_NOW)
            {
                tight_loop_contents();
            }
            tty_putc(1, '\r');
        }
        while (tty_writeready(1) != TTY_READY_NOW)
        {
            tight_loop_contents();
        }
        tty_putc(1, c);
    }
}

void tty_putc(uint_fast8_t minor, uint_fast8_t c)
{
    struct ttymap *map = &ttymap[minor];
    if (map->tty == 0)
        return;
    struct ttydriver *drv = &ttydrivers[map->drv];
    drv->putc(map->tty, c);
}

ttyready_t tty_writeready(uint_fast8_t minor)
{
    struct ttymap *map = &ttymap[minor];
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
    struct ttymap *map = &ttymap[minor];
    if (map->tty == 0)
        return;
    struct ttydriver *drv = &ttydrivers[map->drv];
    drv->sleeping(map->tty);
}

void tty_data_consumed(uint_fast8_t minor) {}

/*
 *	This function is called whenever the terminal interface is opened
 *	or the settings changed. It is responsible for making the requested
 *	changes to the port if possible. Strictly speaking it should write
 *	back anything that cannot be implemented to the state it selected.
 */
void tty_setup(uint_fast8_t minor, uint_fast8_t flags)
{
    struct ttymap *map = &ttymap[minor];
    if (map->tty == 0)
        return;
    struct ttydriver *drv = &ttydrivers[map->drv];
    drv->setup(minor, map->tty, flags);
}

void tty_interrupt(void)
{
    int c;
    for (int minor = 1; minor <= ttymap_count; minor++)
    {
        struct ttymap *map = &ttymap[minor];
        if (map->tty == 0)
            continue;
        struct ttydriver *drv = &ttydrivers[map->drv];
        while ((c = drv->getc(map->tty)) >= 0)
        {
            if (tty_inproc(minor, c) == 0)
            {
                break;
            }
        }
    }
}
/* vim: sw=4 ts=4 et: */
