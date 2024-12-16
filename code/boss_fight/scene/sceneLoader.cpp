/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "scene.h"

namespace {
  struct SceneActor {
    uint32_t type{};
    int16_t pos[3]{};
    uint16_t param{};
  };

  struct SceneFile {
    uint32_t actorCount;
    SceneActor actors[];
  };
}

void Scene::loadScene(const char* path)
{
  SceneFile* scene = (SceneFile*)asset_load(path, nullptr);
  for(uint32_t i = 0; i < scene->actorCount; i++) {
    SceneActor* actor = &scene->actors[i];
    auto pos = T3DVec3{(float)actor->pos[0], (float)actor->pos[1], (float)actor->pos[2]} * (1.0f / 64.0f);

    switch(actor->type)
    {
      // Special actors
      case "Spwn"_u32:
        players[actor->param].setPos(pos);
        camStartPosX = pos.x * COLL_WORLD_SCALE;
      break;
      case "Rset"_u32: respawnPoints.push_back(pos); break;
      case "CEnd"_u32: camEndPosX = pos.x * COLL_WORLD_SCALE; break;
      case "Guid"_u32: navPoints.addPoint(pos); break;

      // Generic actors
      default: spawnActor(actor->type, pos, actor->param); break;
    }
  }
  free(scene);
}