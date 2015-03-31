#ifndef _AUDIO_H
#define _AUDIO_H

struct sound {
  uint8_t channel;
  uint8_t time;				/* 1/10th second */
  uint16_t freq;			/* Hz */
  uint8_t flags;
#define SND_NOISE_ON	1		/* Enable noise on this channel */
#define SND_WAIT	2		/* Wait for existing sound */
  uint8_t volume;
  uint16_t envclock;			/* Hz */
  uint8_t adsrtime[4];			/* in env clocks */
  uint8_t adsrvol[4];			/* volume ramps */
};


struct audio {
  uint8_t type;
#define AUDIO_BEEPER		1	/* Generic beeper */
#define AUDIO_CLICK		2	/* One bit speaker control */
#define AUDIO_AY38910		128	/* And 39811/2/etc */
#define AUDIO_SN76489		129
  uint8_t channels;
  uint16_t flags;
#define SND_DIRECT_IO		1	/* Has ioctls for direct register I/O */
#define SND_ENVELOPE		2	/* Allows envelopes */
#define SND_NOISE		4	/* Allows noise */
#define SND_SAMPLE		8	/* Supports sample playback */
#define SND_SOFT_SAMPLE		16	/* Sample playback is software (slow) */
#define SND_MASTER_VOLUME	32	/* Has a master volume knob */
};


#define AUDIOC_GETINFO		0x0200	/* Query audio features */
#define AUDIOC_PLAY		0x0201	/* Play a sound */
#define AUDIOC_VOLUME		0x0202	/* Master volume adjust if present */
#define AUDIOC_STOP		0x0203	/* Stop any playing sound */
#define AUDIOC_WAIT		0x0204	/* Wait for existing sound */

/* Provided by the device layer */
extern const struct audio audio_info;
extern void devaudio_stop(uint8_t channel);
extern int devaudio_wait(uint8_t channel);
extern int devaudio_play(void);
extern int devaudio_ioctl(uarg_t op, void *val);
extern void devaudio_tick(void);

/* Provided by the audio core */
extern int audio_ioctl(uarg_t op, void *val);
extern struct sound sound;
extern void audio_tick(void);	/* Every 1/10th a second */

#endif
