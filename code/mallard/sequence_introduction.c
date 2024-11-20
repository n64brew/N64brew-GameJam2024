#include <libdragon.h>
#include "mallard.h"
#include "sequence_introduction.h"
#include "sequence_introduction_input.h"
#include "sequence_introduction_graphics.h"
#include "../../minigame.h"

///////////////////////////////////////////////////////////
//                  Globals                              //
///////////////////////////////////////////////////////////
sprite_t *sequence_introduction_mallard_libdragon_sprite;
sprite_t *sequence_introduction_mallard_logo_black_sprite;
sprite_t *sequence_introduction_mallard_logo_white_sprite;

sprite_t *sequence_introduction_a_button_sprite;
sprite_t *sequence_introduction_start_button_sprite;

xm64player_t xm;
int sequence_introduction_currentXMPattern = 0;

int sequence_introduction_frame = 0;
bool sequence_introduction_initialized = false;

// Libdragon Logo
bool sequence_introduction_libdragon_logo_started = false;
bool sequence_introduction_libdragon_logo_finished = false;
float sequence_introduction_libdragon_logo_elapsed = 0.0f;

// Mallard Logo
bool sequence_introduction_mallard_logo_started = false;
bool sequence_introduction_mallard_logo_finished = false;
float sequence_introduction_mallard_logo_elapsed = 0.0f;

// Paragraphs
bool sequence_introduction_paragraphs_started = false;
bool sequence_introduction_paragraphs_finished = false;
int sequence_introduction_current_paragraph = 0;
int sequence_introduction_current_paragraph_speed = 4;
bool sequence_introduction_current_paragraph_finished = false;
char *sequence_introduction_current_paragraph_string;
int sequence_introduction_current_paragraph_drawn_characters = 0;

bool sequence_introduction_paragraph_fade_out_started = false;
float sequence_introduction_paragraph_fade_out_elapsed = 0.0f;
bool sequence_introduction_paragraph_fade_out_finished = false;

void sequence_introduction_init()
{
    ///////////////////////////////////////////////////////////
    //                  Set up Display                       //
    ///////////////////////////////////////////////////////////

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);

    ///////////////////////////////////////////////////////////
    //                  Set up Sprites                       //
    ///////////////////////////////////////////////////////////

    // Libdragon
    sequence_introduction_mallard_libdragon_sprite = sprite_load("rom:/mallard/libdragon.rgba32.sprite");

    // Mallard Logo
    sequence_introduction_mallard_logo_black_sprite = sprite_load("rom:/mallard/mallard_logo_black.rgba32.sprite");
    sequence_introduction_mallard_logo_white_sprite = sprite_load("rom:/mallard/mallard_logo_white.rgba32.sprite");

    // Intro UI
    sequence_introduction_start_button_sprite = sprite_load("rom:/core/StartButton.sprite");
    sequence_introduction_a_button_sprite = sprite_load("rom:/core/AButton.sprite");

    ///////////////////////////////////////////////////////////
    //                  Set up Audio                         //
    ///////////////////////////////////////////////////////////

    xm64player_open(&xm, "rom:/mallard/mallard_intro_music.xm64");
    xm64player_play(&xm, 0);
    xm64player_seek(&xm, sequence_introduction_currentXMPattern, 0, 0);

    sequence_introduction_initialized = true;
    sequence_introduction_libdragon_logo_started = true;
}

void sequence_introduction_cleanup()
{
    // Free the sprites.

    // Libdragon
    sprite_free(sequence_introduction_mallard_libdragon_sprite);

    // Mallard Logo
    sprite_free(sequence_introduction_mallard_logo_black_sprite);
    sprite_free(sequence_introduction_mallard_logo_white_sprite);

    // Intro UI
    sprite_free(sequence_introduction_start_button_sprite);
    sprite_free(sequence_introduction_a_button_sprite);

    // Stop the music and free the allocated memory.
    xm64player_stop(&xm);
    xm64player_close(&xm);

    // Close the display and free the allocated memory.
    rspq_wait();
    display_close();

    // Reset the state.
    sequence_introduction_initialized = false;
    sequence_introduction_finished = false;
    // TODO: Check to make sure that we're resetting the state of a lot of things...

    // End the sequence.
    sequence_introduction_finished = true;
    sequence_game_started = true;
}

void sequence_introduction(float deltatime)
{
    sequence_introduction_process_controller(deltatime);

    if (!sequence_introduction_initialized)
    {
        sequence_introduction_init();
    }

    if (sequence_introduction_finished)
    {
        sequence_introduction_cleanup();
        return;
    }

    rdpq_attach(display_get(), NULL);
    rdpq_clear(BLACK);

    ///////////////////////////////////////////////////////////
    //                  Intro Sequence                       //
    ///////////////////////////////////////////////////////////
    sequence_introduction_draw_libdragon_logo(deltatime);
    sequence_introduction_draw_mallard_logo(deltatime);
    sequence_introduction_draw_press_a_for_next();
    sequence_introduction_draw_press_start_to_skip();
    sequence_introduction_draw_paragraph(deltatime);

    rdpq_detach_show();

    ///////////////////////////////////////////////////////////
    //                  Handle Audio                         //
    ///////////////////////////////////////////////////////////

    int patidx, row;

    xm64player_tell(&xm, &patidx, &row, NULL);

    // If the pattern index is greater than the currently allowed pattern, loop back to the start of the currently allowed pattern.
    if (patidx > sequence_introduction_currentXMPattern)
        xm64player_seek(&xm, sequence_introduction_currentXMPattern, 0, 0);

    sequence_introduction_frame++;
}