#include "./building.h"

#include <libdragon.h>
#include <n64sys.h>
#include "./assets.h"

void rampage_building_init(struct RampageBuilding* building, T3DVec3* position) {
    building->position = *position;
}

void rampage_building_destroy(struct RampageBuilding* building) {

}

void rampage_building_render(struct RampageBuilding* building) {
    T3DQuat rotation;
    t3d_quat_identity(&rotation);
    T3DVec3 scale = {{1.0f, 1.0f, 1.0f}};

    t3d_mat4fp_from_srt(UncachedAddr(&building->mtx), scale.v, rotation.v, building->position.v);
    t3d_matrix_push(&building->mtx);
    t3d_model_draw(rampage_assets_get()->building);
    t3d_matrix_pop(1);
}

void rampage_building_update(struct RampageBuilding* building, float delta_time) {

}