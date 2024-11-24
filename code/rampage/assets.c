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

static const char* building_models[] = {
    "rom:/rampage/house.t3dm",
    "rom:/rampage/building_2story.t3dm",
};

void rampage_assets_init() {
    for (int i = 0; i < BUILDING_HEIGHT_STEPS; i += 1) {
        gRampageAssets.building[i] = t3d_model_load(building_models[i]);
        rampage_model_separate_material(gRampageAssets.building[i], &gRampageAssets.buildingSplit[i]);
    }

    gRampageAssets.player = t3d_model_load("rom:/rampage/Jira_01.t3dm");

    gRampageAssets.ground = t3d_model_load("rom://rampage/ground.t3dm");
    rspq_block_begin();
    t3d_model_draw(gRampageAssets.ground);
    gRampageAssets.ground->userBlock = rspq_block_end();

    gRampageAssets.tank = t3d_model_load("rom://rampage/tank0.t3dm");
    rampage_model_separate_material(gRampageAssets.tank, &gRampageAssets.tankSplit);

    gRampageAssets.bullet = t3d_model_load("rom://rampage/bullet.t3dm");
    rspq_block_begin();
    t3d_model_draw(gRampageAssets.bullet);
    gRampageAssets.bullet->userBlock = rspq_block_end();

    gRampageAssets.swing_effect = t3d_model_load("rom://rampage/swing_effect.t3dm");
    rampage_model_separate_material(gRampageAssets.swing_effect, &gRampageAssets.swing_split);

    for (int i = 0; i < COUNTDOWN_NUMBER_COUNT; i += 1) {
        char filename[32];
        sprintf(filename, "rom:/rampage/%d.sprite", i);
        gRampageAssets.countdown_numbers[i] = sprite_load(filename);
    }

    gRampageAssets.destroy_image = sprite_load("rom:/rampage/destroy.sprite");
    gRampageAssets.finish_image = sprite_load("rom:/rampage/finish.sprite");

    wav64_open(&gRampageAssets.music, "rom:/rampage/stompin.wav64");

    wav64_open(&gRampageAssets.collapseSound, "rom:/rampage/Collapse.wav64");
    wav64_open(&gRampageAssets.countdownSound, "rom:/rampage/countdown.wav64");
    wav64_open(&gRampageAssets.hitSound, "rom:/rampage/hit.wav64");
    wav64_open(&gRampageAssets.roarSounds[0], "rom:/rampage/Roar1.wav64");
    wav64_open(&gRampageAssets.roarSounds[1], "rom:/rampage/RoarShort.wav64");
    wav64_open(&gRampageAssets.startJingle, "rom:/rampage/startjingle.wav64");
}

void rampage_assets_destroy() {
    for (int i = 0; i < BUILDING_HEIGHT_STEPS; i += 1) {
        rampage_model_free_split(&gRampageAssets.buildingSplit[i]);
        t3d_model_free(gRampageAssets.building[i]);
    }

    rampage_model_free_split(&gRampageAssets.tankSplit);
    t3d_model_free(gRampageAssets.player);
    t3d_model_free(gRampageAssets.ground);
    t3d_model_free(gRampageAssets.tank);
    t3d_model_free(gRampageAssets.bullet);
    t3d_model_free(gRampageAssets.swing_effect);
    rampage_model_free_split(&gRampageAssets.swing_split);

    for (int i = 0; i < COUNTDOWN_NUMBER_COUNT; i += 1) {
        sprite_free(gRampageAssets.countdown_numbers[i]);
    }

    wav64_close(&gRampageAssets.music);

    wav64_close(&gRampageAssets.collapseSound);
    wav64_close(&gRampageAssets.countdownSound);
    wav64_close(&gRampageAssets.hitSound);
    wav64_close(&gRampageAssets.roarSounds[0]);
    wav64_close(&gRampageAssets.roarSounds[1]);
    wav64_close(&gRampageAssets.startJingle);
}

struct RampageAssets* rampage_assets_get() {
    return &gRampageAssets;
}