#include "./assets.h"

static struct RampageAssets gRampageAssets;

void rampage_assets_init() {
    gRampageAssets.building = t3d_model_load("rom:/rampage/building.t3dm");
    gRampageAssets.player = t3d_model_load("rom:/rampage/player.t3dm");
    gRampageAssets.ground = t3d_model_load("rom://rampage/ground.t3dm");
}

void rampage_assets_destroy() {
    t3d_model_free(gRampageAssets.building);
    t3d_model_free(gRampageAssets.player);
    t3d_model_free(gRampageAssets.ground);
}

struct RampageAssets* rampage_assets_get() {
    return &gRampageAssets;
}