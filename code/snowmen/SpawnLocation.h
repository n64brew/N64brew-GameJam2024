#ifndef SPAWNLOCATION_HEADER
#define SPAWNLOCATION_HEADER

#include "pickup.h"
#include "actor.h"

#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"




typedef struct SpawnLocation{
  T3DVec3 Pos;
  bool isOccupied;
  struct PickupStruct* pickupPtr;
} SpawnLocation;


//void SetSpawnLocations(SpawnLocation locations[10]);

SpawnLocation* GetRandomLocation(SpawnLocation locations[6], int seed);

#endif