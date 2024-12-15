#include "./scene_query.h"

#include "rampage.h"
#include "./math/mathf.h"

extern struct Rampage gRampage;

struct Vector3* find_nearest_target(struct Vector3* from, float error_tolerance) {
    struct Vector3* result = NULL;
    float score = 0.0f;
    float inv_error_tolerance = 1.0f / error_tolerance;

    for (int y = 0; y < BUILDING_COUNT_Y; y += 1) {
        for (int x = 0; x < BUILDING_COUNT_X; x += 1) {
            struct RampageBuilding* building = &gRampage.buildings[y][x];

            if (building->is_collapsing) {
                continue;
            }

            float building_score = vector3DistSqrd(from, &building->dynamic_object.position);
            float error = randomInRangef(inv_error_tolerance, error_tolerance);

            building_score *= error * error;

            if (result == NULL || building_score < score) {
                result = &building->dynamic_object.position;
                score = building_score;
            }
        }
    }

    return result;
}

bool is_tank_target_used(struct Vector3* target) {
    for (int i = 0; i < TANK_COUNT; i += 1) {
        if (vector3DistSqrd(target, &gRampage.tanks[i].current_target) < 1.0f) {
            return true;
        }
    }

    return false;
}

void give_player_score(int enity_id, int amount) {
    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        if (gRampage.players[i].dynamic_object.entity_id == enity_id) {
            gRampage.players[i].score += amount;
            gRampage.players[i].score_dirty = 2;
        }
    }
}

bool is_player(int entity_id) {
    for (int i = 0; i < PLAYER_COUNT; i += 1) {
        if (gRampage.players[i].dynamic_object.entity_id == entity_id) {
            return true;
        }
    }
    
    return false;
}