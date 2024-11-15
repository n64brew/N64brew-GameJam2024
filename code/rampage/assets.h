#ifndef __RAMPAGE_ASSETS_H__
#define __RAMPAGE_ASSETS_H__

#include <t3d/t3dmodel.h>

#define COUNTDOWN_NUMBER_COUNT  6

struct RampageSplitMesh {
    rspq_block_t* mesh;
    rspq_block_t* material;
};

struct RampageAssets {
    T3DModel* player;
    T3DModel* building;
    struct RampageSplitMesh buildingSplit;
    T3DModel* ground;
    T3DModel* tank;
    struct RampageSplitMesh tankSplit;

    sprite_t* countdown_numbers[COUNTDOWN_NUMBER_COUNT];

    wav64_t music;
};

void rampage_assets_init();
void rampage_assets_destroy();

struct RampageAssets* rampage_assets_get();

#endif