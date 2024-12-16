#ifndef CAMERA_HEADER
#define CAMERA_HEADER

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>

#include "actor.h"

void CreateNewCrate(Actor* CrateActor, float width, float height, const T3DVec3* center, T3DMat4* Transform);

void CrateInit(struct Actor* actor, T3DMat4* Transform);

void CrateInitGiveModel(struct Actor* actor, T3DMat4* Transform, T3DModel *givenModel);

void CrateLoop(struct Actor* actor, float deltaTime);

#endif