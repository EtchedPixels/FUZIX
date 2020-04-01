#include <kernel.h>
#include <kdata.h>
#include <input.h>
#include <devinput.h>

/*
 *	Joysticks in parts of the PIO ports
 */

__sfr __at 0x1C js1;
__sfr __at 0x1E js2;

static uint8_t js_data[2]= {255, 255};

uint8_t read_js(uint8_t *slot, uint8_t n, uint8_t r)
{
    uint8_t d = 0;
    r &= 0xF8;
    if (r == js_data[n])
        return 0;

    js_data[n] = r;
    if (r & 0x08)
        d = STICK_DIGITAL_U;
    if (r & 0x10)
        d |= STICK_DIGITAL_D;
    if (r & 0x20)
        d |= STICK_DIGITAL_L;
    if (r & 0x40)
        d |= STICK_DIGITAL_R;
    if (r & 0x80)
        d |= BUTTON(0);
    *slot++ = STICK_DIGITAL | (n + 1);
    *slot = d;
    return 2;
}

int platform_input_read(uint8_t *slot)
{
    if (read_js(slot, 0, js1))
        return 2;
    if (read_js(slot, 1, js2))
        return 2;
    return 0;
}

void platform_input_wait(void)
{
    psleep(&js_data);
}

void poll_input(void)
{
    if ((js1 & 0xF8) != js_data[0] ||
        ((js2 & 0xF8) != js_data[1]))
        wakeup(&js_data);
}

int platform_input_write(uint8_t flag)
{
    flag;
    udata.u_error = EINVAL;
    return -1;
}
