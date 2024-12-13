#ifndef N64

#include "vec.h"
#include "math/vec3.h"
#include "math/mat4.h"
#include "lib/json.hpp"

namespace {
  constexpr float BASE_SCALE = 64.0f;
}

#include "cgltfHelper.h"

#define CGLTF_IMPLEMENTATION
#include "lib/cgltf.h"

#include "binaryFile.h"

#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

namespace {
  struct Actor {
    uint32_t type{};
    int16_t pos[3]{};
    int16_t param{};
  };
  static_assert(sizeof(Actor) == 12);

  constexpr uint32_t strToU32(const char* str) {
    return (str[0] << 24) | (str[1] << 16) | (str[2] << 8) | str[3];
  }

  std::vector<Actor> parseActors(cgltf_data *data)
  {
    std::vector<Actor> res{};
    for(int i=0; i<data->nodes_count; ++i)
    {
      auto node = &data->nodes[i];

      if(!node->mesh && node->extras.data) {

        Actor actor{};
        if(node->has_translation) {
          actor.pos[0] = (int16_t)(node->translation[0] * BASE_SCALE);
          actor.pos[1] = (int16_t)(node->translation[1] * BASE_SCALE);
          actor.pos[2] = (int16_t)(node->translation[2] * BASE_SCALE);
        }

        auto actorJson = nlohmann::json::parse(node->extras.data);

        int type = actorJson["ootEmptyType"].get<int>();
        if(type == 3) { // actor
          //printf("Data: %s\n", actorJson.dump(2).c_str());
          auto name = actorJson["ootActorProperty"]["actorIDCustom"].get<std::string>();
          int param = std::stoi(actorJson["ootActorProperty"]["actorParam"].get<std::string>());
          actor.type = strToU32(name.c_str());
          actor.param = (int16_t)param;
          res.push_back(actor);
        }
      }
    }
    // sort actors by type
    std::sort(res.begin(), res.end(), [](const Actor &a, const Actor &b) {
      return a.type < b.type;
    });

    return res;
  }
}

int main(int argc, char** argv)
{
  const char* gltfPath = argv[1];
  const char* scenePath = argv[2];
  fs::path gltfBasePath{argv[1]};
  gltfBasePath = gltfBasePath.parent_path();

  cgltf_options options{};
  cgltf_data* data = nullptr;
  cgltf_result result = cgltf_parse_file(&options, gltfPath, &data);

  if(result == cgltf_result_file_not_found) {
    throw std::runtime_error("File not found!");
  }
  if(cgltf_validate(data) != cgltf_result_success) {
    throw std::runtime_error("Invalid glTF data!");
  }

  cgltf_load_buffers(&options, data, gltfPath);

  auto actors = parseActors(data);
  BinaryFile sceneFile{};
  sceneFile.write<uint32_t>(actors.size());

  for(const auto &actor : actors) {
    sceneFile.write<uint32_t>(actor.type);
    sceneFile.writeArray(actor.pos, 3);
    sceneFile.write<int16_t>(actor.param);
  }

  sceneFile.writeToFile(scenePath);
}

#endif