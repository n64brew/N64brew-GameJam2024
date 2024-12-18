#ifndef PICKUP_HEADER
#define PICKUP_HEADER

#include <libdragon.h>
//#include "../../core.h"
//#include "../../minigame.h"
#include <t3d/t3d.h>

#include "actor.h"
#include "player.h"
#include "triggerActor.h"
#include "SpawnLocation.h"
#include "camera.h"

enum EPickUpState {
  EPUS_Idle,
  EPUS_PickedUp,
  EPUS_Inactive
};

enum EPickUpType {
  EPUT_Snowball,
  EPUT_Decoration
};

enum EDecorationType {
  EDT_Empty,
  EDT_Decoration1,
  EDT_Decoration2,
  EDT_Decoration3,
  EDT_Decoration4,
  EDT_Decoration5,
  EDT_Decoration6
};

typedef struct PickupStruct{
    enum EPickUpState pickupState;
    enum EPickUpType pickupType;
    enum EDecorationType decorationType;
    Actor pickupActor;
    struct PlayerStruct* holdingPlayerStruct;
    float snowballSize;
    float maxSnowballSize;
    T3DVec3 prevPosition;
    TriggerStruct triggerStruct;
    struct SpawnLocation *spawnLocationPtr;
    T3DVec3 Rotation;
    float scale;
    rspq_block_t* dplAltSnowball;
} PickupStruct;


void CreatePickup(struct PickupStruct* pickupStruct);

void PickupInit(struct PickupStruct* pickupStruct, enum EDecorationType decoType);

void PickupLoop(struct PickupStruct* pickupStruct, Camera* camera, float deltaTime, T3DViewport* viewport);

void PickupSetLocation(struct PickupStruct* PickupStruct, const T3DVec3* newLocation);

void PlayerPicksUp(struct PickupStruct* pickupStruct,  struct PlayerStruct* playerStruct);

void PlayerDrops(struct PickupStruct* pickupStruct, struct PlayerStruct* playerStruct);

void PickupDelete(struct PickupStruct* pickupStruct);

void PickupSpawn(struct PickupStruct* pickupStruct);//Add Spawn Location Struct

//void SpawnSnowball();

void PickupDeactivate( PickupStruct* pickupStruct);

void PickupActivate( PickupStruct* pickupStruct, struct SpawnLocation* location);

void PickupSetType( PickupStruct* pickupStruct, enum EDecorationType decorationType);

void PickupUpdateModel(struct PickupStruct* PickupStruct);

void SnowballSwapColours(struct PickupStruct* pickupStruct, color_t ballColor);

void PickupFree(struct PickupStruct* pickupStruct);

//void awful_look(T3DMat4 *out, const T3DVec3 *eye, const T3DVec3 *dir, const T3DVec3 *up);

//void awful_lookat(T3DMat4 *out, const T3DVec3 *eye, const T3DVec3 *target, const T3DVec3 *up);


#endif