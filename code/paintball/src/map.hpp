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

class MapRenderer
{
    private:
        RDPQSurface surface;
        U::T3DMat4FP matFP;
        U::RSPQBlock block;

    public:
        MapRenderer();
        void render();
};

#endif // __MAP_H

