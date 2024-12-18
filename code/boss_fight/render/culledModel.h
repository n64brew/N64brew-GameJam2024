/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include <t3d/t3dmodel.h>

class CulledModel
{
  private:
    T3DModel *model{};
    T3DMat4FP* mapMatFP{};

  public:
    CulledModel(const char *modelPath);
    ~CulledModel();

    void update(const T3DVec3 &camPos);
    uint32_t draw(T3DModelState &t3dState);
};
