#include <libdragon.h>
#include "sequence2.h"
#include "ascii.h"

#define SEQUENCE2_DURATION 3.0f

///////////////////////////////////////////////////////////
//                  Globals                              //
///////////////////////////////////////////////////////////

int frame = 0;

bool sequence2_initialized = false;
float sequence2_duration = 0.0f;

void sequence2_init()
{
    console_init();
    console_set_render_mode(RENDER_MANUAL);
    sequence2_initialized = true;
}

void sequence2_cleanup()
{
    console_close();

    // Reset the state.
    sequence2_initialized = false;
    sequence2_duration = 0.0f;
    frame = 0;

    // End the sequence.
    sequence_2_ascii = false;
    sequence_3_video = true;
}

void sequence_2(float deltatime)
{
    sequence2_duration += deltatime;

    if (sequence2_duration > SEQUENCE2_DURATION)
    {
        sequence2_cleanup();
        return;
    }

    if (!sequence2_initialized)
        sequence2_init();

    console_clear();
    puts(strings[frame]);
    console_render();

    if (frame == num_strings - 1)
        frame = frame;
    else
        frame++;
}