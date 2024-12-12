#ifndef HYDRA_UI_H
#define HYDRA_UI_H

#include "hydraharmonics.h"

#define PANEL_TEXT_PADDING_X 15
#define PANEL_TEXT_PADDING_Y 19

#define SIGN_HEIGHT 28

extern uint8_t sign_y_offset;

void ui_init (void);
void ui_animate (void);
void ui_panel_draw (float x0, float y0, float x1, float y1);
void ui_signs_draw (void);
void ui_draw (void);
void ui_clear (void);

#endif
