#include "./assets.h"

static struct MallardAssets gMallardAssets;

void mallard_assets_init() {
    gMallardAssets.building = t3d_model_load("rom:/mallard/building.t3dm");
    gMallardAssets.player = t3d_model_load("rom:/mallard/player.t3dm");
}

void mallard_assets_destroy() {
    t3d_model_free(gMallardAssets.building);
    t3d_model_free(gMallardAssets.player);
}

struct MallardAssets* mallard_assets_get() {
    return &gMallardAssets;
}