#include "./building.h"

#include <libdragon.h>
#include <n64sys.h>
#include "./assets.h"
#include "./util/entity_id.h"
#include "./collision/box.h"
#include "./collision/collision_scene.h"

#include "./rampage.h"

#define BUILDING_COLLIDE_GROUP 1

struct dynamic_object_type building_collider = {
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
};

void rampage_building_init(struct RampageBuilding* building, T3DVec3* position) {
    dynamic_object_init(
        entity_id_next(),
        &building->dynamic_object,
        &building_collider,
        COLLISION_LAYER_TANGIBLE,
        (struct Vector3*)position,
        &gRight2
    );

    building->dynamic_object.collision_group = BUILDING_COLLIDE_GROUP;
    building->dynamic_object.center.y = building_collider.data.box.half_size.y;
    building->dynamic_object.is_fixed = true;

    collision_scene_add(&building->dynamic_object);
}

void rampage_building_destroy(struct RampageBuilding* building) {
    collision_scene_remove(&building->dynamic_object);
}

void rampage_building_render(struct RampageBuilding* building) {
    T3DQuat rotation;
    t3d_quat_identity(&rotation);
    T3DVec3 scale = {{1.0f, 1.0f, 1.0f}};

    t3d_mat4fp_from_srt(UncachedAddr(&building->mtx), scale.v, rotation.v, (float*)&building->dynamic_object.position);
    t3d_matrix_push(&building->mtx);
    t3d_model_draw(rampage_assets_get()->building);
    t3d_matrix_pop(1);
}

void rampage_building_update(struct RampageBuilding* building, float delta_time) {

}