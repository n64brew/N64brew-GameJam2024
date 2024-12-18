/**
* @copyright 2023 - Max Bebök
* @license MIT
*/
#include <stdio.h>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cassert>

#include "structs.h"
#include "parser.h"
#include "hash.h"
#include "args.h"

#include "binaryFile.h"
#include "converter/converter.h"
#include "parser/rdp.h"
#include "optimizer/optimizer.h"

Config config;

namespace fs = std::filesystem;

namespace {
  uint32_t insertString(std::string &stringTable, const std::string &newString) {
    auto strPos = stringTable.find(newString);
    if(strPos == std::string::npos) {
      strPos = stringTable.size();
      stringTable += newString;
      stringTable.push_back('\0');
    }
    return strPos;
  }

  int writeBone(BinaryFile &file, const Bone &bone, std::string &stringTable, int level) {
    //printf("Bone[%d]: %s -> %d\n", bone.index, bone.name.c_str(), bone.parentIndex);

    file.write(insertString(stringTable, bone.name));
    file.write<uint16_t>(bone.parentIndex);
    file.write<uint16_t>(level); // level

    auto normPos = bone.pos * config.globalScale;
    file.writeArray(bone.scale.data, 3);
    file.writeArray(bone.rot.data, 4);
    file.writeArray(normPos.data, 3);

    int boneCount = 1;
    for(const auto& child : bone.children) {
      boneCount += writeBone(file, *child, stringTable, level+1);
    }
    return boneCount;
  };

  std::string getRomPath(const std::string &path) {
    if(path.find("filesystem/") == 0) {
      return std::string("rom:/") + path.substr(11);
    }
    return path;
  }

  std::string getStreamDataPath(const char* filePath, uint32_t idx) {
    auto sdataPath = std::string(filePath).substr(0, std::string(filePath).size()-5);
    std::replace(sdataPath.begin(), sdataPath.end(), '\\', '/');
    return sdataPath + "." + std::to_string(idx) + ".sdata";
  }
}

int main(int argc, char* argv[])
{
    EnvArgs args{argc, argv};
  if(args.checkArg("--help")) {
    printf("Usage: %s <gltf-file> <t3dm-file> [--bvh] [--base-scale=64] [--ignore-materials] [--verbose]\n", argv[0]);
    return 1;
  }
  
  const std::string gltfPath = args.getFilenameArg(0);
  std::string t3dmPath = args.getFilenameArg(1);
  std::string replacement = ".col";
  std::string toReplace = ".t3dm";
  size_t pos = t3dmPath.find(toReplace);
  t3dmPath.replace(pos, toReplace.size(), replacement);

  printf("gltfPath: %s & t3dmPath%s\n", gltfPath.c_str(), t3dmPath.c_str());

  auto allModels = parseGLTFCustom(gltfPath.c_str(), config.globalScale);
  fs::path gltfBasePath{gltfPath};
  
  uint16_t totalTriCount = 0;

  BinaryFile file{};
  file.writeChars("COL", 3);
  //file.write(chunkCount);
  file.write<uint8_t>(42);
  file.write<uint16_t>(69); // total vertex count (set later)
  file.write<uint16_t>(420); // total index count (set later)
  /*file.write(allModels[0].triangles[0].vert[0].pos[0]);
  file.write(allModels[0].triangles[0].vert[0].pos[1]);
  file.write(allModels[0].triangles[0].vert[0].pos[2]);*/
  for (int i = 0; i < allModels[0].triangles.size(); i++)
  {
    for (int j = 0; j < 3; j++)
    {
      file.write(allModels[0].triangles[i].vert[j].pos[0]);
      file.write(allModels[0].triangles[i].vert[j].pos[1]);
      file.write(allModels[0].triangles[i].vert[j].pos[2]);
      
    }
    totalTriCount++;
  }

  file.setPos(0x04);
  file.write(totalTriCount);

  file.writeToFile(t3dmPath.c_str());
}