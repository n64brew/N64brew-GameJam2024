#include <libdragon.h>
#include "sequence2.h"
#include "ascii.h"

bool sequence2_initialized = false;
float sequence2_duration = 0.0f;
int frame = 0;

void sequence2_init()
{
    console_init();
    console_set_render_mode(RENDER_MANUAL);
    sequence2_initialized = true;
}

void sequence2_cleanup()
{
    console_close();
    sequence2_initialized = false;
    sequence2_duration = 0.0f;
    sequence_2_ascii = false;
    sequence_1_libdragon = true;
}

void sequence_2(float deltatime)
{
    sequence2_duration += deltatime;

    if (sequence2_duration > 3.0f)
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
        frame = 0;
    else
        frame++;
}