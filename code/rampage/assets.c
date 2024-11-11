#include "./assets.h"

static struct RampageAssets gRampageAssets;

bool rampage_model_filter(void* userData, const T3DObject *obj) {
    struct RampageSplitMesh* result = (struct RampageSplitMesh*)userData;

    T3DModelState state = t3d_model_state_create();
    rspq_block_begin();
        t3d_model_draw_material(obj->material, &state);
    result->material = rspq_block_end();
    
    rspq_block_begin();
        t3d_model_draw_object(obj, NULL);
    result->mesh = rspq_block_end();

    return false;
}

void rampage_model_separate_material(T3DModel* model, struct RampageSplitMesh* result) {
    t3d_model_draw_custom(
        model,
        (T3DModelDrawConf){
            .filterCb = rampage_model_filter,
            .userData = result,
        }
    );
}

void rampage_model_free_split(struct RampageSplitMesh* result) {
    rspq_block_free(result->material);
    rspq_block_free(result->mesh);
}

void rampage_assets_init() {
    gRampageAssets.building = t3d_model_load("rom:/rampage/house.t3dm");
    rampage_model_separate_material(gRampageAssets.building, &gRampageAssets.buildingSplit);

    gRampageAssets.player = t3d_model_load("rom:/rampage/Jira_01.t3dm");
    gRampageAssets.ground = t3d_model_load("rom://rampage/level.t3dm");
    gRampageAssets.tank = t3d_model_load("rom://rampage/tank0.t3dm");
    rampage_model_separate_material(gRampageAssets.tank, &gRampageAssets.tankSplit);
}

void rampage_assets_destroy() {
    rampage_model_free_split(&gRampageAssets.buildingSplit);
    rampage_model_free_split(&gRampageAssets.tankSplit);

    t3d_model_free(gRampageAssets.building);
    t3d_model_free(gRampageAssets.player);
    t3d_model_free(gRampageAssets.ground);
    t3d_model_free(gRampageAssets.tank);
}

struct RampageAssets* rampage_assets_get() {
    return &gRampageAssets;
}