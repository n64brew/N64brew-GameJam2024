/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

namespace Mesh
{
  inline T3DObject* recordFirstObject(T3DModel *model) {
    T3DObject* obj = t3d_model_get_object_by_index(model, 0);
    rspq_block_begin();
      t3d_model_draw_object(obj, nullptr);
    obj->userBlock = rspq_block_end();
    return obj;
  }

  inline T3DObject* recordObject(T3DModel *model, const char* name) {
    T3DObject* obj = t3d_model_get_object(model, name);
    rspq_block_begin();
      t3d_model_draw_object(obj, nullptr);
    obj->userBlock = rspq_block_end();
    return obj;
  }
}