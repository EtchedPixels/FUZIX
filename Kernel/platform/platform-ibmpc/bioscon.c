#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <devtty.h>
#include <device.h>
#include <vt.h>
#include <tty.h>
#include <graphics.h>

/*
 *	Minimal BIOS mode console. We could do a lot more by using the
 *	attributes, mode information and pages properly but this will get
 *	us going.
 */

/*
 *	Private helpers in the asm library
 */
extern uint16_t bios_videomode(void);
extern uint16_t bios_setpage(uint16_t page);
extern void bios_videopage(uint16_t page);
extern void bios_setcursor(uint16_t xy, uint16_t page);
extern void bios_setchar(uint16_t ch, uint16_t pgattr, uint16_t rep);
extern void bios_scrollup(void);
extern void bios_scrolldown(void);
extern void bios_clearscreen(void);

/*
 *	These get mapped to VT_HEIGHT and friends by config.h
 */
int8_t vt_height = 25;	/* Fixed for now */
int8_t vt_width;
uint8_t vtattr_cap = VTA_INVERSE|VTA_UNDERLINE|VTA_BOLD|VTA_NOCURSOR;
/* Ok we don't do them yet but .. */

/* Methods we must supply to the VT core */
void cursor_off(void)
{
}

void cursor_on(int8_t y, int8_t x)
{
    bios_setcursor((y << 8) | x, 0);
}

void plot_char(int8_t y, int8_t x, uint16_t ch)
{
    bios_setcursor((y << 8) | x, 0);
    bios_setchar(ch, 7, 1);
}

void clear_lines(int8_t y, int8_t ct)
{
    uint8_t i;
    if (ct == 25)
        bios_clearscreen();
    else {
        for(i = 0; i < ct; y++) {
            bios_setcursor((y + i) << 8, 0);
            bios_setchar(' ', 7, vt_width);
        }
    }
}

void clear_across(int8_t y, int8_t x, int16_t l)
{
    bios_setchar(' ', y + (x << 8), l);
}

void vtattr_notify(void)
{
}

void scroll_up(void)
{
    bios_scrollup();
}

void scroll_down(void)
{
    bios_scrolldown();
}

/* We only have one console */
void console_switch(uint8_t n)
{
}

/* Sit in the current mode in black and white and hope it carries on working */
void bioscon_init(void)
{
    uint16_t mode = bios_videomode();
    vt_width = mode >> 8;
    if (vt_width == 0)	/* In case of busted stuff we try */
        vt_width = 80;
    bios_setpage(0);
    bios_videopage(0);
}
