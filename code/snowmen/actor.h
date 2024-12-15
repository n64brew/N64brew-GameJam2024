#ifndef ACTOR_HEADER
#define ACTOR_HEADER

#include "collision.h"

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

typedef struct {
  int16_t v[3];
} int16Vec;

typedef struct {
  int16Vec verts[3];
} triangleCollision;

typedef struct {
  char magic[4];
  uint16_t totalTriCount;
  uint16_t totalIndexCount;

  triangleCollision tris[];
} CollisionStruct;

enum ActorTypes {
    EAT_Player,
    EAT_Crate,
    EAT_Pickup,
    EAT_NPC,
    EAT_TRIGGER,
    EAT_SPAWNER
};

typedef struct Octree{
    T3DVec3 AABB_Min;
    T3DVec3 AABB_Max;
    struct Octree *children;
    T3DVec3 *vertices;
    int numVertices;
} Octree;


typedef struct Actor{
    enum ActorTypes actorType;
    T3DModel *model;
    char* collisionModelPath;
    bool hasCollision;
    bool isDynamic;
    void (*initFunc)(struct Actor *actor);
    void (*loopFunc)(struct Actor *actor, float deltaTime);
    enum CollisionTypes collisionType;
    float collisionRadius;//or half width
    T3DVec3 collisionCenter;
    float CollisionHeight;//generate tip and base from center and height
    T3DVec3 CurrentVelocity;
    T3DVec3 DesiredMovement;
    T3DVec3 Position;
    T3DVec3 PrevPosition;
    T3DVec3 AABB_Min;
    T3DVec3 AABB_Max;
    T3DMat4 Transform;
    T3DMat4FP *TransformFP;
    T3DVec3 *CollisionVertices;//large array of 3 verts (tris)
    int numCollisionTris;
    Octree CollisionOctree;
    rspq_block_t *dpl;
    T3DVec3 BillboardPosition;
    float BillboardTimer;
} Actor;

void CreateNewNPC(Actor* CrateActor, float width, float height, const T3DVec3* center, T3DMat4* Transform);

void NPCInit(struct Actor* actor, T3DMat4* Transform);

void NPCLoop(struct Actor* actor, float deltaTime);

void ActorInit(struct Actor* actor);

void ActorLoop(struct Actor* actor, float deltaTime);

float ApplyFriction(float SpeedValue, float friction, float deltaTime);

float MoveTowards(float CurrentVelocity1d, float DesiredVelocity1d, float MaxSpeedChange, float friction, float deltaTime);

T3DVec3 MoveTowardsVector(T3DVec3 CurrentVelocity, T3DVec3 DesiredVelocity, float MaxSpeedChange);//NOT EVEN USED LMAO

bool TestAllCollision(Actor* InstigatorActor, Actor** AllActors, T3DVec3* penetration_normal, float* penetration_depth, float deltaTime);//for now, for loop through all actors/terrain

bool TestCollision(Actor* InstigatorActor, Actor* OtherActor, T3DVec3* penetration_normal, float* penetration_depth, float deltaTime);//Dynamic against Static, eventually replace ifs with nested switch/case

bool TestCapsuleMeshCollision(Actor* CapsuleActor, Actor* StaticMeshActor, T3DVec3* penetration_normal, float* penetration_depth, float deltaTime);

bool TestCapsuleEnvCollision(Actor* CapsuleActor, Actor* CHANGEEnvstruct, float deltaTime);

bool TestCapsuleCapsuleCollision(Actor* CapsuleActor1, Actor* CapsuleActor2, float deltaTime);

void CapsuleRespondCollideNSlide(Actor* CapsuleActor, T3DVec3* penetration_normal, float penetration_depth, float deltaTime);

bool CollideCapsuleMeshCached(struct Actor* actor, const CapsuleCollider* capsule, T3DVec3* penetration_normal, float* penetration_depth); 

void CalcCapsuleAABB(struct Actor* playerActor);

void GenerateStaticCollisionNew(struct Actor* actor);

void ActorFree(Actor* actor);

#endif