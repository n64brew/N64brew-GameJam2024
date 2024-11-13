#include <libdragon.h>
#include "sequence_2.h"
#include "sequence_2_input.h"
#include "sequence_2_graphics.h"

#include "ascii.h"

#define SEQUENCE_2_DURATION 10.0f
#define FONT_TEXT 1

///////////////////////////////////////////////////////////
//                  Globals                              //
///////////////////////////////////////////////////////////

int sequence_2_frame = 0;
bool sequence_2_initialized = false;
bool sequence_2_finished = false;
float sequence_2_duration = 0.0f;
rdpq_font_t *sequence_2_font;
sprite_t *sequence_2_sprite_start_button;

void sequence_2_init()
{
    rsp_wait(); // TODO: Is this neccessary? If so, why?

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);

    sequence_2_sprite_start_button = sprite_load("rom:/core/StartButton.sprite");
    sequence_2_font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
    rdpq_text_register_font(FONT_TEXT, sequence_2_font);

    sequence_2_initialized = true;
}

void sequence_2_cleanup()
{
    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_font_free(sequence_2_font);
    sprite_free(sequence_2_sprite_start_button);
    rspq_wait();
    display_close();

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

    //////////////////// render ASCII ////////////////////

    // rdpq_attach(display_get(), NULL);

    rdpq_attach_clear(display_get(), NULL);

    const char *text = strings[sequence_2_frame];
    int length = strlen(text);
    int line_length = 64;
    int num_lines = (length + line_length - 1) / line_length; // Calculate the number of lines needed

    for (int i = 0; i < num_lines; i++)
    {
        char line[65]; // Buffer to hold each line, plus null terminator
        strncpy(line, text + i * line_length, line_length);
        line[line_length] = '\0'; // Null-terminate the string

        for (int j = 0; j < strlen(line); j++)
        {
            // Process each character in the line buffer
            char c = line[j];
            // You can add your character processing code here
            rdpq_set_mode_standard();

            char str[2];
            str[0] = c;
            str[1] = '\0'; // Null terminator

            rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, j * 5 - 10, i * 8 + 20, str);
        }
    }

    if (sequence_2_frame == num_strings - 1)
        sequence_2_frame = sequence_2_frame;
    else
        sequence_2_frame++;

    //////////////////////////////////////////////////////

    // int length = strlen(text);
    // for (int i = 0; i < length; i += 64)
    // {
    //     // Print a substring of `chunk_size` characters
    //     fprintf(stderr, "%.64s\n", &text[i]);
    // }

    // if (sequence_2_frame == num_strings - 1)
    //     sequence_2_frame = sequence_2_frame;
    // else
    //     sequence_2_frame++;

    //////////////////////////////////////////////////////

    // rdpq_set_mode_fill(RGBA32(0x30, 0x63, 0x8E, 0xFF));
    // rdpq_fill_rectangle((320 - box_width) / 2, (240 - box_height) / 2, (320 + box_width) / 2, (240 + box_height) / 2);

    // disable_interrupts();

    // int nbytes = strlen(text);
    // rdpq_paragraph_t *par = rdpq_paragraph_build(&(rdpq_textparms_t){
    //                                                  // .line_spacing = -3,
    //                                                  .align = ALIGN_LEFT,
    //                                                  .valign = VALIGN_CENTER,
    //                                                  .width = screen->width,
    //                                                  .height = screen->height,
    //                                                  .wrap = WRAP_WORD,
    //                                              },
    //                                              FONT_TEXT, text, &nbytes);

    // rdpq_paragraph_render(par, 0, 0);
    // rdpq_paragraph_free(par);

    // enable_interrupts();

    // rdpq_set_mode_standard();
    // rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    // rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    // rdpq_set_prim_color(RGBA32(0xFF, 0xFF, 0xFF, 0x30));
    // rdpq_fill_rectangle(par->bbox.x0 + x0, par->bbox.y0 + y0, par->bbox.x1 + x0, par->bbox.y1 + y0);

    // graphics_draw_text(screen, 10, 30, "Additional Content"); // New text on next frame

    // rdpq_detach_show();

    //////////////////////////////////////////////////////

    // if (sequence_2_frame == num_strings - 1)
    //     sequence_2_frame = sequence_2_frame;
    // else
    //     sequence_2_frame++;

    // surface_t *surface = display_get();
    // graphics_draw_text(surface, 10, 30, "Additional Content"); // New text on next frame
    // display_show(surface);

    // Draw "Press Start" text and the Start button
    sequence_2_draw_press_start_to_skip();

    rdpq_detach_show();
}
