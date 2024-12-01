#ifndef SEQUENCE_GAME_INPUT_H
#define SEQUENCE_GAME_INPUT_H

#include <libdragon.h> // Can i remove this????

extern Snowman *snowmen;
extern Duck *ducks;

extern bool sequence_game_should_cleanup;
extern bool sequence_game_paused;
extern int sequence_game_player_holding_start;

extern float sequence_game_start_held_elapsed;

typedef struct Vector2
{
    float x;
    float y;
} Vector2;

typedef struct Rect
{
    float x1;
    float y1;
    float x2;
    float y2;
} Rect;

bool detect_collision(Rect a, Rect b);

void sequence_game_update(float deltatime);

#endif // SEQUENCE_GAME_INPUT_H
