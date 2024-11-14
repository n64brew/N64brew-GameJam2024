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

constexpr int MapWidth = 512;
constexpr int TileSize = 32;
constexpr int SegmentSize = 75;

struct Splash {
    float x;
    float y;
    PlyNum team;
};

class MapRenderer
{
    private:
        RDPQSurface surface;
        U::T3DMat4FP matFP;
        U::RSPQBlock block;
        // U::RSPQBlock paintBlock;
        U::Sprite sprite;

        U::TLUT tlut;

        int newSplashCount;
        // TODO: is it possible to have more?
        std::array<Splash, MAXPLAYERS * 2> newSplashes;

        T3DVertPacked* vertices;

        void __splash(int x, int y, PlyNum player);
    public:
        MapRenderer();
        ~MapRenderer();
        void render();
        void splash(float x, float y, PlyNum team);
};

#endif // __MAP_H

