#include <kernel.h>
#include <kdata.h>
#include <input.h>
#include <input.h>

uint8_t read_js(uint8_t *slot, uint8_t n, uint8_t r)
{
    uint8_t d = 0;

    if (r & 1)
        d = STICK_DIGITAL_U;
    if (r & 2)
        d |= STICK_DIGITAL_D;
    if (r & 4)
        d |= STICK_DIGITAL_L;
    if (r & 8)
        d |= STICK_DIGITAL_R;
    if (r & 16)
        d |= BUTTON(0);
    if (r & 32)
        d |= BUTTON(1);

    *slot++ = STICK_DIGITAL | (n + 1);
    *slot = d;

    return 2;
}

static uint16_t old_js;

int plt_input_read(uint8_t *slot)
{
    uint16_t js = sysmod_joystick();
    uint16_t delta = js ^ old_js;
    old_js = js;

    if (delta & 0xFF00) { 
        *slot++ = STICK_DIGITAL | 1;
        *slot = js >> 8;
        return 2;
    }

    if (delta & 0xFF) {
        *slot++ = STICK_DIGITAL | 1;
        *slot = js;
        return 2;
    }
    return 0;
}

void plt_input_wait(void)
{
    psleep(&old_js);	/* We wake this on timers so it works for sticks */
}

void poll_input(void)
{
    if (sysmod_joystick() != old_js)
        wakeup(&old_js);
}

int plt_input_write(uint8_t flag)
{
    flag;
    udata.u_error = EINVAL;
    return -1;
}
