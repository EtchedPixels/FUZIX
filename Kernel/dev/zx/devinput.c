/*
 *	TODO:
 *	- Fuller mouse on 7F bits are 7-0
 *			F1 F2 F3 1 R L D U
 */
#include <kernel.h>
#include <kdata.h>
#include <input.h>
#include <devinput.h>

__sfr __at 0x1F	kempston_1;
static uint8_t oldkemp;

__sfr __banked __at 0xFBDF mctx;
__sfr __banked __at 0xFFDF mcty;
__sfr __banked __at 0xFADF mbuttons;
__sfr __banked __at 0xFEDF turbodet;

__sfr __at 0x7F fuller_1;
static uint8_t oldfuller;

static uint8_t mousey, mousex, mouseb;
uint8_t kempston_mbmask = 3;

static char buf[32];

static struct s_queue kqueue = {
 buf, buf, buf, sizeof(buf), 0, sizeof(buf) / 2
};

uint8_t fuller, kempston, kmouse;

/* Queue a character to the input device */
void queue_input(uint8_t c)
{
    insq(&kqueue, c);
    wakeup(&kqueue);
}

/* There are extended mice/joysticks that use bit 5-7 for fire 2/3/4. We don't
   deal with that here yet - I need to check the classic ones always keep the
   undefined bits clear first */
static uint8_t kempston_js(uint8_t *slot)
{
    uint8_t k = 0;
    uint8_t r = kempston_1;
    if (r == oldkemp)
        return 0;
    oldkemp = r;
    if (r & 1)
        k = STICK_DIGITAL_R;
    if (r & 2)
        k |= STICK_DIGITAL_L;
    if (r & 4)
        k |= STICK_DIGITAL_D;
    if (r & 8)
        k |= STICK_DIGITAL_U;

    if (r & 16)
        k |= BUTTON(0);
    /* K Mouse turbo has four way fire support */
    if (turbodet == 128) {
        if (r & 32)
            k |= BUTTON(1);
        if (r & 64)
            k |= BUTTON(2);
        if (r & 128)
            k |= BUTTON(3);
    }
    *slot++ = STICK_DIGITAL;
    *slot = k;
    return 2;
}

static uint8_t fuller_js(uint8_t *slot)
{
    uint8_t k = 0;
    uint8_t r = fuller_1;
    if (r == oldfuller)
        return 0;
    oldfuller = r;
    r = ~r;
    if (r & 1)
        k = STICK_DIGITAL_U;
    if (r & 2)
        k |= STICK_DIGITAL_D;
    if (r & 4)
        k |= STICK_DIGITAL_L;
    if (r & 8)
        k |= STICK_DIGITAL_R;
    if (r & 128)
        k |= BUTTON(0);

    /* We are JS 0 unless a Kempston stick is present then we are JS 1 */
    *slot++ = STICK_DIGITAL|kempston;
    *slot = k;
    return 2;
}

/* Remap the button bits to input layer buttons */
/* This works for the real thing and the K-Mouse Turbo, but the Russian
   clones appear to have the buttons as
    d0 - left d1 -right d2 - middle */

static uint8_t flipbits(uint8_t m)
{
    uint8_t r = 0;
    if (!(m & 1))
            r = BUTTON(1);
    if (!(m & 2))
        r |= BUTTON(0);
    if (turbodet == 128) {
        if (!(m & 4))
            r |= BUTTON(2);	/* Has a middle button */
    }
    
    return r;
}

int platform_input_read(uint8_t *slot)
{
    uint8_t r, k;
    uint8_t x, y, m;
    if (remq(&kqueue, &r)) {
        remq(&kqueue, &k);
	*slot++ = KEYPRESS_CODE | r;
	*slot = k;
	return 2;
    }

    if (kempston && kempston_js(slot))
       return 2;

    if (fuller && fuller_js(slot))
        return 2;

    if (!kmouse)
        return 0;

    /* Mouse check */
    x = mctx;
    y = mcty;
    m = mbuttons & kempston_mbmask;
    if (x == mousex && y == mousey && m == mouseb)
        return 0;
    /* Calculate delta relative to counters - they are not zeroed on read
       so we do deltas */
    *slot++ = MOUSE_REL_WHEEL|flipbits(m);
    *slot++ = x - mousex;
    *slot++ = y - mousey;
    /* K Mouse Turbo */
    if (turbodet == 128)
        *slot++ = (m & 0xF0) - (mouseb & 0xF0);
    else
        *slot = 0;
    mousex = x;
    mousey = y;
    mouseb = m;
    return 3;
}

void platform_input_wait(void)
{
    psleep(&kqueue);	/* We wake this on timers so it works for sticks */
}

int platform_input_write(uint8_t flag)
{
    flag;
    udata.u_error = EINVAL;
    return -1;
}

void poll_input(void)
{
    if (kempston && kempston_1 != oldkemp)
	    wakeup(&kqueue);
    else if (fuller && fuller_1 != oldfuller)
	    wakeup(&kqueue);
    /* Mouse check */
    else if (kmouse && (mctx != mousex || mcty != mousey || (mouseb != mbuttons & kempston_mbmask)))
	    wakeup(&kqueue);
}
        