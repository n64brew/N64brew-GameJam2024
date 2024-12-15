/**
 * @copyright 2023 - Max Bebök
 * @license MIT
 */
#pragma once

#include <stdexcept>
#include "lib/cgltf.h"

namespace Gltf
{
  inline int getDataSize(cgltf_component_type type)
  {
    switch(type) {
      case cgltf_component_type_r_8: return 1;
      case cgltf_component_type_r_8u: return 1;
      case cgltf_component_type_r_16: return 2;
      case cgltf_component_type_r_16u: return 2;
      case cgltf_component_type_r_32u: return 4;
      case cgltf_component_type_r_32f: return 4;
      default: return 0;
    }
  }

  inline float readAsFloat(const uint8_t* data, cgltf_component_type type) {
    switch(type) {
      case cgltf_component_type_r_8: return (float)(*(int8_t*)data) / 127.0f;
      case cgltf_component_type_r_8u: return (float)(*(uint8_t*)data) / 255.0f;
      case cgltf_component_type_r_16: return (float)(*(int16_t*)data) / 32767.0f;
      case cgltf_component_type_r_16u: return (float)(*(uint16_t*)data) / 65535.0f;
      case cgltf_component_type_r_32u: return (float)(*(uint32_t*)data);
      case cgltf_component_type_r_32f: return (float)(*(float*)data);
      default: return 0.0f;
    }
  }

  inline Vec3 readAsVec3(uint8_t* &data, cgltf_type type, cgltf_component_type compType) {
    Vec3 result{};
    auto dataSize = getDataSize(compType);
    switch(type) {
      case cgltf_type_scalar: {
        result[0] = readAsFloat(data, compType);
        result[1] = result[0];
        result[2] = result[0];
        data += dataSize;
        break;
      }
      case cgltf_type_vec2: {
        result[0] = readAsFloat(data, compType);
        result[1] = readAsFloat(data + getDataSize(compType), compType);
        result[2] = 0.0f;
        data += dataSize * 2;
        break;
      }
      case cgltf_type_vec4:
      case cgltf_type_vec3: {
        result[0] = readAsFloat(data, compType);
        result[1] = readAsFloat(data + getDataSize(compType), compType);
        result[2] = readAsFloat(data + getDataSize(compType) * 2, compType);
        data += dataSize * 3;
        break;
      }
      default:

        throw std::runtime_error("Unsupported type");
    }
    return result;
  }

  inline uint32_t readAsU32(uint8_t* data, cgltf_component_type type) {
    switch(type) {
      case cgltf_component_type_r_8: return (uint32_t)(*(int8_t*)data);
      case cgltf_component_type_r_8u: return (uint32_t)(*(uint8_t*)data);
      case cgltf_component_type_r_16: return (uint32_t)(*(int16_t*)data);
      case cgltf_component_type_r_16u: return (uint32_t)(*(uint16_t*)data);
      case cgltf_component_type_r_32u: return (uint32_t)(*(uint32_t*)data);
      case cgltf_component_type_r_32f: return (uint32_t)(*(float*)data);
      default: return 0;
    }
  }
}