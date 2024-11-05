#ifndef __RAMPAGE_ASSETS_H__
#define __RAMPAGE_ASSETS_H__

#include <t3d/t3dmodel.h>

struct MallardAssets {
    T3DModel* player;
    T3DModel* building;
};

void mallard_assets_init();
void mallard_assets_destroy();

struct MallardAssets* mallard_assets_get();

#endif