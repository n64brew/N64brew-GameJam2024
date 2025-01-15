#ifndef GAMEJAM2024_LARCENYGAME_H
#define GAMEJAM2024_LARCENYGAME_H 

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

/*********************************
    Structs for the project
*********************************/

    // Enumerator for teams
    typedef enum
    {
        teamThief,
        teamGuard
    } player_team;
    
    // Enumerator for collision channels
    typedef enum
    {
        collisionAll,
        collisionGuardOnly
    } collision_type;
    
    // The player struct, all info needed by guards and thieves go in here
    typedef struct
    {
        bool isActive;
        int playerNumber;
        player_team playerTeam;
        T3DMat4FP* modelMatFP;
        T3DModel* model;
        T3DAnim animAbility;
        bool animAbilityPlaying;
        T3DAnim animWalk;
        T3DAnim animIdle;
        T3DSkeleton skelBlend;
        T3DSkeleton skel;
        rspq_block_t* dplPlayer;
        T3DVec3 moveDir;
        T3DVec3 playerPos;
        float rotY;
        float currSpeed;
        float animBlend;
        bool isAi;
        int aiIndex;    // index of the AI controller, passed to all AI commands
        float stunTimer; // stunTimer stops players from taking action while count is != 0
        float abilityTimer; // cooldown timer for abilities
    } player_data;
    
    typedef struct
    {
        T3DMat4FP* modelMatFP;
        T3DModel* model; // do not load models into this struct as they won't be tracked and free'd only pass copies of pointers
        T3DVec3 effectPos;
        bool isActive;
        float remainingTimer; // self removing timer
        float effectRotationY;
        float effectSize;
    } effect_data;
    
    typedef struct
    {
        bool isActive;
        T3DMat4FP* modelMatFP;
        T3DModel* objectiveModel;
        T3DModel* ringModel;
        float objectiveRotationY;
        rspq_block_t* dplObjective;
        T3DVec3 objectivePos;
    } objective_data;
    
    typedef struct
    {
        T3DMat4FP* modelMatFP;
        rspq_block_t* dplCollision;
        T3DVec3 collisionCentrePos;
        collision_type collisionType;
        int sizeX;
        int sizeZ;
    } collisionobject_data;
    
    typedef struct
    {
        bool didCollide;
        collision_type collisionType;
        int indexOfCollidedObject;
        T3DVec3 intersectionPoint;
    } collisionresult_data;

    typedef struct
    {
        T3DVec3 camStartPos;
        T3DVec3 camEndPos;
        T3DVec3 lookAtStart;
        T3DVec3 lookAtEnd;
        float timeUntilNextKeyframe;
    } cameraanimation_data;

    typedef struct
    {
        cameraanimation_data* currentAnimation;
        int currentAnimationLength;
        int currentAnimationKeyframe;
        float currentAnimationTime;
        bool currentlyPlaying;
    } animatedcameraobject_data;

/*********************************
             Functions
*********************************/

// forward declarations
    void player_guardAbility(float deltaTime, int playerNumber);
    void player_thiefAbility(float deltaTime, int playerNumber);

    // returns the index of the first unused effect, returns 0 (overwrites the first one) if none free
    int effect_getNextEmptyIndex();

    void larcenygame_init();
    void larcenygame_fixedloop(float deltatime);
    void larcenygame_loop(float deltatime);
    void larcenygame_cleanup();

#endif