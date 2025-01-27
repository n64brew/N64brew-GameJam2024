/***************************************************************
                         examplegame.c

An example minigame to demonstrate how to use the template for
the game jam.
***************************************************************/

#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "64beats.h"

#ifndef DEBUG
#define DEBUG false
#endif

#define FONT_TEXT 1

#define GAME_BACKGROUND 0x222222FF
#define POWERBAR_BACKGROUND 0x333333FF
#define POWERBAR_FOREGROUND 0xEEEEEEFF

static arrow ui;
static sprite_t *arrow_sprite;

static sprite_t *arrow_up_sprite;
static sprite_t *arrow_down_sprite;
static sprite_t *arrow_left_sprite;
static sprite_t *arrow_right_sprite;

static sprite_t *arrow_sprites[4];

track myTrack;

int currentTargetArrow;


/*********************************
             Globals
*********************************/

// You need this function defined somewhere in your project
// so that the minigame manager can work
const MinigameDef minigame_def = {
    .gamename = "64Beats",
    .developername = "JvPeek",
    .description = "Beat saber but for boomers", // thanks to rieckz
    .instructions = "Press the C-Buttons to match the arrows in time with the music."};

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

uint32_t start_tick = 0;
gamestate gameState;

/*==============================
    minigame_init
    The minigame initialization function
==============================*/

void minigame_init()
{
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
    rdpq_text_register_font(FONT_TEXT, font);


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

    xm64player_open(&music, myTrack.songPath);
    xm64player_set_loop(&music, false);
    xm64player_play(&music, 0);
    

    start_tick = get_ticks();

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
uint32_t get_music_playtime_ms() {
    if (start_tick == 0) {
        return 0; // Playback hasn't started
    }

    // Get the current tick count
    uint32_t current_tick = TICKS_READ();
    uint32_t elapsed_ticks = current_tick - start_tick;

    // Convert ticks to milliseconds using TIMER_MICROS
    return TIMER_MICROS(elapsed_ticks) / 1000;
}

int get_winner()
{
    int currentHighest = 0;
    for (int players=0; players < 4; players++) {
        if (points[currentHighest] <= points[players]) {
            currentHighest = players;
        }
    }
    return currentHighest;
}

void minigame_loop(float deltatime)
{
    // Render the Background
    rdpq_attach(display_get(), NULL);
    //rdpq_clear(color_from_packed32(GAME_BACKGROUND));
    rdpq_set_mode_standard();
    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_set_prim_color(color_from_packed32((songTime % (60000/myTrack.bpm) < 60) ? 0x00000080 : 0x00000009));
    rdpq_fill_rectangle(0, 0, 320, 240);
    

    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
   
    switch (gameState)
    {
    case INTRO:
        songTime = get_music_playtime_ms() - myTrack.introLength;
        checkInputs();
        drawArrows();
        drawUI();
        if (songTime >= -ACCURACY) {
            gameState = RUNNING;
        }
        break;
    case RUNNING:
        songTime = get_music_playtime_ms() - myTrack.introLength;
        checkInputs();
        AIButtons(songTime, deltatime);
        drawArrows();
        drawUI();
        
        if (!music.playing) {
            gameState = OUTRO;
        }
        break;
    case OUTRO:
        renderOutro();
        break;
    case ENDED:
        core_set_winner(get_winner());
        minigame_end();
        break;

    default:
        break;
    }

    rdpq_set_prim_color(color_from_packed32(0xFFFFFFFF));
    if (DEBUG) {
        rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 10, 10, "FPS: %f", 1.0 / deltatime);
        rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 10, 230, "TIME: %ld,\t\tGS: %d", songTime, gameState);
    }
    
    
    rdpq_detach_show();
}
void renderOutro() {
    static uint32_t endTicks = 0;
    if (endTicks == 0) {
        // Get the current tick count
        endTicks = TICKS_READ();
    }

    uint32_t outroRuntime = TICKS_READ() - endTicks;
    if (TIMER_MICROS(outroRuntime) / 1000 > 5000) {
        gameState = ENDED;
        return;
    }
    int currentHighest = get_winner();
    rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 160, 120, "Player %d wins!", currentHighest+1);

    
}
void checkInputs()
{

    uint32_t playercount = core_get_playercount();
    int buttonSum = 0;
    for (size_t i = 0; i < playercount; i++)
    {
        joypad_buttons_t btn = joypad_get_buttons_pressed(core_get_playercontroller(i));
        
        
        if (btn.start) {
            if (music.playing) {
                //xm64player_stop(&music);
            } else {
                //xm64player_play(&music, 0);
            }
            
        }

        if(music.playing) {
            buttonSum += btn.c_left + btn.c_up + btn.c_down + btn.c_right;
        
            if (buttonSum == 0)
            {
                continue;
            }
            bool directionsPressed[4] = {btn.c_left, btn.c_up, btn.c_down, btn.c_right};
            if (currentTargetArrow > myTrack.arrowNum) {
                continue;
            }
            for (int currentArrow = currentTargetArrow; currentArrow < currentTargetArrow + 16; currentArrow++)
            {
                if (!directionsPressed[myTrack.arrows[currentArrow].direction])
                {
                    continue;
                }
                int deltaTime = calculateDeltaTime(currentArrow);
                if (deltaTime > ACCURACY || deltaTime < 0-ACCURACY ) {
                    continue;
                }
                const int addScore = ACCURACY - abs(deltaTime);
                multi[i]++;
                points[i] += addScore * getMulti(i);
                if (DEBUG) {
                    debugf("P%d: scored %d points for a total of %d (DT: %d, Multi: %d)\n", i, addScore, points[i], deltaTime, getMulti(i));
                }
                myTrack.arrows[currentArrow].hit[i] = true;
                directionsPressed[myTrack.arrows[currentArrow].direction] = false;
            }
            if (directionsPressed[0] + directionsPressed[1] + directionsPressed[2] + directionsPressed[3] > 0) {
                multi[i] = 0;
                if (DEBUG) {
                    debugf("P%dDPressed: %d %d %d %d\n", i, directionsPressed[0], directionsPressed[1], directionsPressed[2], directionsPressed[3]);
                }
            }
        }
        
    }

}

void AIButtons(int songTime, float deltatime) {
    static int lastArrow = -1;
    int nextArrow = findNextTimestamp(songTime);
    if (nextArrow == lastArrow) {
        return;
    }
    if (nextArrow - ACCURACY > songTime) {
        return;
    }
    int cutOff = DIFF_HARD - core_get_aidifficulty();
    cutOff = ((cutOff+1) * (ACCURACY/4)) / 4;
    if (DEBUG) {

        debugf("ST: %d, LA: %d, NA: %d, CO: %d\n", songTime, lastArrow, nextArrow, cutOff);
    }
    lastArrow = nextArrow;
    uint32_t playercount = core_get_playercount();
    for (size_t i = playercount; i < 4; i++)
    {

        float random = (float)rand() / (RAND_MAX / ACCURACY);

        if (random > cutOff) {

            multi[i]++;
            points[i] += (int)random * getMulti(i);
        } else {
            multi[i] = 0;
        }
    }

}
int findNextTimestamp(int songTime) {
	int32_t nextTime = INT32_MAX;

	for (int i = 0; i < myTrack.arrowNum; i++) {
		int arrowTime = myTrack.arrows[i].time;
		if (arrowTime > songTime && arrowTime < nextTime) {
			nextTime = arrowTime;
		}
	}

	return (nextTime == INT32_MAX) ? -1 : nextTime; // return -1 if no valid time is found
}
int countValidEntries()
{
    int count = 0;
    for (int i = 0; i < MAX_ARROWS; i++)
    {
        if (myTrack.arrows[i].time != -1)
        {
            count++;
        }
    }
    return count;
}

track defloration() {
    track thisTrack = {0};
    int arrowCounter = 0;
    thisTrack.bpm = 125;
    thisTrack.introLength = 4929;
    thisTrack.songPath = "rom:/64beats/defloration.xm64";
    // yes, i lied. it's 68 beats.
    for (int i = 0; i < 68; i++)
    {
        float random = (float)rand() / (RAND_MAX / 4.0);
        thisTrack.arrows[arrowCounter].time         = i * 60000/thisTrack.bpm;
        thisTrack.arrows[arrowCounter].direction    = random;
        thisTrack.arrows[arrowCounter++].difficulty = 1;
    }
    // fill up the rest of the array.
    thisTrack.trackLength = thisTrack.arrows[arrowCounter-1].time;
    thisTrack.arrowNum = arrowCounter;
    for (int i = arrowCounter; i<MAX_ARROWS;i++) {
        thisTrack.arrows[arrowCounter].time         = 9000000;
        thisTrack.arrows[arrowCounter].direction    = 1;
        thisTrack.arrows[arrowCounter++].difficulty = 1;
    }

    return thisTrack;
}
void loadSong()
{
    // Fill the array with data
/*
    for (int i = 0; i < MAX_ARROWS; i++)
    {
        float random = (float)rand() / (RAND_MAX / 4.0);
        myTrack.arrows[i].time = i * (60000 / songBPM);
        myTrack.arrows[i].direction = (uint8_t)random;
        myTrack.arrows[i].difficulty = 1;
    }*/
    myTrack = defloration();
}
int calculateXForArrow(uint8_t playerNum, uint8_t dir)
{
    #define SCREEN_WIDTH 320
    #define SCREEN_MIDDLE SCREEN_WIDTH / 2
    #define WIDTH_PER_PLAYER SCREEN_WIDTH / 4
    const uint8_t arrowWidthScaled = arrow_sprite->width * UI_SCALE;
    const uint16_t playerSlotStart = WIDTH_PER_PLAYER * playerNum;
    const uint16_t playerSlotMiddle = playerSlotStart + (WIDTH_PER_PLAYER / 2);
    const uint16_t arrowPosInSlot = playerSlotMiddle + (-1.5 + dir) * arrowWidthScaled;
    return (int)arrowPosInSlot;
}
int calculateYForArrow(int time)
{
    return 0;
}
void drawArrowForPlayer(uint8_t playerNum, int yPos, uint8_t dir)
{

    rdpq_sprite_blit(arrow_sprites[dir],
                     (int32_t)(calculateXForArrow(playerNum, dir)),
                     (int32_t)(yPos),
                     &(rdpq_blitparms_t){
                         .cx = (arrow_sprite->width / 2),
                         .cy = (arrow_sprite->height / 2),
                         .scale_x = ui.scale_factor_x,
                         .scale_y = ui.scale_factor_y,
                     });
    // rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, (int32_t)(calculateXForArrow(playerNum, dir)), (int32_t)(yPos), "TIME: %d", currentTargetArrow);
}
int calculateDeltaTime(int arrowIndex)
{
    return songTime - myTrack.arrows[arrowIndex].time;
}
void drawArrows()
{
    static int arrowsStart = 0;

    joypad_inputs_t joypad = joypad_get_inputs(0);
    float xModifier = (joypad.stick_x / 90.0 + 2) / 2;

    int arrowsEnd = (arrowsStart + 50 > MAX_ARROWS) ? MAX_ARROWS : arrowsStart + 50;
    currentTargetArrow = arrowsStart;
    for (int i = arrowsStart; i < arrowsEnd; i++)
    {
        int timeDelta = calculateDeltaTime(i);

        int yPos = (((-timeDelta)) / 20 * xModifier);

        yPos += SCREEN_MARGIN_TOP;
        if (yPos > 240 + arrow_sprite->height)
        {
            continue;
        }
        if (yPos < 0 - arrow_sprite->height)
        {
            arrowsStart = i;
            continue;
        }
        for (uint8_t thisPlayer = 0; thisPlayer < 4; thisPlayer++)
        {
            // TODO: check myTrack.arrows[i].difficulty
            if (myTrack.arrows[i].hit[thisPlayer])
            {
                continue;
            }
            drawArrowForPlayer(thisPlayer, yPos, myTrack.arrows[i].direction);
        }
    }
}
int getMulti(uint8_t player) {
    int multiFactor = 1 + (multi[player] / 8 <= 4 ? multi[player] / 8 : 4);
    
    return multiFactor;
}
void drawUI()
{
    for (uint8_t thisPlayer = 0; thisPlayer < 4; thisPlayer++)
    {
        for (uint8_t thisDirection = 0; thisDirection < 4; thisDirection++)
        {
            drawUIForPlayer(thisPlayer, thisDirection);
        }
        rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, (int32_t)(calculateXForArrow(thisPlayer, 1)), 220, "%dx / %d", getMulti(thisPlayer), points[thisPlayer]);

    
    }

}

void drawUIForPlayer(uint8_t playerNum, uint8_t dir)
{
    const uint8_t rotationLookup[4] = {1, 0, 2, 3};

    rdpq_set_mode_standard();
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_sprite_blit(arrow_sprite,
                     (int32_t)(calculateXForArrow(playerNum, dir)),
                     (int32_t)(SCREEN_MARGIN_TOP),
                     &(rdpq_blitparms_t){
                         .cx = (arrow_sprite->width / 2),
                         .cy = (arrow_sprite->height / 2),
                         .theta = degreesToRadians(90 * rotationLookup[dir]),
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
    xm64player_stop(&music);
    xm64player_close(&music);

    sprite_free(arrow_up_sprite);
    sprite_free(arrow_down_sprite);
    sprite_free(arrow_left_sprite);
    sprite_free(arrow_right_sprite);
    sprite_free(arrow_sprite);

    display_close();
    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_font_free(font);
    
}