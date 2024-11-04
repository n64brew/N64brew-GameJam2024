#ifndef __RAMPAGE_ASSETS_H__
#define __RAMPAGE_ASSETS_H__

#include <t3d/t3dmodel.h>

struct RampageAssets {
    T3DModel* player;
    T3DModel* building;
};

void rampage_assets_init();
void rampage_assets_destroy();

struct RampageAssets* rampage_assets_get();

#endif