#ifndef __RAMPAGE_ASSETS_H__
#define __RAMPAGE_ASSETS_H__

#include <t3d/t3dmodel.h>

#define COUNTDOWN_NUMBER_COUNT  6

#define BUILDING_HEIGHT_STEPS   2

struct RampageSplitMesh {
    rspq_block_t* mesh;
    rspq_block_t* material;
};

struct RampageAssets {
    T3DModel* player;
    T3DModel* building[BUILDING_HEIGHT_STEPS];
    struct RampageSplitMesh buildingSplit[BUILDING_HEIGHT_STEPS];
    T3DModel* ground;
    T3DModel* tank;
    struct RampageSplitMesh tankSplit;
    T3DModel* bullet;
    T3DModel* swing_effect;
    struct RampageSplitMesh swing_split;
    T3DModel* spark_effect;
    struct RampageSplitMesh spark_split;

    sprite_t* countdown_numbers[COUNTDOWN_NUMBER_COUNT];
    sprite_t* destroy_image;
    sprite_t* finish_image;

    wav64_t music;
    wav64_t collapseSound;
    wav64_t countdownSound;
    wav64_t hitSound;
    wav64_t roarSounds[2];
    wav64_t startJingle;
};

void rampage_assets_init();
void rampage_assets_destroy();

struct RampageAssets* rampage_assets_get();

void rampage_model_separate_material(T3DModel* model, struct RampageSplitMesh* result);
void rampage_model_free_split(struct RampageSplitMesh* result);

#endif