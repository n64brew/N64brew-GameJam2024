#ifndef SEQUENCE_GAME_H
#define SEQUENCE_GAME_H

#define GAME_FADE_IN_DURATION 1.0f
#define GAME_EXIT_DURATION 2.0f
#define GAME_EXIT_THRESHOLD_DURATION 0.1f

extern bool sequence_game_finished;

struct Character
{
    unsigned int x;
    unsigned int y;
};

struct Controller
{
    unsigned int start_down;
    unsigned int start_up;
    float start_held_elapsed;
};

void sequence_game(float deltatime);

#endif // SEQUENCE_GAME_H
