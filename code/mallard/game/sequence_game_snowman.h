#ifndef SEQUENCE_GAME_SNOWMAN_H
#define SEQUENCE_GAME_SNOWMAN_H

#define SNOWMAN_TIME_BETWEEN_DAMAGE 0.5f

#define SNOWMAN_SPRITE_WIDTH 15
#define SNOWMAN_SPRITE_HEIGHT 17

#define SNOWMAN_COLLISION_BOX_X1_OFFSET 0
#define SNOWMAN_COLLISION_BOX_Y1_OFFSET 10
#define SNOWMAN_COLLISION_BOX_X2_OFFSET 12
#define SNOWMAN_COLLISION_BOX_Y2_OFFSET 16

#define SNOWMAN_HIT_BOX_X1_OFFSET 2
#define SNOWMAN_HIT_BOX_Y1_OFFSET 3
#define SNOWMAN_HIT_BOX_X2_OFFSET 10
#define SNOWMAN_HIT_BOX_Y2_OFFSET 12

extern sprite_t *sequence_game_snowman_idle_sprite;
extern sprite_t *sequence_game_snowman_damage_sprite;
extern sprite_t *sequence_game_snowman_jump_sprite;

extern Snowman *snowmen;
extern Duck *ducks;

void add_snowman();
void free_snowmen();
void display_snowmen();

#endif // SEQUENCE_GAME_SNOWMAN_H