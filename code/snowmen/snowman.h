#ifndef SNOWMAN_HEADER
#define SNOWMAN_HEADER

//#include <libdragon.h>
//#include "../../core.h"
//#include "../../minigame.h"

#include "actor.h"
#include "player.h"
#include "pickup.h"
#include "triggerActor.h"

#define SNOWBALL0        0
#define SNOWBALL1        1
#define SNOWBALL2        2
#define HAT              3
#define CARROT           4
#define FACE             5
#define SCARF            6
#define MITT             7
#define STICK            8
#define STONESTORSO      9
#define STONESBOTTOM     10

#define RED              color_from_packed32(0xFF0000<<8)
#define BLUE             color_from_packed32(0x0000FF<<8)
#define YELLOW           color_from_packed32(0xFFFF00<<8)
#define GREEN            color_from_packed32(0x00FF00<<8)


enum ESnowmanLevel {
  ESL_level0,
  ESL_level1,
  ESL_level2,
  ESL_level3
};

enum ESnowmanState {
  ESS_Empty,
  ESS_Decoration1,
  ESS_Decoration2,
  ESS_Decoration3,
  ESS_Decoration4,
  ESS_Decoration5,
};

/*enum ESnowmanOwner {
  ESO_Player1,
  ESO_Player2,
  ESO_Player3,
  ESO_Player4,
  ESO_Team1,
  ESO_Team2
};*/

typedef struct SnowmanStruct{
    enum ESnowmanLevel snowmanLevel;
    enum ESnowmanState snowmanState;
    enum EPlayerId snowmanOwner;
    Actor snowmanActor;
    TriggerStruct triggerStruct;
    uint8_t decorations;
    int BillboardValue;
    T3DMat4 HeadTransform;
    T3DMat4FP *HeadTransformFP;
    T3DMat4 TorsoTransform;
    T3DMat4FP *TorsoTransformFP;
    T3DMat4 StickTransform;
    T3DMat4FP *StickTransformFP;
    float height;
    float height2nd;
    float stickHeight;
    T3DModel* models[11];
    rspq_block_t* dpls[11];
    T3DModel* modelBase;
    rspq_block_t* dplBase;
    float Rotation;
    wav64_t sfx_collect;
} SnowmanStruct;

void CreateSnowman(struct SnowmanStruct* snowmanStruct);

void SnowmanInit(struct SnowmanStruct* snowmanStruct, enum EPlayerId newOwner);

void SnowmanLoop(struct SnowmanStruct* snowmanStruct, float deltatime);

bool SnowmanAttemptAdd(struct SnowmanStruct* snowmanStruct, struct PlayerStruct* playerStruct, struct PickupStruct* pickupStruct, bool* GameEnd);//player must have playernum as part of it

void SnowmanDraw(struct SnowmanStruct* snowmanStruct);

void SnowmanDrawAll(struct SnowmanStruct* snowmanStruct);

void SnowmanUpdateModel(struct SnowmanStruct* snowmanStruct);

void SnowmanFree(struct SnowmanStruct* snowmanStruct);

#endif