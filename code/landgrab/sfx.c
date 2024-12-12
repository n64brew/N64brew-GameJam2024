#include "sfx.h"
#include "global.h"

#define SFX_MUSIC_CHANNELS 16
#define SFX_SIZE 3

static wav64_t sfx[SFX_SIZE];

#define sfx_channel(sound) ((sound) + SFX_MUSIC_CHANNELS)

static void
sfx_set_limits (SFX sound)
{
  float freq = sfx[sound].wave.frequency;
  mixer_ch_set_limits (sfx_channel (sound), 0, freq, 0);
}

void
sfx_init (void)
{
  wav64_open (&sfx[SFX_BUZZ], "rom:/landgrab/buzz.wav64");
  sfx_set_limits (SFX_BUZZ);

  wav64_open (&sfx[SFX_CLICK], "rom:/landgrab/click.wav64");
  mixer_ch_set_vol (SFX_CLICK + SFX_MUSIC_CHANNELS, 0.5f, 0.5f);
  sfx_set_limits (SFX_CLICK);

  wav64_open (&sfx[SFX_POP], "rom:/landgrab/pop.wav64");
  sfx_set_limits (SFX_POP);
}

void
sfx_cleanup (void)
{
  for (size_t i = 0; i < SFX_SIZE; i++)
    {
      wav64_close (&sfx[i]);
    }
}

void
sfx_play (SFX sound)
{
  wav64_play (&sfx[sound], sfx_channel (sound));
}
