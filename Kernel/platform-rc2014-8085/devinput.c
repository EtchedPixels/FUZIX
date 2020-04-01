#include <kernel.h>
#include <kdata.h>
#include <input.h>
#include <devinput.h>

static unsigned char buf[32];

static struct s_queue kqueue = {
	buf, buf, buf, sizeof(buf), 0, sizeof(buf) / 2
};

/* Queue a character to the input device */
void queue_input(uint_fast8_t c)
{
	insq(&kqueue, c);
	wakeup(&kqueue);
}

static uint8_t js_data[2] = { 255, 255 };

uint_fast8_t read_js(uint8_t * slot, uint_fast8_t n)
{
	uint_fast8_t d = 0;
	uint_fast8_t r;

	if (n == 1)
		r = jsin1();
	else
		r = jsin2();

	r &= 63;
	if (r == js_data[n])
		return 0;
	js_data[n] = r;
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

int platform_input_read(uint8_t * slot)
{
	uint_fast8_t r, k;
	if (remq(&kqueue, &r)) {
		remq(&kqueue, &k);
		*slot++ = KEYPRESS_CODE | r;
		*slot = k;
		return 2;
	}
	if (read_js(slot, 0))
		return 2;
	if (read_js(slot, 1))
		return 2;
	return 0;
}

void platform_input_wait(void)
{
	psleep(&kqueue);	/* We wake this on timers so it works for sticks */
}

void poll_input(void)
{
	if ((jsin1() & 63) != js_data[0] || ((jsin2() & 63) != js_data[1]))
		wakeup(&kqueue);
}

int platform_input_write(uint_fast8_t flag)
{
	flag;
	udata.u_error = EINVAL;
	return -1;
}
