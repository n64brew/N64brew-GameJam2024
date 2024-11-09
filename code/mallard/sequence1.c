#include <libdragon.h>
#include "sequence1.h"

#define NUM_DISPLAY 3
#define FONT_TEXT 1
#define BACKGROUND 0x000000FF
#define SEQUENCE1_DURATION 1.0f

///////////////////////////////////////////////////////////
//                  Globals                              //
///////////////////////////////////////////////////////////

rdpq_font_t *font;
sprite_t *libdragon;

bool sequence1_initialized = false;
float sequence1_duration = 0.0f;

void sequence1_init()
{
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, NUM_DISPLAY, GAMMA_NONE, FILTERS_RESAMPLE);
    libdragon = sprite_load("rom:/mallard/libdragon.rgba32.sprite");
    font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
    rdpq_text_register_font(FONT_TEXT, font);
    sequence1_initialized = true;
}

void sequence1_cleanup()
{
    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_font_free(font);
    sprite_free(libdragon);
    rspq_wait();
    display_close();

    // Reset the state.
    sequence1_initialized = false;
    sequence1_duration = 0.0f;

    // End the sequence.
    sequence_1_libdragon = false;
    sequence_2_ascii = true;
}

void sequence_1(float deltatime)
{
    sequence1_duration += deltatime;

    if (sequence1_duration > SEQUENCE1_DURATION)
    {
        sequence1_cleanup();
        return;
    }

    if (!sequence1_initialized)
        sequence1_init();

    // Render the UI
    rdpq_attach(display_get(), NULL);
    rdpq_clear(color_from_packed32(BACKGROUND));

    // Draw "Made with Libdragon" text
    float x = RESOLUTION_320x240.width / 2 - 42;
    float y = RESOLUTION_320x240.height / 2 - 80;
    rdpq_set_mode_standard();
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, x, y, "Made with Libdragon");

    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_sprite_blit(libdragon, RESOLUTION_320x240.width / 2 - libdragon->width / 2, RESOLUTION_320x240.height / 2 - libdragon->height / 2, NULL);

    rdpq_detach_show();
}