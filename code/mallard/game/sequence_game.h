#ifndef SEQUENCE_GAME_H
#define SEQUENCE_GAME_H

#define GAME_DURATION 60.0f
#define GAME_FADE_IN_DURATION 1.0f
#define GAME_COUNTDOWN_DURATION 3.0f
#define GAME_ENDING_DURATION 3.0f
#define GAME_EXIT_DURATION 2.0f
#define GAME_EXIT_THRESHOLD_DURATION 0.1f

extern bool sequence_game_finished;

typedef enum
{
    LEFT = 0,
    RIGHT = 1,
} Directions;

typedef enum DuckActions
{
    DUCK_IDLE = 1,
    DUCK_WALK = 2,
    DUCK_SLAP = 3,
    DUCK_RUN = 4,
    DUCK_DAMAGE = 5,
} DuckActions;

typedef struct Duck
{
    int id;
    float x;
    float y;
    float score;
    float time_since_last_hit;
    float time_seeking_target;
    DuckActions action;
    Directions direction;
    sprite_t *walk_sprite;
    sprite_t *slap_sprite;
    sprite_t *idle_sprite;
    sprite_t *run_sprite;
    sprite_t *damage_sprite;
    int frames;
    int frames_locked_for_slap;
    int frames_locked_for_damage;
    float collision_box_x1;
    float collision_box_y1;
    float collision_box_x2;
    float collision_box_y2;
    float slap_box_x1;
    float slap_box_y1;
    float slap_box_x2;
    float slap_box_y2;
    float hit_box_x1;
    float hit_box_y1;
    float hit_box_x2;
    float hit_box_y2;
    struct Duck *next;
} Duck;

typedef enum SnowmanActions
{
    SNOWMAN_IDLE = 0,
    SNOWMAN_JUMP = 1,
    SNOWMAN_DAMAGE = 2,
} SnowmanActions;

typedef struct Snowman
{
    int id;
    float x;
    float y;
    float time_since_last_hit;
    int health;
    SnowmanActions action;
    sprite_t *idle_sprite;
    sprite_t *damage_sprite;
    sprite_t *jump_sprite;
    int frames;
    int frames_locked_for_damage;
    float collision_box_x1;
    float collision_box_y1;
    float collision_box_x2;
    float collision_box_y2;
    float hit_box_x1;
    float hit_box_y1;
    float hit_box_x2;
    float hit_box_y2;
    struct Snowman *next;
} Snowman;

typedef struct Controller
{
    unsigned int start_down;
    unsigned int start_up;
    float start_held_elapsed;
} Controller;

void sequence_game(float deltatime);

#endif // SEQUENCE_GAME_H
