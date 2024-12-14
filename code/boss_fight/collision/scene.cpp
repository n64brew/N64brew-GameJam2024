/**
 * @author Max Bebök
 * @license TBD
 */
#include "scene.h"
#include "bvh.h"
#include "../debug/debugDraw.h"

namespace {
  constexpr float MIN_PENETRATION = 0.00005f;
  constexpr float FLOOR_ANGLE = 0.7f;

  constexpr bool isFloor(const Coll::IVec3 &normal) {
    return normal.v[1] > (int16_t)(0x7FFF * FLOOR_ANGLE);
  }
  constexpr bool isFloor(const T3DVec3 &normal) {
    return normal.v[1] > FLOOR_ANGLE;
  }
}

Coll::CollInfo Coll::Scene::vsSphere(Coll::Sphere &sphere, const T3DVec3 &velocity, float deltaTime) {
  uint64_t ticksStart = get_ticks();
  float len2 = t3d_vec3_len2(velocity);

  int steps = (int)(len2 * 0.8f);
  if(steps <= 0)steps = 1;
  if(steps > 10)steps = 10;

  auto velocityStep = velocity * (deltaTime / steps);

  Coll::CollInfo res{
    .hitPos = T3DVec3{0.0f, 0.0f, 0.0f},
    .penetration = T3DVec3{0.0f, 0.0f, 0.0f},
    .normal = T3DVec3{0.0f, 0.0f, 0.0f},
    .collCount = 0,
  };
  Coll::BVHResult bvhRes{};

  for(int s=0; s<steps; ++s)
  {
    sphere.center = sphere.center + velocityStep;

    for(auto meshInst : meshes)
    {
      auto &mesh = *meshInst->mesh;

      auto sphereLocal = sphere;
      sphereLocal.center = sphereLocal.center - meshInst->pos;

      auto ticksBvhStart = get_ticks();
      bvhRes.reset();
      mesh.bvh->vsSphere(sphereLocal, bvhRes);
      ticksBVH += get_ticks() - ticksBvhStart;
      if(bvhRes.count >= Coll::MAX_RESULT_COUNT-1) {
        //debugf("BVH count: %d\n", bvhRes.count);
      }

      for(int b=0; b<bvhRes.count; ++b) {
        uint32_t t = bvhRes.triIndex[b];

        int idxA = mesh.indices[t*3];
        int idxB = mesh.indices[t*3+1];
        int idxC = mesh.indices[t*3+2];
        auto &norm = mesh.normals[t];

        Triangle tri{
          .normal = {{
           (float)norm.v[0] * (1.0f / 32767.0f),
           (float)norm.v[1] * (1.0f / 32767.0f),
           (float)norm.v[2] * (1.0f / 32767.0f)
          }},
          .v = {&mesh.verts[idxA], &mesh.verts[idxB], &mesh.verts[idxC]}
        };

        auto collInfo = mesh.vsSphere(sphereLocal, tri);
        if(collInfo.collCount)
        {
          float penLen2 = t3d_vec3_len2(&collInfo.penetration);
          if(penLen2 < MIN_PENETRATION)continue;

          ++res.collCount;
          res.penetration = res.penetration + collInfo.penetration;
          res.hitPos = collInfo.hitPos + meshInst->pos;
          res.normal = collInfo.normal;

          //DebugDraw::drawPoint(collInfo.hitPos, RGBA32(0xFF, 0x00, 0x00, 0xFF));
          sphere.center = sphere.center - collInfo.penetration;
        }
      } // BVH res
    } // meshes
  } // steps

  ticks += get_ticks() - ticksStart;
  return res;
}

void Coll::Scene::update(float deltaTime)
{
  for(auto sp : spheres) {
    sp->hitTriTypes = 0;
  }

  for(uint32_t s=0; s<spheres.size(); ++s) {
    auto &sphere = spheres[s];

    // Static/Triangle mesh collision
    bool checkColl = sphere->interactType & InteractType::TRI_MESH;

    // first check for void-sphere, those cut-out a section of geometry...
    if(checkColl) {
      for(uint32_t v=0; v<VOID_SPHERE_COUNT; ++v) {
        float radSum = voidSpheres[v].radius;
        if(radSum <= 0)continue;
        auto dist2 = t3d_vec3_distance2(voidSpheres[v].center, sphere->center);
        if(dist2 < radSum * radSum) {
          checkColl = false;
          sphere->center += sphere->velocity * deltaTime;
          break;
        }
      }
    }

    // ...if we are not inside one, check actual mesh data
    if(checkColl) {
      auto res = vsSphere(*sphere, sphere->velocity, deltaTime);
      if(res.collCount) {
        bool hitFloor = isFloor(res.normal);
        sphere->hitTriTypes |= hitFloor ? TriType::FLOOR : TriType::WALL;

        if(sphere->interactType & InteractType::BOUNCY) {
          sphere->velocity = sphere->velocity - res.normal * 2.0f * t3d_vec3_dot(sphere->velocity, res.normal);
          sphere->velocity *= 0.8f;
        } else if(hitFloor) {
          sphere->velocity.v[1] = 0.0f;
        }
      }
    }

    // Dynamic Colliders
    for(uint32_t s2=s+1; s2<spheres.size(); ++s2)
    {
      if(!(sphere->mask & spheres[s2]->mask))continue;
      // TODO: source & target mask instead of one mask
      if(sphere->type == CollType::COIN && spheres[s2]->type == CollType::COIN)continue;

      auto sphere2 = spheres[s2];
      T3DVec3 dir = sphere->center - sphere2->center;
      auto dist2 = t3d_vec3_len2(dir);
      float radSum = sphere->radius + sphere2->radius;
      radSum *= radSum;
      if(dist2 < radSum)
      {
        bool solidA = sphere->interactType & InteractType::SPHERES;
        bool solidB = sphere2->interactType & InteractType::SPHERES;
        if(solidA && solidB)
        {
          if(dist2 > 0.0001f) {
            dir /= sqrtf(dist2);
          } else {
            dir = T3DVec3{0.0f, 1.0f, 0.0f};
          }
          float pen = radSum - dist2;

          bool isFixedA = sphere->interactType & InteractType::FIXED_Y;
          bool isFixedB = sphere2->interactType & InteractType::FIXED_Y;

          if(isFixedA || isFixedB) {
            dir.v[1] = 0.0f;
          }

          // get interp factor based on mass (in this case mass=radius)
          float interp = sphere->radius / (sphere->radius + sphere2->radius);
          sphere->center = sphere->center + dir * (pen * (1.0f - interp));
          sphere2->center = sphere2->center - dir * (pen * interp);

          sphere->hitTriTypes |= TriType::SPHERE;
          sphere2->hitTriTypes |= TriType::SPHERE;
        }

        if(sphere->callback)sphere->callback(*sphere2);
        if(sphere2->callback)sphere2->callback(*sphere);
      }
    }
  }
}

Coll::CollInfo Coll::Scene::raycastFloor(const T3DVec3 &pos) {
  ++raycastCount;
  Coll::CollInfo res{
    .hitPos = T3DVec3{0.0f, 0.0f, 0.0f},
    .penetration = T3DVec3{0.0f, 0.0f, 0.0f},
    .normal = T3DVec3{0.0f, 0.0f, 0.0f},
    .collCount = 0,
  };

  for(auto meshInst : meshes)
  {
    auto &mesh = *meshInst->mesh;
    auto posLocal = pos - meshInst->pos;
    Coll::IVec3 posInt = {
      .v = {
        (int16_t)(posLocal.v[0] * 64.0f),
        (int16_t)(posLocal.v[1] * 64.0f),
        (int16_t)(posLocal.v[2] * 64.0f)
      }
    };

    Coll::BVHResult bvhRes{};
    mesh.bvh->raycastFloor(posInt, bvhRes);

    float highestFloor = -99999.0f;
    for(int b=0; b<bvhRes.count; ++b)
    {
    //for(uint32_t b=0; b<mesh.triCount; ++b) {
      uint32_t t = bvhRes.triIndex[b];
      //uint32_t t = b;
      if(!isFloor(mesh.normals[t]))continue;

      int idxA = mesh.indices[t*3];
      int idxB = mesh.indices[t*3+1];
      int idxC = mesh.indices[t*3+2];
      auto &norm = mesh.normals[t];

      Triangle tri{
        .normal = {{
         (float)norm.v[0] / 32767.0f,
         (float)norm.v[1] / 32767.0f,
         (float)norm.v[2] / 32767.0f
        }},
        .v = {&mesh.verts[idxA], &mesh.verts[idxB], &mesh.verts[idxC]}
      };

      auto collInfo = mesh.vsFloorRay(posLocal, tri);
      if(collInfo.collCount && collInfo.hitPos.v[1] > highestFloor)
      {
        res.collCount = 1;
        res.hitPos = collInfo.hitPos + meshInst->pos;
        res.normal = collInfo.normal;
        highestFloor = collInfo.hitPos.v[1];
      }
    }
  }

  if (res.collCount) {
    for(uint32_t v=0; v<VOID_SPHERE_COUNT; ++v) {
      float radSum = voidSpheres[v].radius;
      if(radSum <= 0)continue;
      auto dist2 = t3d_vec3_distance2(voidSpheres[v].center, res.hitPos);
      if(dist2 < radSum * radSum) {
        res.collCount = 0;
        return res;
      }
    }
  }

  return res;
}

void Coll::Scene::debugDraw(bool showMesh, bool showSpheres)
{
  if(showMesh) {
    for(const auto &meshInst : meshes) {
      auto &mesh = *meshInst->mesh;
      for(uint32_t t=0; t<mesh.triCount; ++t) {
        int idxA = mesh.indices[t*3];
        int idxB = mesh.indices[t*3+1];
        int idxC = mesh.indices[t*3+2];
        auto v0 = (mesh.verts[idxA] + meshInst->pos) * 16.0f;
        auto v1 = (mesh.verts[idxB] + meshInst->pos) * 16.0f;
        auto v2 = (mesh.verts[idxC] + meshInst->pos) * 16.0f;

        if(mesh.normals[t].v[2] < 0)continue;
        auto color = isFloor(mesh.normals[t])
          ? color_t{0x00, 0xAA, 0xEE, 0xFF}
          : color_t{0x00, 0xEE, 0x42, 0xFF};

        Debug::drawLine(v0, v1, color);
        Debug::drawLine(v1, v2, color);
        Debug::drawLine(v2, v0, color);
      }
    }
  }

  if(showSpheres) {
    for(const auto &sphere : spheres) {
      if(sphere->mask == 0)continue;

      color_t col{0xFF, 0xFF, 0x00, 0xFF};
      if(sphere->interactType & InteractType::TRI_MESH) {
        col = color_t{0x44, 0x44, 0x44, 0xFF};
        bool isFloor = sphere->hitTriTypes & TriType::FLOOR;
        bool isWall = sphere->hitTriTypes & TriType::WALL;
        if(isFloor)col.b = 0xEE;
        if(isWall)col.r = 0xEE;
      }

      Debug::drawSphere(sphere->center * 16.0f, sphere->radius * 16.0f, col);
    }

    for(uint32_t v=0; v<VOID_SPHERE_COUNT; ++v) {
      if(voidSpheres[v].radius <= 0)continue;
      Debug::drawSphere(voidSpheres[v].center * 16.0f, voidSpheres[v].radius * 16.0f, {0x00, 0x00, 0x00, 0xFF});
    }
  }
}

bool Coll::Scene::isInVoid(const T3DVec3 &pos) const {
  for(uint32_t v=0; v<VOID_SPHERE_COUNT; ++v) {
    float radSum = voidSpheres[v].radius;
    if(radSum <= 0)continue;
    auto dist2 = t3d_vec3_distance2(voidSpheres[v].center, pos);
    if(dist2 < radSum * radSum) {
      return true;
    }
  }
  return false;
}
