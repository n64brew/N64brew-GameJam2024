#include <libdragon.h>
#include "sequence_4.h"
#include "sequence_4_input.h"
#include "../../core.h"
#include "../../minigame.h"

void sequence_4_process_controller(float deltatime)
{
    for (size_t i = 0; i < core_get_playercount(); i++)
    {
        joypad_port_t controllerPort = core_get_playercontroller(i);
        joypad_buttons_t pressed = joypad_get_buttons_pressed(controllerPort);

        if (!joypad_is_connected(controllerPort))
        {
            continue;
        }

        // Terminate Minigame.
        if (pressed.start)
        {
            sequence_4_finished = true;
        }

        // Intro - Libdragon.
        if (sequence_4_time <= DRAW_LIBDRAGON_LOGO_DURATION)
        {
            if (pressed.a)
            {
                sequence_4_time = DRAW_LIBDRAGON_LOGO_DURATION;
            }
        }

        // Intro - Mallard.
        if (sequence_4_time > DRAW_LIBDRAGON_LOGO_DURATION &&
            sequence_4_time <= DRAW_LIBDRAGON_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_OUT_DURATION )
        {
            if (pressed.a)
            {
                sequence_4_time = DRAW_LIBDRAGON_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_OUT_DURATION ;
            }
        }

        // Paragraphs.
        if (sequence_4_paragraphs_finished == false && sequence_4_time > DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_OUT_DURATION )
        {
            if (pressed.a)
            {
                // Speed Up.
                if (sequence_4_current_paragraph_finished == false)
                {
                    if (sequence_4_current_paragraph_speed == 1)
                    {
                        sequence_4_current_paragraph_drawn_characters = strlen(sequence_4_current_paragraph_string);
                    }
                    else
                    {
                        sequence_4_current_paragraph_speed = 1;
                    }
                }

                // Next.
                if (sequence_4_current_paragraph_finished == true)
                {
                    sequence_4_current_paragraph_speed = 4;
                    sequence_4_current_paragraph_finished = false;
                    sequence_4_current_paragraph_drawn_characters = 0;
                    sequence_4_current_paragraph++;

                    // Allowing music to progress based on progression of paragraphs.
                    switch (sequence_4_current_paragraph)
                    {
                    case 0:
                        sequence_4_currentXMPattern = 1;
                        break;
                    case 1:
                        sequence_4_currentXMPattern = 2;
                        break;
                    case 2:
                        sequence_4_currentXMPattern = 3;
                        break;
                    default:
                        sequence_4_currentXMPattern = 4;
                        break;
                    }
                }
            }
        }
    }
}