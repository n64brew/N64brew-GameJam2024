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

        // Skip the sequence.
        if (pressed.start)
        {
            sequence_4_finished = true;
        }

        // Paragraph next.
        if (sequence_4_all_paragraphs_finished == false && sequence_4_time > DRAW_MALLARD_LOGO_FADE_IN_DURATION + DRAW_MALLARD_LOGO_DURATION + DRAW_MALLARD_LOGO_FADE_OUT_DURATION + DRAW_FADE_WHITE_TO_BLACK_DURATION)
        {
            if (pressed.a)
            {
                if (sequence_4_current_paragraph_finished == false)
                {
                    if (sequence_4_paragraph_speed == 1)
                    {
                        sequence_4_drawn_characters = strlen(sequence_4_current_paragraph_string);
                    }
                    else
                    {
                        sequence_4_paragraph_speed = 1;
                    }
                }
                // Next Paragraph.
                if (sequence_4_current_paragraph_finished == true)
                {
                    sequence_4_paragraph_speed = 4;
                    sequence_4_current_paragraph_finished = false;
                    sequence_4_drawn_characters = 0;
                    sequence_4_current_paragraph++;

                    // Allowing music to progress based on progression of paragraphs.
                    switch (sequence_4_current_paragraph)
                    {
                    case 0:
                        sequence_4_currentXMPattern = 4;
                        break;
                    case 1:
                        sequence_4_currentXMPattern = 14;
                        break;
                    case 2:
                        sequence_4_currentXMPattern = 18;
                        break;
                    default:
                        sequence_4_currentXMPattern = 23;
                        break;
                    }
                }
            }
        }
    }
}