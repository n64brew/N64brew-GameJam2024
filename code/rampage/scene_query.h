#ifndef __RAMPAGE_SCENE_QUERY_H__
#define __RAMPAGE_SCENE_QUERY_H__

#include "./math/vector3.h"
#include <stdbool.h>

struct Vector3* find_nearest_target(struct Vector3* from, float error_tolerance);

bool is_tank_target_used(struct Vector3* target);

void give_player_score(int enity_id, int amount);
bool is_player(int entity_id);

#endif