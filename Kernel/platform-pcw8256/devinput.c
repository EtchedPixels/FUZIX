#include <kernel.h>
#include <kdata.h>
#include <input.h>
#include <devinput.h>

__sfr __at 0xA0 amx_v;
__sfr __at 0xA1 amx_h;
__sfr __at 0xA2 amx_button;
__sfr __at 0xA3 amx_ctrl;

__sfr __at 0xD0 kemp_x;
__sfr __at 0xD1 kemp_y;
__sfr __at 0xD4 kemp_button;

__sfr __at 0x9F kemp_js;

static uint8_t has_amx, has_kemp, has_kempjs;
static uint8_t okx, oky;

static char buf[32];
static struct s_queue kqueue = {
 buf, buf, buf, sizeof(buf), 0, sizeof(buf) / 2
};

/* Queue a character to the input device */
void queue_input(uint8_t c)
{
    insq(&kqueue, c);
    wakeup(&kqueue);
}

static uint8_t delta(uint8_t old, uint8_t new)
{
    /* Rolled over positive */
    if (old > 0xE0 && new < 0x20)
       return new + (0x100 - old);
    /* Rolled over negative */
    if (old < 0x20 && new > 0xE0)
       return -((0x100 - new) + old);
    else
        /* Actully this is signed 8bit but it doesn't matter here */
        return new - old;
}
        
/* First cut - we really should do the mouse delta accumulation in the
   vblank interrupt and just ouput the sum here */
int plt_input_read(uint8_t *slot)
{
    uint8_t r;
    if (remq(&kqueue, &r)) {
	*slot++ = KEYPRESS_CODE | r;
        remq(&kqueue, &r);
	*slot++ = r;
	return 2;
    }
    if (has_amx) {
        *slot++ = MOUSE_REL;
        r = amx_v;
        *slot++ = ~amx_button & 7;
        *slot++ = (r >> 4) - (r & 0x0F);
        *slot++ = (r & 0x0F) - (r >> 4);
        return 4;
    }
    if (has_kemp) {
        uint8_t kx = kemp_x;
        uint8_t ky = kemp_y;            
        *slot++ = MOUSE_REL;
        *slot++ = ~kemp_button & 3;
        *slot++ = delta(okx, kx);
        *slot++ = delta(oky, ky);
        /* Need to think about this eg 0xF0 -> 0x0F is a positive move
           of 0x1F while 0x0F->0xF0 is a negative move of 0x1F */
        oky = ky;
        okx = kx;
        return 4;
    }
    if (has_kempjs) {
        uint8_t v = 0;
        r = kemp_js;
        if (r & 1)
            v = STICK_DIGITAL_R;
        if (r & 2)
            v = STICK_DIGITAL_L;
        if (r & 4)
            v = STICK_DIGITAL_U;
        if (r & 8)
            v = STICK_DIGITAL_D;
        if (r & 16)
            v |= BUTTON(0);
        *slot++ = STICK_DIGITAL;
        *slot++ = v;
        return 2;
    }
    return 0;
}

/* All our devices are polled */

/* On an IRQ based system this routine is internally responsible for avoiding
   races between event wakeup and sleep */
void plt_input_wait(void)
{
    /* Mice need polling every vblank, for a typical tty only machine however
       we can do less wakeups. Really we need to move the mouse to the timer
       and do a wakeup only if there is a delta */
    if (has_amx||has_kemp)
	psleep(&vblank);
    else
	psleep(&kqueue);
}

int plt_input_write(uint8_t flag)
{
    flag;
    udata.u_error = EINVAL;
    return -1;
}

uint8_t plt_input_init(void)
{
    /* Kempston or AMS mice - don't support both at once */
    if (kemp_x != 0xFF || kemp_y != 0xFF || kemp_button != 0xFF)
        has_kemp = 1;
    else if (amx_ctrl != 0xFF && amx_button != 0x10) {
        amx_ctrl = 0x93;
        amx_button = 0xFF;
        amx_button = 0x00;
        has_amx = 1;
    }
    /* Could also have a keymouse */
    /* Kempston Joystick */
    if ((kemp_js & 0x80) == 0)
        has_kempjs = 1;
    /* TODO: DkTronics sound card has a joystick port too */
    return 0;
}
