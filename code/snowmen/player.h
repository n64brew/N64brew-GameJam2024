#ifndef PLAYER_HEADER
#define PLAYER_HEADER

#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

#include "actor.h"
#include "pickup.h"
#include "triggerActor.h"
#include "AStar.h"

enum EPlayerState {
  EPS_Idle,
  EPS_Walking,
  EPS_Running,
  EPS_Airborne,
  EPS_EngagedInAttack,
  EPS_NoClip,
  EPS_Stunned
};

enum EPlayerAIState {
  EPAIS_Idle,
  EPAIS_Walking //need multiple walking states?
};

enum EPlayerId {
  EPID_1,
  EPID_2,
  EPID_3,
  EPID_4
};

enum EAIGoalType {
  EAIGT_PickupIdle,
  EAIGT_SpawnerPickup,
  EAIGT_Snowman,
  EAIGT_Random
};

typedef struct PlayerStruct{
    Actor PlayerActor;
    enum EPlayerState PlayerState;
    T3DVec3 DesiredVelocity;
    T3DVec3 DesiredVelocityLocal;
    float MaxSpeedChange;
    T3DVec3 NewDirection;//feel like this implementation should be re-done
    float newAngle;
    T3DVec3 CurrentVelocityLocal;
    float MaxSpeed;
    float MaxAcceleration;
    float AirAcceleration;
    float MaxWalkingSpeed;
    float MaxWalkingAcceleration;
    float MaxRunningSpeed;
    float MaxRunningAcceleration;
    float MaxJumpAcceleration;
    float VerticalAccelerationChange;
    float CurrentVerticalAcceleration;
    float CurrentVerticalSpeed;
    float currentSpeed;
    float VerticalMovement;
    float Friction;//10.f
    T3DAnim animIdle;
    T3DAnim animWalk;
    T3DAnim animRun;
    T3DAnim animEngaged;
    T3DAnim animDodge;
    T3DAnim animAttack;
    float animBlend;
    T3DSkeleton skel;
    T3DSkeleton skelBlend;
    enum EPlayerId playerId;
    struct PickupStruct* heldPickup;
    struct TriggerStruct attackTrigger;
    bool attackActive;
    float attackTimer;
    float stunTimer;
    int ai_path_index;
    bool isAI;
    NodeDynamicArray AIPath;
    enum EAIGoalType AIGoalType;
    bool isDestGoalPickupDirectAI;
    enum EPlayerAIState AIState;
    struct PickupStruct* AIGoalPickup;
    float AI_x;
    float AI_y;
    float AIStuckTimer;
    float AINoGoalTimer;
    T3DVec3 AI_InitialPosition;
    wav64_t sfx_woosh;
    wav64_t sfx_hit;
    float invincibleTimer;
    T3DMat4 ArrowTransform;
    T3DMat4FP *ArrowTransformFP;
    T3DModel* modelArrow;
    rspq_block_t* dplArrow;
    float arrowOffset;
} PlayerStruct;

void CreatePlayer(PlayerStruct* playerStruct);

void PlayerInit(struct PlayerStruct* playerStruct, enum EPlayerId playerId);

void PlayerLoop(struct PlayerStruct* playerStruct, joypad_buttons_t* btn, Actor** AllActors, float XStick, float YStick, T3DVec3* cameraFront, T3DVec3* cameraUp, float deltaTime);

void PlayerAnimUpdate(struct PlayerStruct* playerStruct, T3DVec3* SnowmanPosition, int snowmanLevel, rspq_syncpoint_t syncPoint, float deltaTime);

bool GroundedMovement(PlayerStruct* ThisPlayer, Actor** AllActors, float XStick, float YStick, T3DVec3* cameraFront, T3DVec3* cameraUp, float deltaTime);

void Player_Jump(PlayerStruct* ThisPlayer, float deltaTime);

void Player_RunBegin(PlayerStruct* ThisPlayer);

void Player_RunEnd(PlayerStruct* ThisPlayer);

void PlayerCleanup(PlayerStruct* playerStruct);

void PlayerAttackBegin(PlayerStruct* playerStruct, float XStick, float YStick);

void PlayerAttackEnd(PlayerStruct* playerStruct);

void PlayerStun(PlayerStruct* playerStruct);

void PlayerStunEnd(PlayerStruct* playerStruct);

void PlayerFree(PlayerStruct* playerStruct);


#endif