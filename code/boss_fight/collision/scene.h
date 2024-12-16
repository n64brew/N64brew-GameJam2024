/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include "mesh.h"
#include "shapes.h"
#include <set>
#include <vector>

namespace Coll
 {
  class Scene {
    private:
      constexpr static uint32_t VOID_SPHERE_COUNT = 2;

      std::set<MeshInstance*> meshes{};
      std::vector<Sphere*> spheres{};
      Sphere voidSpheres[VOID_SPHERE_COUNT]{};

    public:
      uint64_t ticks{0};
      uint64_t ticksBVH{0};
      uint64_t raycastCount{0};

      void registerMesh(MeshInstance *mesh) {
        meshes.insert(mesh);
      }

      void unregisterMesh(MeshInstance *mesh) {
        meshes.erase(mesh);
      }

      void registerSphere(Sphere *sphere) {
        spheres.push_back(sphere);
      }

      void unregisterSphere(Sphere *sphere) {
        for(auto it = spheres.begin(); it != spheres.end(); ++it) {
          if(*it == sphere)return (void)spheres.erase(it);
        }
      }

      void setVoidSphere(uint32_t idx, const T3DVec3 &pos, float radius) {
        idx %= VOID_SPHERE_COUNT;
        voidSpheres[idx].center = pos;
        voidSpheres[idx].radius = radius;
      }

      CollInfo vsSphere(Sphere &sphere, const T3DVec3 &velocity, float deltaTime);

      CollInfo raycastFloor(const T3DVec3 &pos);

      [[nodiscard]] const std::vector<Sphere*> &getSpheres() const {
        return spheres;
      }

      bool isInVoid(const T3DVec3 &pos) const;

      void update(float deltaTime);

      void debugDraw(bool showMesh, bool showSpheres);
  };
 }