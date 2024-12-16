#ifndef SCREEN_H
#define SCREEN_H

#define SCREEN_WIDTH display_get_width()
#define SCREEN_HEIGHT display_get_height()

typedef struct
{

	surface_t depthBuffer;
	T3DViewport gameplay_viewport;

} Screen;

void screen_initDisplay(Screen *screen);
void screen_clearDisplay(Screen *screen);

void screen_initDisplay(Screen *screen)
{
	display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
	screen->depthBuffer = surface_alloc(FMT_RGBA16, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void screen_clearDisplay(Screen *screen)
{
	rdpq_attach(display_get(), &screen->depthBuffer);
	t3d_frame_start();
}

void screen_initT3dViewport(Screen *screen)
{
	screen->gameplay_viewport = t3d_viewport_create();
}

void screen_clearT3dViewport(Screen *screen)
{
	t3d_viewport_attach(&screen->gameplay_viewport);
}

void screen_applyColor_Depth(Screen *screen, color_t color, bool fog)
{
	if (fog)
	{
		rdpq_mode_fog(RDPQ_FOG_STANDARD);
		rdpq_set_fog_color(color);
	}

	t3d_screen_clear_color(color);
	t3d_screen_clear_depth();

	if (fog)
		t3d_fog_set_range(750.0f, 900.0f);
}

void screen_applyColor(Screen *screen, color_t color, bool fog)
{
	if (fog)
	{
		rdpq_mode_fog(RDPQ_FOG_STANDARD);
		rdpq_set_fog_color(color);
	}
	t3d_screen_clear_color(color);

	if (fog)
		t3d_fog_set_range(750.0f, 900.0f);
}

#endif