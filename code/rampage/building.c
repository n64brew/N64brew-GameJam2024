#include "./building.h"

#include <libdragon.h>
#include <n64sys.h>
#include "./assets.h"
#include "./util/entity_id.h"
#include "./collision/box.h"
#include "./collision/collision_scene.h"
#include "./math/mathf.h"
#include "./scene_query.h"
#include "./spark_effect.h"

#include "./rampage.h"

#define BUILDING_COLLIDE_GROUP 1
#define BUILDING_HEALTH 2

#define SHAKE_TIME          0.5f
#define SHAKE_AMPLITUDE     SCALE_FIXED_POINT(0.03f)

#define COLLAPSE_SPEED      SCALE_FIXED_POINT(0.5f)

#define HOUSE_SEGMENT_HEIGHT    SCALE_FIXED_POINT(1.0f)

static T3DMat4FP offsetMatrix[9];

static struct Vector3 billboards_locations[BILLBOARD_COUNT] = {
    {SCALE_FIXED_POINT(0.65f), SCALE_FIXED_POINT(1.2f), SCALE_FIXED_POINT(-0.36f)},
    {SCALE_FIXED_POINT(0.62f), SCALE_FIXED_POINT(1.5f), SCALE_FIXED_POINT(0.22f)},
    {SCALE_FIXED_POINT(-0.68f), SCALE_FIXED_POINT(0.89f), SCALE_FIXED_POINT(0.39f)},
    {SCALE_FIXED_POINT(-0.1f), SCALE_FIXED_POINT(1.1f), SCALE_FIXED_POINT(-0.6f)},
    {SCALE_FIXED_POINT(0.14f), SCALE_FIXED_POINT(0.92f), SCALE_FIXED_POINT(0.58f)},
    {SCALE_FIXED_POINT(-0.53f), SCALE_FIXED_POINT(1.3f), SCALE_FIXED_POINT(-0.2f)},
};

struct dynamic_object_type building_colliders[] = {
    {
        .minkowsi_sum = box_minkowski_sum,
        .bounding_box = box_bounding_box,
        .data = {
            .box = {
                .half_size = {
                    SCALE_FIXED_POINT(0.5f),
                    SCALE_FIXED_POINT(0.5f), 
                    SCALE_FIXED_POINT(0.5f)
                },
            },
        },
        .bounce = 0.0f,
        .friction = 0.5f,
    },
    {
        .minkowsi_sum = box_minkowski_sum,
        .bounding_box = box_bounding_box,
        .data = {
            .box = {
                .half_size = {
                    SCALE_FIXED_POINT(0.5f),
                    SCALE_FIXED_POINT(1.0f), 
                    SCALE_FIXED_POINT(0.5f)
                },
            },
        },
        .bounce = 0.0f,
        .friction = 0.5f,
    },
    {
        .minkowsi_sum = box_minkowski_sum,
        .bounding_box = box_bounding_box,
        .data = {
            .box = {
                .half_size = {
                    SCALE_FIXED_POINT(0.5f),
                    SCALE_FIXED_POINT(1.5f), 
                    SCALE_FIXED_POINT(0.5f)
                },
            },
        },
        .bounce = 0.0f,
        .friction = 0.5f,
    },
    {
        .minkowsi_sum = box_minkowski_sum,
        .bounding_box = box_bounding_box,
        .data = {
            .box = {
                .half_size = {
                    SCALE_FIXED_POINT(0.5f),
                    SCALE_FIXED_POINT(2.0f), 
                    SCALE_FIXED_POINT(0.5f)
                },
            },
        },
        .bounce = 0.0f,
        .friction = 0.5f,
    },
};

void ramapge_building_spawn_spark(struct RampageBuilding* building) {
    if (!building->billboards) {
        return;
    }
    
    for (int i = 0; i < BILLBOARD_COUNT; i += 1) {
        if ((1 << i) & building->billboards) {
            struct Vector3 position;
            struct Vector3* local_pos = &billboards_locations[i];
            position.x = building->dynamic_object.position.x + 
                building->dynamic_object.rotation.x * local_pos->x - building->dynamic_object.rotation.y * local_pos->z;
            position.y = local_pos->y;
            position.z = building->dynamic_object.position.z + 
                building->dynamic_object.rotation.x * local_pos->z + building->dynamic_object.rotation.y * local_pos->x;

            spark_effects_spawn(&position);
            break;
        }
    }
}

void rampage_building_damage(void* data, int amount, struct Vector3* velocity, int source_id) {
    struct RampageBuilding* building = (struct RampageBuilding*)data;

    if (building->hp == 0) {
        return;
    }

    building->hp -= amount;
    building->shake_timer = SHAKE_TIME;

    if (building->hp <= 0) {
        wav64_play(&rampage_assets_get()->collapseSound, 3);
        give_player_score(source_id, building->height);
        building->health.is_dead = 1;
        building->is_collapsing = true;
        ramapge_building_spawn_spark(building);
    } else {
        wav64_play(&rampage_assets_get()->hitSound, 3);
    }
}

uint8_t building_health[] = {
    0,
    1,
    3,
    5,
};

void rampage_building_init(struct RampageBuilding* building, T3DVec3* position, int rotation, int building_height) {
    int entity_id = entity_id_next();

    building->height = building_height;

    dynamic_object_init(
        entity_id,
        &building->dynamic_object,
        &building_colliders[building->height - 1],
        COLLISION_LAYER_TANGIBLE,
        (struct Vector3*)position,
        &gRight2
    );

    building->dynamic_object.collision_group = BUILDING_COLLIDE_GROUP;
    building->dynamic_object.center.y = building_colliders[building->height].data.box.half_size.y;
    building->dynamic_object.is_fixed = true;
    building->is_destroyed = false;
    building->rotation = rotation;

    building->hp = building_health[building->height];
    building->shake_timer = 0.0f;
    building->is_collapsing = false;
    building->billboards = 0;

    collision_scene_add(&building->dynamic_object);

    health_register(entity_id, &building->health, rampage_building_damage, building);

    for (int i = 0; i < 9; i += 1) {
        int x = i % 3;
        int y = i / 3;

        T3DVec3 pos;
        pos.v[0] = (x - 1) * SHAKE_AMPLITUDE;
        pos.v[1] = HOUSE_SEGMENT_HEIGHT;
        pos.v[2] = (y - 1) * SHAKE_AMPLITUDE;
        t3d_mat4fp_identity(UncachedAddr(&offsetMatrix[i]));
        t3d_mat4fp_set_pos(UncachedAddr(&offsetMatrix[i]), pos.v);
    }

    building->redraw_handle = redraw_aquire_handle();
}

static bool allowed_billboards_by_height[3][BILLBOARD_COUNT][4] = {
    {
        {false, false, false, false},
        {false, false, false, false},
        {false, false, false, false},
        {false, false, false, false},
        {false, false, false, false},
        {false, false, false, false},
    },
    {
        {true, false, true, true},
        {true, true, false, true},
        {true, true, true, false},
        {false, true, true, true},
        {true, true, false, true},
        {false, true, true, true},
    },
    {
        {true, false, true, true},
        {true, true, false, true},
        {true, true, true, false},
        {false, false, true, true},
        {true, true, false, false},
        {false, true, true, true},
    },
};

bool rampage_building_add_billboard(struct RampageBuilding* building, int billboard_index) {
    int billboard_mask = 1 << billboard_index;
    if ((building->billboards & billboard_mask) || 
        building->height <= 1 || 
        !allowed_billboards_by_height[building->height-1][billboard_index][building->rotation]) {
        // already has billboard
        return false;
    }

    building->billboards |= billboard_mask;
    return true;
}

void rampage_building_destroy(struct RampageBuilding* building) {
    if (building->is_destroyed) {
        return;
    }
    collision_scene_remove(&building->dynamic_object);
    health_unregister(building->dynamic_object.entity_id);
    building->is_destroyed = true;
}

static T3DQuat rotations[] = {
    {{0, 0, 0, 1}},
    {{0, 0.707f, 0, 0.707f}},
    {{0, 1, 0, 0}},
    {{0, 0.707f, 0, -0.707f}},
};

bool rampage_building_is_light_off(struct RampageBuilding* building) {
    return randomInRangef(building->dynamic_object.position.y, 1.0f) < 0.0f || (building->shake_timer > 0.0f && randomInRangef(0.0f, 1.0f) > 0.5f);
}

void rampage_building_render(struct RampageBuilding* building, int height_pass) {
    if (building->is_destroyed) {
        return;
    }

    if (building->height - 1 != height_pass) {
        return;
    }

    T3DVec3 scale = {{1.0f, 1.0f, 1.0f}};

    struct Vector3 final_pos = building->dynamic_object.position;
    
    if (building->is_collapsing) {
        final_pos.x += randomInRangef(-SHAKE_AMPLITUDE, SHAKE_AMPLITUDE);
        final_pos.z += randomInRangef(-SHAKE_AMPLITUDE, SHAKE_AMPLITUDE);
    } else if (building->shake_timer > 0.0f) {
        float amplitude = (SHAKE_AMPLITUDE * (1.0f / SHAKE_TIME)) * building->shake_timer;

        final_pos.x += randomInRangef(-amplitude, amplitude);
        final_pos.z += randomInRangef(-amplitude, amplitude);
    }

    t3d_mat4fp_from_srt(UncachedAddr(&building->mtx), scale.v, rotations[building->rotation].v, (float*)&final_pos);
    t3d_matrix_push(&building->mtx);
    if (rampage_building_is_light_off(building)) {
        rdpq_set_prim_color((color_t){0x00, 0x00, 0x00, 0xFF});
    } else {
        rdpq_set_prim_color((color_t){0xEE, 0xE3, 0xC4, 0xFF});
    }
    rspq_block_run(rampage_assets_get()->buildingSplit[height_pass].mesh);
    t3d_matrix_pop(1);
}

void rampage_building_render_billboards(struct RampageBuilding* building) {
    if (building->is_destroyed || !building->billboards) {
        return;
    }

    if (rampage_building_is_light_off(building)) {
        rdpq_set_prim_color((color_t){0x40, 0x40, 0x40, 0xFF});
    } else {
        rdpq_set_prim_color((color_t){0xFF, 0xFF, 0xFF, 0xFF});
    }

    t3d_matrix_push(&building->mtx);
    for (int i = 0; i < BILLBOARD_COUNT; i += 1) {
        if ((1 << i) & building->billboards) {
            rspq_block_run(rampage_assets_get()->billboardsSplit[i].mesh);
        }
    }
    t3d_matrix_pop(1);
}

void rampage_building_update(struct RampageBuilding* building, float delta_time) {
    if (building->is_destroyed) {
        return;
    }

    if (building->shake_timer > 0.0f) {
        building->shake_timer -= delta_time;
    }

    if (building->is_collapsing) {
        building->dynamic_object.position.y -= COLLAPSE_SPEED * delta_time;

        if (building->dynamic_object.bounding_box.max.y < 0.0f) {
            rampage_building_destroy(building);
        }
    }
}

#define BUILDING_RADIUS SCALE_FIXED_POINT(0.6f)

void rampage_building_redraw_rect(T3DViewport* viewport, struct RampageBuilding* building) {
    if (building->is_destroyed) {
        return;
    }

    redraw_get_screen_rect(viewport, &building->dynamic_object.position, BUILDING_RADIUS, 0.0f, SCALE_FIXED_POINT(building->height + 0.1f), &building->redraw_rect);
    
    if (!building->shake_timer && !building->is_collapsing) {
        return;
    }

    redraw_update_dirty(building->redraw_handle, &building->redraw_rect);
}