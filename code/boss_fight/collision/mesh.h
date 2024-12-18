/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once
#include <vector>
#include <string>
#include "shapes.h"

namespace Coll
{
  struct BVH;

  struct Mesh
  {
    // NOTE: don't place any extra members here!
    // mirrors the .coll file format
    uint32_t triCount{};
    uint32_t vertCount{};
    float collScale{};
    T3DVec3 *verts{};
    IVec3 *normals{};
    BVH* bvh{};
    // data follows here: indices, normals, verts, BVH
    int16_t indices[];

    [[nodiscard]] Coll::CollInfo vsSphere(const Coll::Sphere &sphere, const Triangle& triangle) const;
    [[nodiscard]] Coll::CollInfo vsFloorRay(const T3DVec3 &pos, const Triangle& triangle) const;

    static Mesh* load(const std::string &path);
  };

  struct MeshInstance {
    Mesh *mesh{};
    T3DVec3 pos{0.0f, 0.0f, 0.0f};
    T3DVec3 scale{1.0f, 1.0f, 1.0f};
    T3DQuat rot{0.0f, 0.0f, 0.0f, 1.0f};
  };
}