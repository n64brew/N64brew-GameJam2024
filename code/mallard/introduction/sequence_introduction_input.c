#include <libdragon.h>
#include "sequence_introduction.h"
#include "sequence_introduction_input.h"
#include "../../../core.h"
#include "../../../minigame.h"

void sequence_introduction_process_controller(float deltatime)
{
    for (size_t i = 0; i < core_get_playercount(); i++)
    {
        joypad_port_t controllerPort = core_get_playercontroller(i);
        joypad_buttons_t pressed = joypad_get_buttons_pressed(controllerPort);

        if (!joypad_is_connected(controllerPort))
        {
            continue;
        }

        // Skip.
        if (pressed.start)
        {
            sequence_introduction_should_cleanup = true;
        }

        // Intro - Libdragon Logo.
        if (sequence_introduction_libdragon_logo_started == true && sequence_introduction_libdragon_logo_finished == false)
        {
            if (pressed.a)
            {
                sequence_introduction_libdragon_logo_elapsed = DRAW_LIBDRAGON_LOGO_DURATION;
            }
        }

        // Intro - Mallard Logo.
        if (sequence_introduction_mallard_logo_started == true && sequence_introduction_mallard_logo_finished == false)
        {
            if (pressed.a)
            {
                sequence_introduction_mallard_logo_elapsed = DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_OUT_DURATION;
            }
        }

        // Intro - Paragraphs.
        if (sequence_introduction_paragraphs_started == true && sequence_introduction_paragraphs_finished == false)
        {
            if (pressed.a)
            {
                // Speed Up.
                if (sequence_introduction_current_paragraph_finished == false)
                {
                    if (sequence_introduction_current_paragraph_speed == 1)
                    {
                        sequence_introduction_current_paragraph_drawn_characters = strlen(sequence_introduction_current_paragraph_string);
                    }
                    else
                    {
                        sequence_introduction_current_paragraph_speed = 1;
                    }
                }

                if (sequence_introduction_paragraph_fade_out_started == true)
                {
                    sequence_introduction_paragraph_fade_out_elapsed = PARAGRAPH_FADE_OUT_DURATION;
                }

                // Next.
                if (sequence_introduction_current_paragraph_finished == true)
                {
                    sequence_introduction_current_paragraph_speed = DEFAULT_PARAGRAPH_SPEED;
                    sequence_introduction_current_paragraph_finished = false;
                    sequence_introduction_current_paragraph_drawn_characters = 0;
                    sequence_introduction_current_paragraph++;

                    // Allowing music to progress based on progression of paragraphs.
                    switch (sequence_introduction_current_paragraph)
                    {
                    case 0:
                        sequence_introduction_currentXMPattern = 1;
                        break;
                    case 1:
                        sequence_introduction_currentXMPattern = 2;
                        break;
                    case 2:
                        sequence_introduction_currentXMPattern = 3;
                        break;
                    default:
                        sequence_introduction_currentXMPattern = 4;
                        break;
                    }
                }
            }
        }
    }
}