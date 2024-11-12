#include <libdragon.h>
#include "sequence_2.h"
#include "sequence_2_input.h"

#include "ascii.h"

#define SEQUENCE_2_DURATION 3.0f

///////////////////////////////////////////////////////////
//                  Globals                              //
///////////////////////////////////////////////////////////

int sequence_2_frame = 0;
bool sequence_2_initialized = false;
bool sequence_2_finished = false;
float sequence_2_duration = 0.0f;

void sequence_2_init()
{
    rsp_wait(); // TODO: Is this neccessary? If so, why?

    console_init();
    console_set_render_mode(RENDER_MANUAL);
    sequence_2_initialized = true;
}

void sequence_2_cleanup()
{
    console_close();

    // Reset the state.
    sequence_2_initialized = false;
    sequence_2_finished = false;
    sequence_2_duration = 0.0f;
    sequence_2_frame = 0;

    // End the sequence.
    sequence_2_ascii = false;
    sequence_3_video = true;
}

void sequence_2(float deltatime)
{
    sequence_2_process_controller(deltatime);

    sequence_2_duration += deltatime;

    if (sequence_2_finished || sequence_2_duration > SEQUENCE_2_DURATION)
    {
        sequence_2_cleanup();
        return;
    }

    if (!sequence_2_initialized)
        sequence_2_init();

    console_clear();
    puts(strings[sequence_2_frame]);
    console_render();

    if (sequence_2_frame == num_strings - 1)
        sequence_2_frame = sequence_2_frame;
    else
        sequence_2_frame++;
}