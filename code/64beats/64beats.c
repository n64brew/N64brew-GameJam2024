/***************************************************************
                         examplegame.c

An example minigame to demonstrate how to use the template for
the game jam.
***************************************************************/

#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "64beats.h"

#define FONT_TEXT 1

#define COUNTDOWN_DELAY 3.0f
#define GO_DELAY 1.0f
#define WIN_DELAY 5.0f
#define WIN_SHOW_DELAY 2.0f

#define POINTS_TO_WIN 150
#define POINTS_PER_PRESS 6

#define GAME_BACKGROUND 0x222222FF
#define POWERBAR_BACKGROUND 0x333333FF
#define POWERBAR_FOREGROUND 0xEEEEEEFF


static arrow up;
static arrow ui;
static sprite_t *arrow_sprite;

static sprite_t *arrow_up_sprite;
static sprite_t *arrow_down_sprite;
static sprite_t *arrow_left_sprite;
static sprite_t *arrow_right_sprite;

static sprite_t *arrow_sprites[4];



/*********************************
             Globals
*********************************/

// You need this function defined somewhere in your project
// so that the minigame manager can work
const MinigameDef minigame_def = {
    .gamename = "64Beats",
    .developername = "JvPeek",
    .description = "Beat saber but for boomers", // thanks to rieckz
    .instructions = "Push A to win."};

rdpq_font_t *font;

uint32_t player_points[MAXPLAYERS];
uint32_t ai_press_timer[MAXPLAYERS];

float countdown_timer;
bool is_ending;
float end_timer;

wav64_t sfx_start;
wav64_t sfx_countdown;
wav64_t sfx_stop;
wav64_t sfx_winner;

bool has_player_won(PlyNum player)
{
    return player_points[player] >= POINTS_TO_WIN;
}

bool is_countdown()
{
    return countdown_timer > 0.0f;
}

bool can_control()
{
    return !(is_ending || is_countdown());
}

uint32_t ai_get_ticks_until_next_press()
{
    const float min_ticks[] = {2.8f, 2.4f, 2.0f}; // Minimum delay in ticks by difficulty
    const float max_ticks[] = {8.4f, 7.7f, 7.0f}; // Maximum delay in ticks by difficulty

    float random = (float)rand() / RAND_MAX;

    int diff = core_get_aidifficulty();
    return min_ticks[diff] + random * (max_ticks[diff] - min_ticks[diff]);
}

/*==============================
    minigame_init
    The minigame initialization function
==============================*/

void minigame_init()
{
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
    rdpq_text_register_font(FONT_TEXT, font);

    for (size_t i = 0; i < MAXPLAYERS; i++)
    {
        player_points[i] = 0;
        ai_press_timer[i] = ai_get_ticks_until_next_press();
    }

    countdown_timer = COUNTDOWN_DELAY;
    wav64_open(&sfx_start, "rom:/core/Start.wav64");
    wav64_open(&sfx_countdown, "rom:/core/Countdown.wav64");
    wav64_open(&sfx_stop, "rom:/core/Stop.wav64");
    wav64_open(&sfx_winner, "rom:/core/Winner.wav64");
    arrow_up_sprite = sprite_load("rom:/64beats/up.rgba32.sprite");
    arrow_down_sprite = sprite_load("rom:/64beats/down.rgba32.sprite");
    arrow_left_sprite = sprite_load("rom:/64beats/left.rgba32.sprite");
    arrow_right_sprite = sprite_load("rom:/64beats/right.rgba32.sprite");
    arrow_sprite = sprite_load("rom:/64beats/arrow.sprite");

    arrow_sprites[0] = arrow_left_sprite;
    arrow_sprites[1] = arrow_up_sprite;
    arrow_sprites[2] = arrow_down_sprite;
    arrow_sprites[3] = arrow_right_sprite;
    ui.scale_factor_x = UI_SCALE;
    ui.scale_factor_y = UI_SCALE;
    loadSong();
    
}

/*==============================
    minigame_fixedloop
    Code that is called every loop, at a fixed delta time.
    Use this function for stuff where a fixed delta time is
    important, like physics.
    @param  The fixed delta time for this tick
==============================*/

void minigame_fixedloop(float deltatime)
{
    bool couldcontrol = can_control();
    if (countdown_timer > -GO_DELAY)
    {
        float prevtime = countdown_timer;
        countdown_timer -= deltatime;
        if ((int)prevtime != (int)countdown_timer && countdown_timer >= 0)
            wav64_play(&sfx_countdown, 31);
    }

    if (is_ending)
    {
        float prevendtime = end_timer;
        end_timer += deltatime;
        if ((int)prevendtime != (int)end_timer && (int)end_timer == WIN_SHOW_DELAY)
            wav64_play(&sfx_winner, 31);
        if (end_timer > WIN_DELAY)
            minigame_end();
    }

    if (!can_control())
        return;
    if (!couldcontrol && can_control())
        wav64_play(&sfx_start, 31);

    for (size_t i = 0; i < MAXPLAYERS; i++)
    {
        // Subtract "point drain" for all players at fixed rate
        if (player_points[i] > 0)
            player_points[i] -= 1;

        if (i < core_get_playercount())
            continue;

        // For AI players, wait for a random number of ticks until the next A press
        ai_press_timer[i] -= 1;
        if (ai_press_timer[i] == 0)
        {
            player_points[i] += POINTS_PER_PRESS;
            ai_press_timer[i] = ai_get_ticks_until_next_press();
        }
    }

    // Check if anyone has won
    for (size_t i = 0; i < MAXPLAYERS; i++)
    {
        if (has_player_won(i))
        {
            core_set_winner(i);
            is_ending = true;
            wav64_play(&sfx_stop, 31);
        }
    }
}
double degreesToRadians(double degrees)
{
    const double PI = 3.14159265358979323846; // Approximation of π
    return degrees * PI / 180.0;
}
/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/

void minigame_loop(float deltatime)
{
    songTime += deltatime * 1000;
    static int rotation = 0;
    if (can_control())
    {
        // Handle button presses of human players in variable step loop so the input feels more responsive
        for (size_t i = 0; i < core_get_playercount(); i++)
        {
            // For human players, check if the physical A button on the controller was pressed
            joypad_buttons_t btn = joypad_get_buttons_pressed(core_get_playercontroller(i));
            
            if (btn.a)
                player_points[i] += POINTS_PER_PRESS;
        }
    }
    // Render the UI
    rdpq_attach(display_get(), NULL);
    rdpq_clear(color_from_packed32(GAME_BACKGROUND));

    
    // if (is_countdown())
    // {
    //     // Draw countdown
    //     rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 155, 100, "%d", (int)ceilf(countdown_timer));
    // }
    // else if (countdown_timer > -GO_DELAY)
    // {
    //     // For a short time after countdown is over, draw "GO!"
    //     rdpq_text_print(NULL, FONT_BUILTIN_DEBUG_MONO, 150, 100, "GO!");
    // }
    // else if (is_ending && end_timer >= WIN_SHOW_DELAY)
    // {
    //     // Draw winner announcement (There might be multiple winners)
    //     int ycur = 100;
    //     for (size_t i = 0; i < MAXPLAYERS; i++)
    //     {
    //         if (!has_player_won(i))
    //             continue;
    //         ycur += rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 120, ycur, "Player %d wins!\n", i + 1).advance_y;
    //     }
    // }

    
    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    updateArrows(); // TODO
    checkInputs();// TODO
    
    drawUI();
    drawArrows();

    updateArrowList();
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 10, 10, "FPS: %f", 1.0 / deltatime);
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 10, 230, "TIME: %ld", songTime);

    rdpq_detach_show();
}
void updateArrows() {
    
}
void checkInputs() {


    uint32_t playercount = core_get_playercount();
    for (size_t i = 0; i < playercount; i++) {
        joypad_buttons_t btn = joypad_get_buttons_pressed(core_get_playercontroller(i));
        //debugf("Loading minigame: %s\n", name);
        if (btn.c_up) {
            if (myTrack.arrows[currentTargetArrow].direction == ARR_UP) {
                wav64_play(&sfx_countdown, 31);
            }
        }
        if (btn.c_left) {
            if (myTrack.arrows[currentTargetArrow].direction == ARR_LEFT) {
                wav64_play(&sfx_countdown, 31);
            }
        }
        if (btn.c_right) {
            if (myTrack.arrows[currentTargetArrow].direction == ARR_RIGHT) {
                wav64_play(&sfx_countdown, 31);
            }
        }
        if (btn.c_down) {
            if (myTrack.arrows[currentTargetArrow].direction == ARR_LEFT) {
                wav64_play(&sfx_countdown, 31);
            }
        }
    }
    
            
}
int countValidEntries() {
    int count = 0;
    for (int i = 0; i < MAX_ARROWS; i++) {
        if (myTrack.arrows[i].time != -1) {
            count++;
        }
    }
    return count;
}
void loadSong() {
 // Fill the array with data
    
    for (int i = 0; i < MAX_ARROWS; i++) {
        float random = (float)rand() / (RAND_MAX / 4.0);
        myTrack.arrows[i].time = i * (60000 / 174);
        myTrack.arrows[i].direction = (uint8_t) random;
        myTrack.arrows[i].difficulty = 1;
    }
}
void updateArrowList() {


}
int calculateXForArrow(uint8_t playerNum, uint8_t dir) {
    int paddingArrow = 10;
    #define SCREEN_WIDTH 320
    #define SCREEN_MIDDLE SCREEN_WIDTH/2
    #define WIDTH_PER_PLAYER SCREEN_WIDTH/4
    const uint8_t arrowWidthScaled = arrow_sprite->width * UI_SCALE;
    const uint16_t playerSlotStart = WIDTH_PER_PLAYER * playerNum;
    const uint16_t playerSlotMiddle = playerSlotStart + (WIDTH_PER_PLAYER / 2);
    const uint16_t arrowPosInSlot = playerSlotMiddle + (-1.5 + dir) * arrowWidthScaled ;
    return (int)arrowPosInSlot;
}
int calculateYForArrow(int time) {
    return 0;
}
void drawArrowForPlayer(uint8_t playerNum, int yPos, uint8_t dir) {

    rdpq_sprite_blit(arrow_sprites[dir],
                            (int32_t)(calculateXForArrow(playerNum, dir)),
                            (int32_t)(yPos),
                            &(rdpq_blitparms_t){
                                .cx = (arrow_sprite->width / 2),
                                .cy = (arrow_sprite->height / 2),
                                .scale_x = ui.scale_factor_x,
                                .scale_y = ui.scale_factor_y,
                            });
    //rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO,(int32_t)(calculateXForArrow(playerNum, dir)), (int32_t)(yPos), "TIME: %d", timeDelta);
}
void drawArrows() {
    static int arrowsStart = 0;
    int arrowsEnd = (arrowsStart + 50 > MAX_ARROWS) ? MAX_ARROWS : arrowsStart + 50;
    currentTargetArrow = arrowsStart;
    for (int i = arrowsStart; i < arrowsEnd; i++) {
        int timeDelta = songTime - myTrack.arrows[i].time;

        int yPos = 240 - timeDelta / 5;
        if (yPos > 240 + arrow_sprite->height) {
            continue;
        }
        if (yPos < 0 - arrow_sprite->height) {
            arrowsStart = i;
            continue;

        }
        
        for (uint8_t thisPlayer=0; thisPlayer<4; thisPlayer++) {
            // TODO: check myTrack.arrows[i].difficulty
            
            drawArrowForPlayer(thisPlayer,yPos , myTrack.arrows[i].direction);
            
        }
            
        
    }
    
    
}
void drawUI() {
    for (uint8_t thisPlayer=0; thisPlayer<4; thisPlayer++) {
        for (uint8_t thisDirection = 0; thisDirection < 4; thisDirection++) {
            drawUIForPlayer(thisPlayer, thisDirection);
        }
    }
}

void drawUIForPlayer(uint8_t playerNum, uint8_t dir) {
    const uint8_t rotationLookup[4] = {1, 0, 2, 3};
    rdpq_sprite_blit(arrow_sprite,
                            (int32_t)(calculateXForArrow(playerNum, dir)),
                            (int32_t)(SCREEN_MARGIN_TOP),
                            &(rdpq_blitparms_t){
                                .cx = (arrow_sprite->width / 2),
                                .cy = (arrow_sprite->height / 2),
                                .theta = degreesToRadians( 90 * rotationLookup[dir] ),
                                .scale_x = ui.scale_factor_x,
                                .scale_y = ui.scale_factor_y,
                            });
}
/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/

void minigame_cleanup()
{
    wav64_close(&sfx_start);
    wav64_close(&sfx_countdown);
    wav64_close(&sfx_stop);
    wav64_close(&sfx_winner);

    display_close();
    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_font_free(font);
}