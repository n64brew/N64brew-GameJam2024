#ifndef __MAP_H
#define __MAP_H

#include <libdragon.h>

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>

#include <memory>
#include <vector>

#include "./constants.hpp"
#include "./wrappers.hpp"

#include "../../../core.h"

constexpr int MapWidth = 1024;
constexpr int MapHeight = 1024;

class MapRenderer
{
    private:
        RDPQSurface surface;
        U::T3DMat4FP matFP;
        U::RSPQBlock block;
        U::Sprite sprite;

        U::TLUT tlut;

    public:
        MapRenderer();
        void render();
        void splash(int x, int y, PlyNum player);
};

#endif // __MAP_H

