#include <kernel.h>
#include <kdata.h>
#include <input.h>
#include <devtty.h>

uint8_t mousein[7];
uint8_t mouse_present;
static int16_t mousedx, mousedy;
static uint8_t mousebuttons;
static uint8_t mousemod;

static uint_fast8_t buf[32];

static struct s_queue kqueue = {
    buf, buf, buf, sizeof(buf), 0, sizeof(buf) / 2
};

/* Queue a character to the input device */
void queue_input(uint_fast8_t c)
{
    insq(&kqueue, c);
    wakeup(&kqueue);
}

static int8_t clampdec(int16_t *v)
{
    int8_t r;
    if (*v > 127) {
        *v -= 127;
        return 127;
    }
    if (*v < -127) {
        *v += 127;
        return -127;
    }
    r = *v;
    return r;
}

int plt_input_read(uint8_t *slot)
{
    uint8_t r, k;
    if (remq(&kqueue, &r)) {
        remq(&kqueue, &k);
	*slot++ = KEYPRESS_CODE | r;
	*slot++ = k;
	return 2;
    }
    if (mousemod) {
        *slot++ = MOUSE_REL;
        *slot++ = mousebuttons;
        *slot++ = clampdec(&mousedx);
        *slot++ = clampdec(&mousedy);
        mousemod = !!(mousedx | mousedy);
        return 4;
    }
    return 0;
}

void plt_input_wait(void)
{
    psleep(&kqueue);
}

int plt_input_write(uint_fast8_t flag)
{
    flag;
    udata.u_error = EINVAL;
    return -1;
}

void poll_input(void)
{
	if (mouse_present) {
		mousescan();
		mousedy += mouse12(mousein + 1);
		mousedx += mouse12(mousein + 4);
		if (mousebuttons != *mousein || mousedy || mousedx) {
			mousebuttons = *mousein;
			mousemod = 1;
			wakeup(&kqueue);
		}
	}

}
