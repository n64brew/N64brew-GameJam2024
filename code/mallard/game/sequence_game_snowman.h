#ifndef SEQUENCE_GAME_SNOWMAN_H
#define SEQUENCE_GAME_SNOWMAN_H

extern sprite_t *sequence_game_snowman_idle_sprite;
extern sprite_t *sequence_game_snowman_jump_sprite;

extern Snowman *snowmen;
extern Duck *ducks;

void add_snowman();
void free_snowmen();
void display_snowmen();

#endif // SEQUENCE_GAME_SNOWMAN_H