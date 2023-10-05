/*
 *	Audio support for the NC100. Pretty minimal
 */

#include <kernel.h>
#include <audio.h>

/* Simple two channel beeper style ton generator with no volume control */
const struct audio audio_info = {
	AUDIO_BEEPER,
	2,
	0
};

static uint8_t audio_count[2];

static inline uint16_t devaudio_map(uint16_t hz)
{
	/* The given equation is freq = 1000000/(rate * 2 * 1.6276) */
	/* ie rate = 307201/freq */
	return 307201UL/hz;
}

/* The sound core will call this when it wants to stop a sound */
void devaudio_stop(uint8_t channel)
{
	out(0x51 + 2 * channel, 0xFF);
	audio_count[channel] = 0;
	wakeup(&audio_count[channel]);
}

/* The NC100 has two tone generators, no volume control, nothing fancy
   at all */
int devaudio_play(void)
{
	uint16_t rate;
	audio_count[sound.channel] = sound.time;
	rate = devaudio_map(sound.freq);
	if (sound.volume > 32) {		/* Arbitrary low = off */
		out(0x50 + 2 * sound.channel, rate & 0xFF);
		out(0x51 + 2 * sound.channel, rate >> 8);
	}
	return 0;
}

int devaudio_ioctl(uarg_t op, void *val)
{
	used(op);
	used(val);
	return -EINVAL;
}

void devaudio_tick(void)
{
	if (audio_count[0]) {
		if (!--audio_count[0]) {
			out(0x51, 0xFF);
			wakeup(&audio_count[0]);
		}
	}
	if (audio_count[1]) {
		if (!--audio_count[1]) {
			out(0x53, 0xFF);
			wakeup(&audio_count[1]);
		}
	}
}

/* FIXME? - worth making a pwait_irq or similar 'wait for an IRQ to clear
   a flag' function ? */
int devaudio_wait(uint8_t channel)
{
	uint8_t *p = &audio_count[channel];
	irqflags_t irq = di();

	while(*p) {
		if (psleep_flags(p,0) == -1) {
			irqrestore(irq);
			return -1;
		}
	}
	irqrestore(irq);
	return 0;
}
