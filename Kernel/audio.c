/*
 *	FUZIX audio core support
 */

#include <kernel.h>
#include <kdata.h>
#include <printf.h>
#include <audio.h>

#ifdef CONFIG_AUDIO

struct sound sound;

int audio_ioctl(uarg_t op, void *val)
{
  uint8_t i;

  if ((op >> 8) != 0x02)
    return -EINVAL;
  switch(op) {
    case AUDIOC_GETINFO:
      return uput(&audio_info, val, sizeof(audio_info));
    case AUDIOC_STOP:
      for (i = 0; i < audio_info.channels;i++)
        devaudio_stop(i);
      return 0;
    case AUDIOC_WAIT:
      for (i = 0; i < audio_info.channels;i++)
        devaudio_wait(i);
      return 0;
    case AUDIOC_PLAY:
      if (uget(val, &sound, sizeof(sound)) == -1)
        return -1;
      if (sound.channel > audio_info.channels) {
        udata.u_error = ERANGE;
        return -1;
      }
      if (sound.flags & SND_WAIT)
        if (devaudio_wait(sound.channel))
          return -1;
      return devaudio_play();
    default:
      return devaudio_ioctl(op, val);
  }
}

void audio_tick(void)
{
  /* For now no core processing */
  devaudio_tick();
}

/* TODO: read/write for DSP devices */

#endif
