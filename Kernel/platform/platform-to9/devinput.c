#include <kernel.h>
#include <kdata.h>
#include <input.h>
#include <devtty.h>
#include <devinput.h>

uint8_t mouse_buttons;
uint16_t mouse_x;
uint16_t mouse_y;

static unsigned bios_pen(void)
{
    unsigned r;
    in_bios = 1;
    r = mon_lightpen();
    in_bios = 0;
    return r;
}

static void bios_mouse(void)
{
    in_bios = 1;
    mon_mouse();
    in_bios = 0;
}

int plt_input_read(uint8_t *buf)
{
    if (inputdev == 0xC0) {
        bios_mouse();
        *buf = MOUSE_ABS;
    } else if (inputdev == 0x40 && bios_pen()) {
        *buf = LIGHTPEN_ABS;
    } else
        return 0;

    buf[1] = mouse_buttons;
    /* Scale to 16bit virtual co-ordinates */
    mouse_x *= 205;
    mouse_x += 70;
    mouse_y *= 329;
    mouse_y += 32;
    /* We are big endian */
    buf[2] = mouse_x >> 8;
    buf[3] = mouse_x;
    buf[4] = mouse_y >> 8;
    buf[5] = mouse_y;
    return 6;
}

void plt_input_wait(void)
{
    inputwait++;
    psleep(&inputwait);	/* We wake this on timers so it works for polled */
    inputwait--;
}

int plt_input_write(uint8_t flag)
{
    flag;
    udata.u_error = EINVAL;
    return -1;
}
