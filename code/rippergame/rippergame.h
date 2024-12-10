#ifndef GAMEJAM2024_RIPPERGAME_H
#define GAMEJAM2024_RIPPERGAME_H 

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
    //    T3DAnim animAttack;
    //    T3DAnim animWalk;
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
        // TODO: Remove, temp AI variable
        //int framesRemainingAi;
        //T3DVec3 aiDir;
    } player_data;
    
    typedef struct
    {
        T3DMat4FP* modelMatFP;
        T3DModel* model; // do not load models into this struct as they won't be tracked and free'd only pass copies of pointers
        T3DVec3 effectPos;
        bool isActive;
        float remainingTimer; // self removing timer
        float effectSize;
    } effect_data;
    
    typedef struct
    {
        bool isActive;
        T3DMat4FP* modelMatFP;
        T3DModel* model;
        rspq_block_t* dplObjective;
        T3DVec3 objectivePos;
    } objective_data;
    
    typedef struct
    {
        T3DMat4FP* modelMatFP;
        // rspq_block_t* dplCollision;
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
    } collisionresult_data;

/*********************************
             Functions
*********************************/

// forward declarations
    void player_guardAbility(float deltaTime, int playerNumber);
    void player_theifAbility(float deltaTime, int playerNumber);

    void rippergame_init();
    void rippergame_fixedloop(float deltatime);
    void rippergame_loop(float deltatime);
    void rippergame_cleanup();

#endif