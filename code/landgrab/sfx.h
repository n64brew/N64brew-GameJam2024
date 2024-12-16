#ifndef GAMEJAM2024_LANDGRAB_SFX_H
#define GAMEJAM2024_LANDGRAB_SFX_H

typedef enum
{
  SFX_BUZZ,
  SFX_CLICK,
  SFX_POP,
} SFX;

void sfx_init (void);

void sfx_cleanup (void);

void sfx_play (SFX sound);

#endif // GAMEJAM2024_LANDGRAB_SFX_H
