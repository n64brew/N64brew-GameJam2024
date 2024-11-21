#include <libdragon.h>
#include <stdlib.h>
#include "mallard.h"
#include "sequence_game.h"
#include "sequence_game_graphics.h"
#include "../../core.h"
#include "../../minigame.h"

// These player boxes are just used for visualizing the boxes and spawn points while developing.
#define PLAYER_1_BOX_X1 16
#define PLAYER_1_BOX_Y1 135
#define PLAYER_1_BOX_X2 168
#define PLAYER_1_BOX_Y2 220

#define PLAYER_2_BOX_X1 168
#define PLAYER_2_BOX_Y1 320
#define PLAYER_2_BOX_X2 288
#define PLAYER_2_BOX_Y2 220

#define PLAYER_3_BOX_X1 16
#define PLAYER_3_BOX_Y1 50
#define PLAYER_3_BOX_X2 168
#define PLAYER_3_BOX_Y2 135

#define PLAYER_4_BOX_X1 168
#define PLAYER_4_BOX_Y1 50
#define PLAYER_4_BOX_X2 320
#define PLAYER_4_BOX_Y2 135

// These boxesa are the actual spawn points for the characters.
#define PLAYER_1_SPAWN_X1 5   // Confirmed
#define PLAYER_1_SPAWN_Y1 111 // Confirmed
#define PLAYER_1_SPAWN_X2 152 // Confirmed
#define PLAYER_1_SPAWN_Y2 194 // Confirmed

#define PLAYER_2_SPAWN_X1 5 + 152        // Confirmed
#define PLAYER_2_SPAWN_Y1 111            // Confirmed
#define PLAYER_2_SPAWN_X2 152 + 152 - 15 // Confirmed
#define PLAYER_2_SPAWN_Y2 194            // Confirmed

#define PLAYER_3_SPAWN_X1 5            // Confirmed
#define PLAYER_3_SPAWN_Y1 111 - 85     // Confirmed
#define PLAYER_3_SPAWN_X2 152          // Confirmed
#define PLAYER_3_SPAWN_Y2 194 - 85 - 1 // Confirmed

#define PLAYER_4_SPAWN_X1 5 + 152        // Confirmed
#define PLAYER_4_SPAWN_Y1 111 - 85       // Confirmed
#define PLAYER_4_SPAWN_X2 152 + 152 - 15 // Confirmed
#define PLAYER_4_SPAWN_Y2 194 - 85 - 1   // Confirmed

float sequence_game_fade_in_elapsed = 0.0f;
float sequence_game_start_held_elapsed = 0.0f;
int sequence_game_player_holding_start = -1;

struct Character
{
    unsigned int x;
    unsigned int y;
};

struct Character *characters;

int random_between(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

void sequence_game_free_players()
{
    if (characters != NULL)
    {
        free(characters);
        characters = NULL;
    }
}

void sequence_game_render_players()
{
    if (characters == NULL)
    {
        int player_count = core_get_playercount();
        if (player_count <= 0)
        {
            return; // Avoid division by zero
        }

        characters = malloc(player_count * sizeof(struct Character));

        for (size_t i = 0; i < player_count; i++)
        {
            switch (i)
            {
            case 0:
                characters[i].x = random_between(PLAYER_1_SPAWN_X1, PLAYER_1_SPAWN_X2);
                characters[i].y = random_between(PLAYER_1_SPAWN_Y1, PLAYER_1_SPAWN_Y2);
                break;
            case 1:
                characters[i].x = random_between(PLAYER_2_SPAWN_X1, PLAYER_2_SPAWN_X2);
                characters[i].y = random_between(PLAYER_2_SPAWN_Y1, PLAYER_2_SPAWN_Y2);
                break;
            case 2:
                characters[i].x = random_between(PLAYER_3_SPAWN_X1, PLAYER_3_SPAWN_X2);
                characters[i].y = random_between(PLAYER_3_SPAWN_Y1, PLAYER_3_SPAWN_Y2);
                break;
            default:
                characters[i].x = PLAYER_4_SPAWN_X2;
                characters[i].y = PLAYER_4_SPAWN_Y1;
                // characters[i].x = random_between(PLAYER_4_SPAWN_X1, PLAYER_4_SPAWN_X2);
                // characters[i].y = random_between(PLAYER_4_SPAWN_Y1, PLAYER_4_SPAWN_Y2);
                break;
            }
        }
    }

    for (size_t i = 0; i < core_get_playercount(); i++)
    {
        int sequence_game_mallard_idle_frame = (sequence_game_frame >> 3) % SEQUENCE_GAME_MALLARD_IDLE_FRAMES;
        rdpq_blitparms_t blitparms = {
            .s0 = sequence_game_mallard_idle_frame * 32,
            .t0 = 0,
            .width = 32,
            .height = 32,
            .flip_x = true,
        };
        rdpq_mode_push();
        rdpq_set_mode_standard();
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);

        rdpq_sprite_blit(sequence_game_mallard_idle_sprite,
                         characters[i].x,
                         characters[i].y,
                         &blitparms);
        rdpq_mode_pop();
    }
}

void sequence_game_draw_background_lakeview_terrace()
{
    rdpq_mode_push();
    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_sprite_blit(sequence_game_background_lakeview_terrace_sprite,
                     0,
                     0,
                     NULL);
    rdpq_mode_pop();
}

void sequence_game_draw_press_start_to_pause()
{
    if (sequence_game_started == true && sequence_game_finished == false)
    {
        rdpq_mode_push();
        rdpq_set_mode_standard();
        rdpq_mode_alphacompare(1);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY_CONST);
        rdpq_set_fog_color(RGBA32(0, 0, 0, (int)(255.0f * 0.5f)));
        // Draw "Start" button
        rdpq_sprite_blit(sequence_game_start_button_sprite,
                         RESOLUTION_320x240.width - sequence_game_start_button_sprite->width - 36,
                         RESOLUTION_320x240.height - sequence_game_start_button_sprite->height - 1,
                         NULL);
        rdpq_mode_pop();

        // Draw "Pause" text
        rdpq_text_print(NULL, FONT_HALODEK, RESOLUTION_320x240.width - 34, RESOLUTION_320x240.height - 5, "$01^01Pause");
    }
}

void sequence_game_draw_paused()
{
    float x = powf(sequence_game_start_held_elapsed, 3) * ((((float)rand() / (float)RAND_MAX) * 2.0f) - 1.0f);
    float y = powf(sequence_game_start_held_elapsed, 3) * ((((float)rand() / (float)RAND_MAX) * 2.0f) - 1.0f);
    float percentage = sequence_game_start_held_elapsed / GAME_EXIT_DURATION > 1.0 ? 1.0 : sequence_game_start_held_elapsed / GAME_EXIT_DURATION;

    // COLOR
    char *utf8_text = "$02^01PAUSED";
    if (sequence_game_player_holding_start == 0)
        utf8_text = "$02^01PAUSED";
    else if (sequence_game_player_holding_start == 1)
        utf8_text = "$02^02PAUSED";
    else if (sequence_game_player_holding_start == 2)
        utf8_text = "$02^03PAUSED";
    else if (sequence_game_player_holding_start == 3)
        utf8_text = "$02^04PAUSED";

    rdpq_set_scissor(0, 0, 70 + x + (180.0f * percentage), 240);
    rdpq_text_print(NULL, FONT_HALODEK_BIG, 70 + x, 140 + y, utf8_text);

    // // WHITE
    rdpq_set_scissor(70 + x + (180.0f * percentage), 0, 320, 240);
    rdpq_text_print(NULL, FONT_HALODEK_BIG, 70 + x, 140 + y, "$02^00PAUSED");
}

void sequence_game_render(float deltatime)
{
    if (sequence_game_started == true && sequence_game_finished == false)
    {
        sequence_game_draw_background_lakeview_terrace();

        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_set_prim_color(RGBA32(0, 0, 0, 128));
        rdpq_fill_rectangle(PLAYER_1_SPAWN_X1, PLAYER_1_SPAWN_Y1, PLAYER_1_SPAWN_X2, PLAYER_1_SPAWN_Y2);

        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_set_prim_color(RGBA32(PLAYERCOLOR_1.r, PLAYERCOLOR_1.g, PLAYERCOLOR_1.b, 64));
        rdpq_fill_rectangle(PLAYER_1_BOX_X1, PLAYER_1_BOX_Y1, PLAYER_1_BOX_X2, PLAYER_1_BOX_Y2);

        rdpq_set_prim_color(RGBA32(PLAYERCOLOR_2.r, PLAYERCOLOR_2.g, PLAYERCOLOR_2.b, 64));
        rdpq_fill_rectangle(PLAYER_2_BOX_X1, PLAYER_2_BOX_Y1, PLAYER_2_BOX_X2, PLAYER_2_BOX_Y2);

        rdpq_set_prim_color(RGBA32(PLAYERCOLOR_3.r, PLAYERCOLOR_3.g, PLAYERCOLOR_3.b, 64));
        rdpq_fill_rectangle(PLAYER_3_BOX_X1, PLAYER_3_BOX_Y1, PLAYER_3_BOX_X2, PLAYER_3_BOX_Y2);

        rdpq_set_prim_color(RGBA32(PLAYERCOLOR_4.r, PLAYERCOLOR_4.g, PLAYERCOLOR_4.b, 64));
        rdpq_fill_rectangle(PLAYER_4_BOX_X1, PLAYER_4_BOX_Y1, PLAYER_4_BOX_X2, PLAYER_4_BOX_Y2);

        sequence_game_render_players();

        if (sequence_game_paused == false)
            sequence_game_draw_press_start_to_pause();

        if (sequence_game_paused == true)
            sequence_game_draw_paused();

        // Fade in
        if (sequence_game_fade_in_elapsed < GAME_FADE_IN_DURATION)
        {
            float percentage = sequence_game_fade_in_elapsed > GAME_FADE_IN_DURATION ? 1.0f : sequence_game_fade_in_elapsed / GAME_FADE_IN_DURATION;
            uint8_t alpha = (int)(255.0f * (1.0f - percentage));
            rdpq_mode_push();
            rdpq_set_mode_standard();
            rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            rdpq_set_prim_color(RGBA32(0, 0, 0, alpha));
            rdpq_fill_rectangle(0, 0, RESOLUTION_320x240.width, RESOLUTION_320x240.height);
            rdpq_mode_pop();
        }

        sequence_game_fade_in_elapsed += deltatime;
    }
    else
    {
        sequence_game_free_players(); // Free memory when the game ends.
    }
}