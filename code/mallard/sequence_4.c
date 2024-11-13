#include <libdragon.h>
#include "sequence_4.h"
#include "sequence_4_input.h"
#include "sequence_4_graphics.h"
#include "../../minigame.h"

#define NUM_DISPLAY 3
#define FONT_TEXT 1

///////////////////////////////////////////////////////////
//                  Globals                              //
///////////////////////////////////////////////////////////

rdpq_font_t *sequence_4_font;
sprite_t *sequence_4_sprite_start_button;

bool sequence_4_initialized = false;
bool sequence_4_finished = false;

void sequence_4_init()
{
    ///////////////////////////////////////////////////////////
    //                  Set up Display                       //
    ///////////////////////////////////////////////////////////

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, NUM_DISPLAY, GAMMA_NONE, FILTERS_RESAMPLE);

    ///////////////////////////////////////////////////////////
    //                  Set up UI Elements                   //
    ///////////////////////////////////////////////////////////

    sequence_4_sprite_start_button = sprite_load("rom:/core/StartButton.sprite");
    sequence_4_font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
    rdpq_text_register_font(FONT_TEXT, sequence_4_font);

    sequence_4_initialized = true;
}

void sequence_4_cleanup()
{
    // Unregister the font and free the allocated memory.
    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_font_free(sequence_4_font);
    sprite_free(sequence_4_sprite_start_button);

    // Close the display and free the allocated memory.
    rspq_wait();
    display_close();

    // Reset the state.
    sequence_4_initialized = false;
    sequence_4_finished = false;

    // End the sequence.
    sequence_4_story = false;
    sequence_5_BLANK = true;
}

void sequence_4(float deltatime)
{
    sequence_4_process_controller(deltatime);

    if (sequence_4_finished)
    {
        sequence_4_cleanup();
        return;
    }

    if (!sequence_4_initialized)
        sequence_4_init();

    // TODO: Implement sequence_4
}