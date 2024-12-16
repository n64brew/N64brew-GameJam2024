//Spawner should have multiple models for empty or full

#ifndef DECORATIONSPAWNER_HEADER
#define DECORATIONSPAWNER_HEADER

#include "pickup.h"
#include "actor.h"
#include "triggerActor.h"
#include "player.h"

#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"





typedef struct DecorationSpawnerStruct{
  //T3DVec3 Pos;
  //bool isOccupied;
  struct Actor spawnerActor;
  struct TriggerStruct spawnerTrigger;
  struct PickupStruct decorations[2];
  enum EDecorationType decorationType;
  int numInactive;
    T3DModel* modelBase;
    rspq_block_t* dplBase;
} DecorationSpawnerStruct;

void CreateSpawner(struct DecorationSpawnerStruct* decorationSpawner);

void SpawnerInit(struct DecorationSpawnerStruct* decorationSpawner, enum EDecorationType decorationType);

void SpawnerLoop(struct DecorationSpawnerStruct* decorationSpawner, Camera* camera, float deltaTime, T3DViewport* viewport);

//SpawnDecoration, apply to player immediatley, must check if any inactive (available) before spawn
void SpawnDecoration(struct DecorationSpawnerStruct* decorationSpawner, struct PlayerStruct* player);


void DecorationDeactivate(struct PickupStruct* pickupStruct);

void DecorationActivate(struct PickupStruct* pickupStruct);

void SpawnerUpdateModel(struct DecorationSpawnerStruct* decorationSpawner);

void SpawnerFree(struct DecorationSpawnerStruct* decorationSpawner);


#endif