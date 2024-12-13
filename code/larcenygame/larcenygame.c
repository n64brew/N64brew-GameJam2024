/***************************************************************
                         larcenygame.c
                               
RiPpEr253's entry into the N64Brew 2024 Gamejam
***************************************************************/

#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "larcenygame.h"
#include "larcenygameAI.h"

#define COUNTDOWN_DELAY 4.0f
#define FINISH_DELAY 10.0f
#define STARTING_GAME_TIME 60.0f
#define DEFAULT_ABILITY_COOLDOWN 2.0f
#define GUARD_ABILITY_RANGE 50
#define THIEF_ABILITY_RANGE 28
#define THIEF_ABILITY_STRIDE 2.0f
#define OBJECTIVE_TOUCH_DISTANCE 27

#define CONTROLLER_LOWER_DEADZONE 4
#define CONTROLLER_UPPER_DEADZONE 50
#define CONTROLLER_DEFAULT_DEBOUNCE 0.5f


#define FONT_DEBUG 1
#define FONT_BILLBOARD 2

#define RESOLUTION_WIDTH 320
#define RESOLUTION_HEIGHT 240

#define DEFAULT_PLAYER_SCALE 0.375f

/*********************************
             Globals
*********************************/

// Display and T3D math globals
surface_t* depthBuffer;
T3DViewport viewport;
T3DMat4FP* mapMatFP;
T3DVec3 camPos;
T3DVec3 camTarget;
T3DVec3 lightDirVec;
T3DModel* modelMap;
T3DModel* modelCollision;
T3DModel* modelWallJumpEffect;

sprite_t* spriteAButton;

player_data players[MAXPLAYERS];
effect_data effectPool[MAXPLAYERS];
objective_data objectives[2];
collisionobject_data collisionObjects[9];

// drawing variables
rspq_block_t* dplMap;
rspq_syncpoint_t syncPoint;
int resolutionDisplayX;
int resolutionDisplayY;

// fonts
rdpq_font_t* fontDebug;
rdpq_font_t* fontBillboard;

// Sound globals
wav64_t sfx_start;
wav64_t sfx_countdown;
wav64_t sfx_winner;
wav64_t sfx_objectiveCompleted;
wav64_t sfx_guardStunAbility;
wav64_t sfx_thiefJumpAbility;
wav64_t sfx_thiefCaught;

xm64player_t xm_music;

// Gameplay globals
int lastCountdownNumber;
float countdownTimer;
bool gameStarting;
bool gameEnding;
bool gamePaused;
float gamePauseDebounce;
float gameTimeRemaining;

player_team winningTeam;

// You need this function defined somewhere in your project
// so that the minigame manager can work
const MinigameDef minigame_def = {
    .gamename = "Larceny",
    .developername = "RiPpEr253",
    .description = "A 2v2 game pitting Thieves against Guards and their treasures, hold r on start for hires",
    .instructions = "Stick: Move, A: Thief: Wall Jump, Guard: Stun Thief"
};


//TODO remove:
float tempSpeed;
T3DVec3 tempIntersectionPoint;

/*==============================
    debugInfoDraw
    draws debug HUD info using the RDP
==============================*/

void debugInfoDraw(float deltaTime)
{
    rdpq_sync_pipe(); // Hardware crashes otherwise
    rdpq_sync_tile(); // Hardware crashes otherwise

    rdpq_textparms_t textparms = { .align = ALIGN_LEFT, .width = resolutionDisplayX, .disable_aa_fix = true, };
    //rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 10, 80+10, "Test Debug");
    rdpq_sync_tile(); rdpq_sync_pipe(); // make sure the RDP is sync'd Hardware crashes otherwise
    rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 10, 80+20, "FPS: %f", 1.0f/deltaTime);
    //rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 10, 80+30, "AI Statuses: P1: %i P2: %i P3: %i P4: %i", ai_getCurrentStateAsInt(0), ai_getCurrentStateAsInt(1), ai_getCurrentStateAsInt(2), ai_getCurrentStateAsInt(3));
    //rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 10, 80+40, "Intersection point: %f, %f", tempIntersectionPoint.x, tempIntersectionPoint.z);
    //rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 10, 80+50, "p1 x: %f, p1 y: %f", players[0].playerPos.v[0], players[0].playerPos.v[2]);
    //rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 10, 80+60, "Debounce Status: %f", gamePauseDebounce);
    /*joypad_port_t controllerPort = core_get_playercontroller(0);
    
    if(!joypad_is_connected(controllerPort))
    {
        return;
    }
    else
    {
        joypad_inputs_t joypad = joypad_get_inputs(controllerPort);
        rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 10, 80+40, "p1 stick x: %i, p1 stick y: %i, p1 speed: %f", joypad.stick_x, joypad.stick_y, tempSpeed);
        
        // TODO: remove
        // linear algebra tests
        T3DVec3 tempvec = {0};
        tempvec = (T3DVec3){{0,0,1}};
        tempvec.v[0] = -sinf(players[0].rotY);
        tempvec.v[2] = cosf(players[0].rotY);

        t3d_vec3_add(&tempvec, &tempvec, &players[0].playerPos);

        rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 10, 80+50, "rotation of player: %f, vector position test X: %f, Z: %f",players[0].rotY,tempvec.x, tempvec.z);
    }*/
}

/*==============================
    HUD_Update
    updates HUD elements outside of the draw loop
==============================*/
void HUD_Update(float deltaTime)
{
    if(!gameStarting && !gameEnding && !gamePaused)
    {
        // Update anything needed on the HUD (ability timer bar resizing/game time remaining)
        gameTimeRemaining -= deltaTime;
    }
}

/*==============================
    HUD_draw
    draws main HUD info using the RDP
==============================*/
void HUD_draw()
{
    const T3DVec3 HUDOffsets[] = {
    (T3DVec3){{16.0f, resolutionDisplayY - 32.0f, 0.0f}},
    (T3DVec3){{resolutionDisplayX - 96.0f, resolutionDisplayY - 32.0f, 0.0f}},
    (T3DVec3){{resolutionDisplayX - 96.0f, 16.0f, 0.0f}},
    (T3DVec3){{16.0f, 16.0f, 0.0f}},
    };

    const color_t colours[] = {
    PLAYERCOLOR_1,
    PLAYERCOLOR_2,
    PLAYERCOLOR_3,
    PLAYERCOLOR_4,
    };

    rdpq_textparms_t textparms = { .align = ALIGN_CENTER, .width = resolutionDisplayX, .disable_aa_fix = true, };
    rdpq_sync_tile(); rdpq_sync_pipe(); // make sure the RDP is sync'd Hardware crashes otherwise
    rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 0, 20, "Time Left: %i", (int)gameTimeRemaining);

    // iterate through all the players and draw what we need
    for(int iDx = 0; iDx < MAXPLAYERS; iDx++)
    {
        // if player isn't active, skip
        if(!players[iDx].isActive)
        {
            continue;
        }
        
        // set up some basic vectors for reverse picking
        T3DVec3 billboardPos = (T3DVec3){{
            players[iDx].playerPos.v[0],
            players[iDx].playerPos.v[1],
            players[iDx].playerPos.v[2]
        }};

        T3DVec3 billboardScreenPos;
        t3d_viewport_calc_viewspace_pos(&viewport, &billboardScreenPos, &billboardPos);

        // round the result to the nearest whole number to avoid blurry text
        int x = floorf(billboardScreenPos.v[0]);
        int y = floorf(billboardScreenPos.v[1]);
        
        // make sure the RDP is sync'd
        rdpq_sync_tile();
        rdpq_sync_pipe(); // Hardware crashes otherwise

        rdpq_text_printf(&(rdpq_textparms_t){ .style_id = iDx, .disable_aa_fix = true, }, FONT_BILLBOARD, x-5, y-16, "P%d", iDx+1);
        
        if(players[iDx].stunTimer > 0.0f)
        {
            rdpq_sync_tile(); rdpq_sync_pipe(); // make sure the RDP is sync'd Hardware crashes otherwise
            rdpq_text_printf(&(rdpq_textparms_t){ .style_id = iDx, .disable_aa_fix = true, }, FONT_BILLBOARD, x-35, y-26, "Stunned: %.2f", players[iDx].stunTimer);
        }

        // draw the rest of the text for the HUD
        // draw the A button sprite
        rdpq_set_mode_standard();
        rdpq_mode_alphacompare(128);
        rdpq_sync_load();
        
        rdpq_sync_tile(); rdpq_sync_pipe(); // make sure the RDP is sync'd Hardware crashes otherwise

        rdpq_sprite_blit(spriteAButton, HUDOffsets[iDx].v[0], HUDOffsets[iDx].v[1] - 16, &(rdpq_blitparms_t){ .scale_x = 1.0f,  .scale_y = 1.0f});
        if(players[iDx].playerTeam == teamThief)
        {
            rdpq_sync_tile(); rdpq_sync_pipe(); // make sure the RDP is sync'd Hardware crashes otherwise
            rdpq_text_printf(&(rdpq_textparms_t){ .style_id = iDx, .disable_aa_fix = true, }, FONT_BILLBOARD, HUDOffsets[iDx].v[0] + 24, HUDOffsets[iDx].v[1], "Jump Wall");
            rdpq_sync_tile(); rdpq_sync_pipe(); // make sure the RDP is sync'd Hardware crashes otherwise
            rdpq_text_printf(&(rdpq_textparms_t){ .style_id = iDx, .disable_aa_fix = true, }, FONT_BILLBOARD, HUDOffsets[iDx].v[0], HUDOffsets[iDx].v[1] + 10, "Player %i: Thief", iDx + 1);
        }
        else if(players[iDx].playerTeam == teamGuard)
        {
            rdpq_sync_tile(); rdpq_sync_pipe(); // make sure the RDP is sync'd Hardware crashes otherwise
            rdpq_text_printf(&(rdpq_textparms_t){ .style_id = iDx, .disable_aa_fix = true, }, FONT_BILLBOARD, HUDOffsets[iDx].v[0] + 24, HUDOffsets[iDx].v[1], "Stun Attack");
            rdpq_sync_tile(); rdpq_sync_pipe(); // make sure the RDP is sync'd Hardware crashes otherwise
            rdpq_text_printf(&(rdpq_textparms_t){ .style_id = iDx, .disable_aa_fix = true, }, FONT_BILLBOARD, HUDOffsets[iDx].v[0], HUDOffsets[iDx].v[1] + 10, "Player %i: Guard", iDx + 1);
        }

        // make sure the RDP is sync'd
        rdpq_sync_tile();
        rdpq_sync_pipe(); // Hardware crashes otherwise

        // draw ability cooldown bars
        rdpq_set_mode_fill(RGBA32(0 ,0 ,0 ,128));
        rdpq_fill_rectangle(HUDOffsets[iDx].v[0] - 2, HUDOffsets[iDx].v[1] + 12, HUDOffsets[iDx].v[0] + 85, HUDOffsets[iDx].v[1] + 15);
        
        rdpq_set_mode_fill(colours[iDx]);
        rdpq_fill_rectangle(HUDOffsets[iDx].v[0] - 2, HUDOffsets[iDx].v[1] + 12, HUDOffsets[iDx].v[0] + t3d_lerp(85, -2, (players[iDx].abilityTimer / DEFAULT_ABILITY_COOLDOWN) ) , HUDOffsets[iDx].v[1] + 15);
        
    }
}

/*==============================
    collision_init
    Initialises collision objects array manually
    with positions, sizes and collision types
==============================*/
void collision_init()
{
    collisionObjects[0].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    collisionObjects[0].collisionCentrePos = (T3DVec3){{0.0f, 1.0f, 0.0f}};
    collisionObjects[0].collisionType = collisionAll;
    collisionObjects[0].sizeX = 100;
    collisionObjects[0].sizeZ = 20;
        rspq_block_begin();
        t3d_matrix_push(collisionObjects[0].modelMatFP);
        t3d_model_draw(modelCollision);
        t3d_matrix_pop(1);
    collisionObjects[0].dplCollision = rspq_block_end();
    
    collisionObjects[1].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    collisionObjects[1].collisionCentrePos = (T3DVec3){{-80.0f, 1.0f, 106.0f}};
    collisionObjects[1].collisionType = collisionAll;
    collisionObjects[1].sizeX = 60;
    collisionObjects[1].sizeZ = 12;
        rspq_block_begin();
        t3d_matrix_push(collisionObjects[1].modelMatFP);
        t3d_model_draw(modelCollision);
        t3d_matrix_pop(1);
    collisionObjects[1].dplCollision = rspq_block_end();

    collisionObjects[2].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    collisionObjects[2].collisionCentrePos = (T3DVec3){{-106.0f, 1.0f, -80.0f}};
    collisionObjects[2].collisionType = collisionAll;
    collisionObjects[2].sizeX = 12;
    collisionObjects[2].sizeZ = 60;
        rspq_block_begin();
        t3d_matrix_push(collisionObjects[2].modelMatFP);
        t3d_model_draw(modelCollision);
        t3d_matrix_pop(1);
    collisionObjects[2].dplCollision = rspq_block_end();

    collisionObjects[3].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    collisionObjects[3].collisionCentrePos = (T3DVec3){{-80.0f, 1.0f, -106.0f}};
    collisionObjects[3].collisionType = collisionAll;
    collisionObjects[3].sizeX = 60;
    collisionObjects[3].sizeZ = 12;
        rspq_block_begin();
        t3d_matrix_push(collisionObjects[3].modelMatFP);
        t3d_model_draw(modelCollision);
        t3d_matrix_pop(1);
    collisionObjects[3].dplCollision = rspq_block_end();

    collisionObjects[4].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    collisionObjects[4].collisionCentrePos = (T3DVec3){{106.0f, 1.0f, -80.0f}};
    collisionObjects[4].collisionType = collisionAll;
    collisionObjects[4].sizeX = 12;
    collisionObjects[4].sizeZ = 60;
        rspq_block_begin();
        t3d_matrix_push(collisionObjects[4].modelMatFP);
        t3d_model_draw(modelCollision);
        t3d_matrix_pop(1);
    collisionObjects[4].dplCollision = rspq_block_end();

    collisionObjects[5].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    collisionObjects[5].collisionCentrePos = (T3DVec3){{80.0f, 1.0f, -106.0f}};
    collisionObjects[5].collisionType = collisionAll;
    collisionObjects[5].sizeX = 60;
    collisionObjects[5].sizeZ = 12;
        rspq_block_begin();
        t3d_matrix_push(collisionObjects[5].modelMatFP);
        t3d_model_draw(modelCollision);
        t3d_matrix_pop(1);
    collisionObjects[5].dplCollision = rspq_block_end();

    collisionObjects[6].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    collisionObjects[6].collisionCentrePos = (T3DVec3){{106.0f, 1.0f, 80.0f}};
    collisionObjects[6].collisionType = collisionAll;
    collisionObjects[6].sizeX = 12;
    collisionObjects[6].sizeZ = 60;
        rspq_block_begin();
        t3d_matrix_push(collisionObjects[6].modelMatFP);
        t3d_model_draw(modelCollision);
        t3d_matrix_pop(1);
    collisionObjects[6].dplCollision = rspq_block_end();

    collisionObjects[7].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    collisionObjects[7].collisionCentrePos = (T3DVec3){{80.0f, 1.0f, 106.0f}};
    collisionObjects[7].collisionType = collisionAll;
    collisionObjects[7].sizeX = 60;
    collisionObjects[7].sizeZ = 12;
        rspq_block_begin();
        t3d_matrix_push(collisionObjects[7].modelMatFP);
        t3d_model_draw(modelCollision);
        t3d_matrix_pop(1);
    collisionObjects[7].dplCollision = rspq_block_end();

    collisionObjects[8].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    collisionObjects[8].collisionCentrePos = (T3DVec3){{-106.0f, 1.0f, 80.0f}};
    collisionObjects[8].collisionType = collisionAll;
    collisionObjects[8].sizeX = 12;
    collisionObjects[8].sizeZ = 60;
        rspq_block_begin();
        t3d_matrix_push(collisionObjects[8].modelMatFP);
        t3d_model_draw(modelCollision);
        t3d_matrix_pop(1);
    collisionObjects[8].dplCollision = rspq_block_end();
}

/*==============================
    collision_draw
    Iterates over all collision objects
    and draws them out
    Could be more optimised with display lists,
    but it's intended for debugging
==============================*/
void collision_draw()
{
    int numberOfObjects = sizeof(collisionObjects) / sizeof(collisionObjects[0]);

    for(int iDx = 0; iDx < numberOfObjects; iDx++)
    {
        // update matrix
        t3d_mat4fp_from_srt_euler(collisionObjects[iDx].modelMatFP,
            (float[3]){collisionObjects[iDx].sizeX * 0.00625f, 1.0f,collisionObjects[iDx].sizeZ *  0.00625f},
            (float[3]){0.0f, 0, 0},
            collisionObjects[iDx].collisionCentrePos.v);
        if(collisionObjects[iDx].collisionType == collisionGuardOnly) rdpq_set_prim_color(RGBA32(0,0,255,255));
        else
            rdpq_set_prim_color(RGBA32(255,0,0,255));
        
        rspq_block_run(collisionObjects[iDx].dplCollision);
    }
}

// pass in position to check if intersecting
// adjust the passed in vector accordingly
// return true if colliding

/*==============================
    collision_check
    pass in a collisionresult_data struct
    and a position to check if intersecting
==============================*/
void collision_check(collisionresult_data* returnStruct, T3DVec3* pos)
{
    returnStruct->didCollide = false; returnStruct->collisionType = collisionAll; returnStruct->indexOfCollidedObject = 0; returnStruct->intersectionPoint = (T3DVec3){{0}};

    int numberOfObjects = sizeof(collisionObjects) / sizeof(collisionObjects[0]);

    // iterate over every collision box to check
    for(int iDx = 0; iDx < numberOfObjects; iDx++)
    {
        if( pos->v[0] > collisionObjects[iDx].collisionCentrePos.v[0] - (collisionObjects[iDx].sizeX / 2) &&
            pos->v[0] < collisionObjects[iDx].collisionCentrePos.v[0] + (collisionObjects[iDx].sizeX / 2) &&
            pos->v[2] > collisionObjects[iDx].collisionCentrePos.v[2] - (collisionObjects[iDx].sizeZ / 2) &&
            pos->v[2] < collisionObjects[iDx].collisionCentrePos.v[2] + (collisionObjects[iDx].sizeZ / 2))
        {
            returnStruct->didCollide = true; returnStruct->collisionType = collisionObjects[iDx].collisionType; returnStruct->indexOfCollidedObject = iDx;
            return;
        }
    }
    return;
}

// takes two lines, AB and CD are arrays of 2, with two points, using X and Z
// returns true if there's an intersection, otherwise returns false
bool lineLineIntersectTest(T3DVec3* AB, T3DVec3* CD, T3DVec3* XZ)
{
    // First line for testing is AB, a1x+b1y=c1
    float a1 = AB[1].z - AB[0].z;
    float b1 = AB[0].x - AB[1].x;
    float c1 = a1*(AB[0].x) + b1*(AB[0].z);

    // Intersection line is CD, a2x+b2y=c2
    float a2 = CD[1].z - CD[0].z;
    float b2 = CD[0].x - CD[1].x;
    float c2 = a2*(CD[0].x) + b2*(CD[0].z);

    float determinant = (a1*b2) - (a2*b1);

    if(determinant == 0)
    {
        //no intersection
        return false;
    }
    else
    {
        XZ->x = (b2*c1 - b1*c2) / determinant;
        XZ->z = (a1*c2 - a2*c1) / determinant;

        return true;
    }
}

// a version of collision check that handles intersecting points
void collision_check_intersect(collisionresult_data* returnStruct, T3DVec3* startingPos, T3DVec3* endingPos)
{
    returnStruct->didCollide = false; returnStruct->collisionType = collisionAll; returnStruct->indexOfCollidedObject = 0; returnStruct->intersectionPoint = (T3DVec3){{0}};

    int numberOfObjects = sizeof(collisionObjects) / sizeof(collisionObjects[0]);

    // iterate over every collision box to check
    for(int iDx = 0; iDx < numberOfObjects; iDx++)
    {
        if( endingPos->v[0] > collisionObjects[iDx].collisionCentrePos.v[0] - (collisionObjects[iDx].sizeX / 2) &&
            endingPos->v[0] < collisionObjects[iDx].collisionCentrePos.v[0] + (collisionObjects[iDx].sizeX / 2) &&
            endingPos->v[2] > collisionObjects[iDx].collisionCentrePos.v[2] - (collisionObjects[iDx].sizeZ / 2) &&
            endingPos->v[2] < collisionObjects[iDx].collisionCentrePos.v[2] + (collisionObjects[iDx].sizeZ / 2))
        {
            // fill return struct as normal
            returnStruct->didCollide = true; returnStruct->collisionType = collisionObjects[iDx].collisionType; returnStruct->indexOfCollidedObject = iDx;
            // determine the intersection point of the box
            T3DVec3 IntersectionLine[2]; // line made up of collision checker movement
            IntersectionLine[0] = *endingPos; IntersectionLine[1] = *startingPos;
            // create lines of each of the four sides
            T3DVec3 Line1[2]; // top line
            // use mini arrays to determine the 4 points, counter clockwise wrapped starting top-right
            Line1[0].x = collisionObjects[iDx].collisionCentrePos.v[0] + (collisionObjects[iDx].sizeX / 2); Line1[0].z = collisionObjects[iDx].collisionCentrePos.v[0] - (collisionObjects[iDx].sizeZ / 2);
            Line1[1].x = collisionObjects[iDx].collisionCentrePos.v[0] - (collisionObjects[iDx].sizeX / 2); Line1[1].z = collisionObjects[iDx].collisionCentrePos.v[0] - (collisionObjects[iDx].sizeZ / 2);
            T3DVec3 Line2[2]; // left line
            Line2[0].x = collisionObjects[iDx].collisionCentrePos.v[0] - (collisionObjects[iDx].sizeX / 2); Line2[0].z = collisionObjects[iDx].collisionCentrePos.v[0] - (collisionObjects[iDx].sizeZ / 2);
            Line2[1].x = collisionObjects[iDx].collisionCentrePos.v[0] - (collisionObjects[iDx].sizeX / 2); Line2[1].z = collisionObjects[iDx].collisionCentrePos.v[0] + (collisionObjects[iDx].sizeZ / 2);
            T3DVec3 Line3[2]; // bottom line
            Line3[0].x = collisionObjects[iDx].collisionCentrePos.v[0] - (collisionObjects[iDx].sizeX / 2); Line3[0].z = collisionObjects[iDx].collisionCentrePos.v[0] + (collisionObjects[iDx].sizeZ / 2);
            Line3[1].x = collisionObjects[iDx].collisionCentrePos.v[0] + (collisionObjects[iDx].sizeX / 2); Line3[1].z = collisionObjects[iDx].collisionCentrePos.v[0] + (collisionObjects[iDx].sizeZ / 2);
            T3DVec3 Line4[2]; // right line
            Line4[0].x = collisionObjects[iDx].collisionCentrePos.v[0] + (collisionObjects[iDx].sizeX / 2); Line4[0].z = collisionObjects[iDx].collisionCentrePos.v[0] + (collisionObjects[iDx].sizeZ / 2);
            Line4[1].x = collisionObjects[iDx].collisionCentrePos.v[0] + (collisionObjects[iDx].sizeX / 2); Line4[1].z = collisionObjects[iDx].collisionCentrePos.v[0] - (collisionObjects[iDx].sizeZ / 2);

            T3DVec3 intersectionResult;
            if(lineLineIntersectTest(Line1, IntersectionLine, &intersectionResult))
            {
                returnStruct->intersectionPoint = intersectionResult;
            }
            else if(lineLineIntersectTest(Line2, IntersectionLine, &intersectionResult))
            {
                returnStruct->intersectionPoint = intersectionResult;
            }
            else if(lineLineIntersectTest(Line3, IntersectionLine, &intersectionResult))
            {
                returnStruct->intersectionPoint = intersectionResult;
            }
            else if(lineLineIntersectTest(Line4, IntersectionLine, &intersectionResult))
            {
                returnStruct->intersectionPoint = intersectionResult;
            }

            return;
        }
    }
    return;
}

/*==============================
    collision_cleanup
    The collision object cleanup function,
    frees any memory allocated to collision objects
==============================*/
void collision_cleanup()
{
    int numberOfObjects = sizeof(collisionObjects) / sizeof(collisionObjects[0]);

    for(int iDx = 0; iDx < numberOfObjects; iDx++)
    {
        free_uncached(collisionObjects[iDx].modelMatFP);
        rspq_block_free(collisionObjects[iDx].dplCollision);
    }
}


/*==============================
    end_game
    Sets the game state to ending and performs any tasks that are related
    
    Call when game is entirely over as this starts a timer to send us back to the main menu

    @param the winning team passed in as an enum
==============================*/

void end_game(player_team victoriousTeam)
{
    winningTeam = victoriousTeam;
    // reset the countdown timers
    lastCountdownNumber = COUNTDOWN_DELAY;
    countdownTimer = COUNTDOWN_DELAY;

    // set game state to game ending
    gameEnding = true;
    gameStarting = false;
    gamePaused = false;
    gamePauseDebounce = 0.0f;

    wav64_play(&sfx_winner, 31);

    // interate through teams and set a winner
    for(int iDx = 0; iDx < MAXPLAYERS; iDx++)
    {
        if(players[iDx].playerTeam == winningTeam) core_set_winner(iDx);
    }
}

/*==============================
    player_init
    The player initialization function, determines number of 
    players and assigns AI to players that aren't human
==============================*/
void player_init(int playerNumber)
{
    const color_t colours[] = {
    PLAYERCOLOR_1,
    PLAYERCOLOR_2,
    PLAYERCOLOR_3,
    PLAYERCOLOR_4,
    };

    if(playerNumber < core_get_playercount())
    {
        players[playerNumber].isAi = false;
        players[playerNumber].aiIndex = 0;
    }
    else
    {
        players[playerNumber].isAi = true;
        players[playerNumber].aiIndex = ai_assign(playerNumber);
    }

    // remember that players are zero indexed
    if(playerNumber == 0 || playerNumber == 2)
    {
        players[playerNumber].playerTeam = teamThief;
        players[playerNumber].model = t3d_model_load("rom:/larcenygame/foxThief.t3dm");
    }
    else
    {
        players[playerNumber].playerTeam = teamGuard;
        players[playerNumber].model = t3d_model_load("rom:/larcenygame/dogGuard.t3dm");
    }

    switch(playerNumber)
    {
        case 0:
            players[playerNumber].playerPos = (T3DVec3){{-128, 0.0f, 128}};
        break;
        case 1:
            players[playerNumber].playerPos = (T3DVec3){{128, 0.0f, 128}};
        break;
        case 2:
            players[playerNumber].playerPos = (T3DVec3){{128, 0.0f, -128}};
        break;
        case 3:
            players[playerNumber].playerPos = (T3DVec3){{-128, 0.0f, -128}};
        default:
        break;
    }

    players[playerNumber].isActive = true;
    players[playerNumber].playerNumber = playerNumber;
    players[playerNumber].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    // Instantiate skeletons, they will be used to draw skinned meshes
    players[playerNumber].skel = t3d_skeleton_create(players[playerNumber].model);
    players[playerNumber].skelBlend = t3d_skeleton_clone(&players[playerNumber].skel, false);
    players[playerNumber].animAbility = t3d_anim_create(players[playerNumber].model, "Anim_Ability");
    t3d_anim_set_looping(&players[playerNumber].animAbility, false);
    t3d_anim_set_playing(&players[playerNumber].animAbility, false);
    t3d_anim_attach(&players[playerNumber].animAbility, &players[playerNumber].skelBlend);
    players[playerNumber].animAbilityPlaying = false;
    players[playerNumber].animWalk = t3d_anim_create(players[playerNumber].model, "Anim_Run");
    t3d_anim_attach(&players[playerNumber].animWalk, &players[playerNumber].skelBlend);
    players[playerNumber].animIdle = t3d_anim_create(players[playerNumber].model, "Anim_Idle");
    t3d_anim_attach(&players[playerNumber].animIdle, &players[playerNumber].skel);
    rspq_block_begin();
        t3d_matrix_push(players[playerNumber].modelMatFP);
        rdpq_set_prim_color(colours[playerNumber]);
        t3d_model_draw_skinned(players[playerNumber].model, &players[playerNumber].skel);
        t3d_matrix_pop(1);
    players[playerNumber].dplPlayer = rspq_block_end();
    players[playerNumber].moveDir = (T3DVec3){{0,0,0}};
    players[playerNumber].rotY = 0.0f;
    players[playerNumber].currSpeed = 0.0f;
    players[playerNumber].animBlend = 0.0f;
    players[playerNumber].stunTimer = 0.0f;
    players[playerNumber].abilityTimer = 0.0f;
}

/*==============================
    player_cleanup
    The player cleanup function, frees and unloads
    anything that was allocated during the game
==============================*/

void player_cleanup(int playerNumber)
{
    rspq_block_free(players[playerNumber].dplPlayer);

    t3d_skeleton_destroy(&players[playerNumber].skel);
    t3d_skeleton_destroy(&players[playerNumber].skelBlend);

    t3d_anim_destroy(&players[playerNumber].animIdle);
    t3d_anim_destroy(&players[playerNumber].animWalk);
    t3d_anim_destroy(&players[playerNumber].animAbility);

    t3d_model_free(players[playerNumber].model);
    free_uncached(players[playerNumber].modelMatFP);
}

/*==============================
    player_fixedloop
    Updates players at the exact tickrate of 30 tps
==============================*/

void player_fixedloop(float deltaTime, int playerNumber)
{
    if(gameStarting || gameEnding || gamePaused)
    {
        return;
    }

    if(!players[playerNumber].isActive) return;

    // process timers on player
    
    if(players[playerNumber].stunTimer > 0.0f)
    {
        players[playerNumber].stunTimer -= deltaTime;
        if(players[playerNumber].stunTimer <= 0.0f) players[playerNumber].stunTimer = 0.0f;
    }

    if(players[playerNumber].abilityTimer > 0.0f)
    {
        players[playerNumber].abilityTimer -= deltaTime;
        if(players[playerNumber].abilityTimer <= 0.0f) players[playerNumber].abilityTimer = 0.0f;
    }

    T3DVec3 newDir = {0};
    float speed = 0.0f;

    // only get controller inputs from actual players
    if(!players[playerNumber].isAi && !(players[playerNumber].stunTimer > 0.0f))
    {
        joypad_port_t controllerPort = core_get_playercontroller(playerNumber);
        joypad_inputs_t joypad = joypad_get_inputs(controllerPort);

        // upper and lower deadzones
        float tempJoypadStickX;
        float tempJoypadStickY;

        if(joypad.stick_x > -CONTROLLER_LOWER_DEADZONE && joypad.stick_x < CONTROLLER_LOWER_DEADZONE)
        {   
            tempJoypadStickX = 0.0f;
        }
        else if(joypad.stick_x < -CONTROLLER_UPPER_DEADZONE) tempJoypadStickX = -6.35f;
        else if(joypad.stick_x >  CONTROLLER_UPPER_DEADZONE) tempJoypadStickX =  6.35f;
        else
        {
            tempJoypadStickX = (float)joypad.stick_x * 0.05f;
        }

        if(joypad.stick_y > -CONTROLLER_LOWER_DEADZONE && joypad.stick_y < CONTROLLER_LOWER_DEADZONE)
        {   
            tempJoypadStickY = 0.0f;
        }
        else if(joypad.stick_y < -CONTROLLER_UPPER_DEADZONE) tempJoypadStickY = -6.35f;
        else if(joypad.stick_y >  CONTROLLER_UPPER_DEADZONE) tempJoypadStickY =  6.35f;
        else
        {
            tempJoypadStickY = (float)joypad.stick_y * 0.05f;
        }
        newDir.v[0] = tempJoypadStickX;
        newDir.v[2] = -tempJoypadStickY;
        speed = sqrtf(t3d_vec3_len2(&newDir));
    }

    if(players[playerNumber].isAi && !(players[playerNumber].stunTimer > 0.0f)) // is an AI
    {
        // run the generic ai update function to let the AI state machine worry about it
        newDir.x = 0.0f; newDir.y = 0.0f; newDir.z = 0.0f; speed = 0.0f;
        ai_update(players[playerNumber].aiIndex, deltaTime, &newDir, &speed);
    }

    //TODO: Remove
    //if(playerNumber == 0) tempSpeed = speed;

    if(speed > 0.15f)
    {
        newDir.v[0] /= speed;
        newDir.v[2] /= speed;
        players[playerNumber].moveDir = newDir;

        float newAngle = atan2f(-players[playerNumber].moveDir.v[0], players[playerNumber].moveDir.v[2]);
        players[playerNumber].rotY = t3d_lerp_angle(players[playerNumber].rotY, newAngle, 0.5f);
        players[playerNumber].currSpeed = t3d_lerp(players[playerNumber].currSpeed, speed * 0.3f, 0.15f);
    }
    else
    {
        players[playerNumber].currSpeed *= 0.64f;
    }

    // simulate a move for the player
    T3DVec3 tempPosition = players[playerNumber].playerPos;
    tempPosition.v[0] += players[playerNumber].moveDir.v[0] * players[playerNumber].currSpeed;
    tempPosition.v[2] += players[playerNumber].moveDir.v[2] * players[playerNumber].currSpeed;

    //TODO: remove this
    //if(playerNumber == 0) tempIntersectionPoint = (T3DVec3){{0}};

    // do collision checks here
    collisionresult_data collisionResult;

    collision_check(&collisionResult, &tempPosition);
    
    //collision_check_intersect(&collisionResult, &players[playerNumber].playerPos, &tempPosition);

    //TODO: remove this
    //if(playerNumber == 0) tempIntersectionPoint = collisionResult.intersectionPoint;

    // use collisionResult data to determine if to apply simulated move
    if(!collisionResult.didCollide)
    {
        players[playerNumber].playerPos = tempPosition;
    }
    // Check if the player is a thief and so ignores Guard Only collision
    if(collisionResult.didCollide && collisionResult.collisionType == collisionGuardOnly && players[playerNumber].playerTeam == teamThief)
    {
        players[playerNumber].playerPos = tempPosition;
    }
    // and limit movement inside bounding box
    const float BOX_SIZE = 140.0f;
    if(players[playerNumber].playerPos.v[0] < -BOX_SIZE)players[playerNumber].playerPos.v[0] = -BOX_SIZE;
    if(players[playerNumber].playerPos.v[0] > BOX_SIZE)players[playerNumber].playerPos.v[0] = BOX_SIZE;
    if(players[playerNumber].playerPos.v[2] < -BOX_SIZE)players[playerNumber].playerPos.v[2] = -BOX_SIZE;
    if(players[playerNumber].playerPos.v[2] > BOX_SIZE)players[playerNumber].playerPos.v[2] = BOX_SIZE;

    // use blend based on speed for smooth transitions
    players[playerNumber].animBlend = players[playerNumber].currSpeed / 0.51f;
    if(players[playerNumber].animBlend > 1.0f)players[playerNumber].animBlend = 1.0f;
    if(players[playerNumber].animBlend < 0.01f) t3d_anim_set_time(&players[playerNumber].animWalk, 0.0f);

    // do objective touching check
    // only thieves can complete objectives
    if(players[playerNumber].playerTeam == teamThief)
    {
        // for every objective, check if closer than 10 units
        for(int iDx = 0; iDx < sizeof(objectives) / sizeof(objectives[0]); iDx++)
        {
            // skip if not active
            if(objectives[iDx].isActive == false)
            {
                continue;
            }
            // create a temporary vector
            T3DVec3 tempVec = {0};
            // get the vector offset between the player and the objective positions
            t3d_vec3_diff(&tempVec, &players[playerNumber].playerPos, &objectives[iDx].objectivePos);
            // if objective is closer than 10 units, then count as having been completed
            if(t3d_vec3_len(&tempVec) < OBJECTIVE_TOUCH_DISTANCE)
            {
                objectives[iDx].isActive = false;
                
                wav64_play(&sfx_objectiveCompleted, 30);
            }
        }
    }

    // do thief catching check
    if(players[playerNumber].playerTeam == teamGuard)
    {
        for(int iDx = 0; iDx < MAXPLAYERS; iDx++)
        {
            if(players[iDx].isActive == false || players[iDx].playerTeam == teamGuard)
            {
                continue;
            }
            T3DVec3 tempVec = {0};
            t3d_vec3_diff(&tempVec, &players[playerNumber].playerPos, &players[iDx].playerPos);
            if(t3d_vec3_len(&tempVec) < 10 && players[iDx].stunTimer > 0.0f)
            {
                players[iDx].isActive = false;

                
                wav64_play(&sfx_thiefCaught, 28);
            }
        }
    }
}

/*==============================
    player_guardAbility
    Triggers the guard's ability on the chosen player entity
==============================*/

void player_guardAbility(float deltaTime, int playerNumber)
{
    // stun in an AoE
    for(int iDx = 0; iDx < MAXPLAYERS; iDx++)
    {
        if(players[iDx].isActive == false || players[iDx].playerTeam == teamGuard)
        {
            continue;
        }
        T3DVec3 tempVec = {0};
        t3d_vec3_diff(&tempVec, &players[playerNumber].playerPos, &players[iDx].playerPos);
        if(t3d_vec3_len(&tempVec) < GUARD_ABILITY_RANGE && players[iDx].stunTimer == 0.0f) players[iDx].stunTimer = 1.0f;
    }
    // set the ability timer
    players[playerNumber].abilityTimer = DEFAULT_ABILITY_COOLDOWN;
    
    // play the animation for the ability
    t3d_anim_set_playing(&players[playerNumber].animAbility, true);
    t3d_anim_set_time(&players[playerNumber].animAbility, 0.0f);

    // play sound effect here
    wav64_play(&sfx_guardStunAbility, 29);
}

/*==============================
    player_thiefAbility
    Triggers the thief's ability on the chosen player entity
==============================*/

void player_thiefAbility(float deltaTime, int playerNumber)
{
    T3DVec3 tempUnitisedRotatedVector = {0};
    T3DVec3 tempvec = {0};
    collisionresult_data tempResult;
    bool hasHitAWall = false;
    tempUnitisedRotatedVector = (T3DVec3){{0,0,1}};
    tempUnitisedRotatedVector.v[0] = -sinf(players[playerNumber].rotY);
    tempUnitisedRotatedVector.v[2] = cosf(players[playerNumber].rotY);

    for(float fDx = 1.0f; fDx < THIEF_ABILITY_RANGE; fDx+= THIEF_ABILITY_STRIDE)
    {
        t3d_vec3_scale(&tempvec, &tempUnitisedRotatedVector, fDx);
        t3d_vec3_add(&tempvec, &tempvec, &players[playerNumber].playerPos);

        collision_check(&tempResult, &tempvec);
        if(tempResult.didCollide == true && tempResult.collisionType != collisionGuardOnly)
        {
            hasHitAWall = true;
            continue;
        }
        else if(hasHitAWall && (tempResult.didCollide == false || (tempResult.didCollide == true && tempResult.collisionType == collisionGuardOnly)))
        {
            // set a cooldown if ability worked
            players[playerNumber].abilityTimer = DEFAULT_ABILITY_COOLDOWN;
            
            // play the animation for the ability
            t3d_anim_set_playing(&players[playerNumber].animAbility, true);
            t3d_anim_set_time(&players[playerNumber].animAbility, 0.0f);

            // set the effect model to appear at the entry point of the effect
            int effectPoolIndex = effect_getNextEmptyIndex();
            effectPool[effectPoolIndex].isActive = true;
            effectPool[effectPoolIndex].remainingTimer = 0.25f;
            effectPool[effectPoolIndex].effectPos = players[playerNumber].playerPos;
            effectPool[effectPoolIndex].effectRotationY = players[playerNumber].rotY;
            effectPool[effectPoolIndex].effectSize = 20.0f;

            // make a second effect on the other side of the wall
            effectPoolIndex = effect_getNextEmptyIndex();
            effectPool[effectPoolIndex].isActive = true;
            effectPool[effectPoolIndex].remainingTimer = 0.25f;
            effectPool[effectPoolIndex].effectPos = tempvec;
            effectPool[effectPoolIndex].effectRotationY = players[playerNumber].rotY;
            effectPool[effectPoolIndex].effectSize = 20.0f;

            // move the player
            // TODO: make it a destination and do an animation over to it
            players[playerNumber].playerPos = tempvec;

            // play sound effect here
            // TODO: make this its own sound effect
            wav64_play(&sfx_guardStunAbility, 29);

            break;
        }
    }


    // take out current position
    // Get current location
        //get a vector of 1,1
        // X = Cos(Th), Y = Sin(Th)
        // ex.  Cos(45) = 0.707, Sin(45) = 0.707
        // rotation Th in degrees (not rads)
        // result should be a unitised vector for direction
        // can use this to iterate
	// Get direction
	// loop like X amount of times with difference between current location and direction times length
	// see if the spot is open, if it is, set is as a destination and teleport to the other side
    // if teleported, set abilitytimer

    // scratchpad
    // starting position: 45,10
    // rotation of 45 degrees
    // unitised rotated vector = 0.707, 0.707
    // starting position += (URV * steps)
    // 45.000, 10.000
    // 45.707, 10.707
    // 46.414, 11.414
    // 47.121, 12.121
    // 47.828, 12.828
    // 48.535, 13.535
    // 49.242, 14.242
    // 49.949, 14.949
    // 50.656, 15.656
    // 51.363, 16.363
    // 52.070, 17.070
}

// stops player animations, usually called when game ends
void player_stopAnimations(float deltaTime, int playerNumber)
{
    players[playerNumber].animBlend = 0.0f;
    t3d_anim_update(&players[playerNumber].animIdle, deltaTime);
    t3d_anim_set_speed(&players[playerNumber].animIdle, 1.0f);
    t3d_anim_set_speed(&players[playerNumber].animWalk, players[playerNumber].animBlend);
    t3d_anim_update(&players[playerNumber].animWalk, deltaTime);

    if(players[playerNumber].animAbility.isPlaying) t3d_anim_update(&players[playerNumber].animAbility, deltaTime);

    t3d_skeleton_blend(&players[playerNumber].skel, &players[playerNumber].skel, &players[playerNumber].skelBlend, players[playerNumber].animBlend);

    if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame

    t3d_skeleton_update(&players[playerNumber].skel);
    // update matrix
    t3d_mat4fp_from_srt_euler(players[playerNumber].modelMatFP,
        (float[3]){DEFAULT_PLAYER_SCALE, DEFAULT_PLAYER_SCALE, DEFAULT_PLAYER_SCALE},
        (float[3]){0.0f, players[playerNumber].rotY, 0},
        players[playerNumber].playerPos.v);
}

/*==============================
    player_loop
    Updates players in any way that is not required to be
    on a fixed timebase
==============================*/

void player_loop(float deltaTime, int playerNumber)
{
    if(!players[playerNumber].isActive) return;

    joypad_port_t controllerPort = core_get_playercontroller(playerNumber);
    joypad_buttons_t btn;
    
    if(!joypad_is_connected(controllerPort))
    {
        return;
    }

    if(gameEnding)
    {
        player_stopAnimations(deltaTime, playerNumber);
    }

    if(gamePaused)
    {
        // first pause all animations
        t3d_anim_set_speed(&players[playerNumber].animIdle, 0.0f);
        t3d_anim_set_speed(&players[playerNumber].animWalk, 0.0f);
        t3d_anim_set_speed(&players[playerNumber].animAbility, 0.0f);

        //t3d_anim_update(&players[playerNumber].animIdle, deltaTime);
        //t3d_anim_update(&players[playerNumber].animWalk, deltaTime);
        //if(players[playerNumber].animAbility.isPlaying) t3d_anim_update(&players[playerNumber].animAbility, deltaTime);

        // check for someone pressing start to unpause
        btn = joypad_get_buttons_held(controllerPort);
        if(btn.start && gamePauseDebounce <= 0.0f)
        {
            gamePaused = false;
            gamePauseDebounce = CONTROLLER_DEFAULT_DEBOUNCE;
        }

        if(btn.l && gamePauseDebounce <= 0.0f)
        {
            minigame_end();
        }
    }
    else
    {
        t3d_anim_set_speed(&players[playerNumber].animIdle, 1.0f);
        t3d_anim_set_speed(&players[playerNumber].animWalk, players[playerNumber].animBlend);
        t3d_anim_set_speed(&players[playerNumber].animAbility, 1.0f);
    }

    // TODO: Remove this because it's fucked
    // spam the fuck out of the button
    if(players[playerNumber].isAi && players[playerNumber].playerTeam == teamThief)
    {
        if(!(players[playerNumber].abilityTimer > 0.0f))
        {
            player_thiefAbility(deltaTime, playerNumber);
        }
    }

    if(!players[playerNumber].isAi && !gameStarting && !gameEnding && !gamePaused)
    {
        btn = joypad_get_buttons_held(controllerPort);

        if(btn.start && gamePauseDebounce <= 0.0f)
        {
            gamePauseDebounce = CONTROLLER_DEFAULT_DEBOUNCE;
            gamePaused = true;
        }
        if(btn.c_left) end_game(teamThief);
        if(btn.c_right) end_game(teamGuard);

        // if A button pressed and player team is guard, then use guard ability
        if((btn.a || btn.b) && players[playerNumber].playerTeam == teamGuard)
        {
            // check to see if our ability cooldown is not active
            if(!(players[playerNumber].abilityTimer > 0.0f))
            {
                player_guardAbility(deltaTime, playerNumber);
            }
        }

        // if A button pressed, and team is thief, then start up the thief ability
        if((btn.a || btn.b) && players[playerNumber].playerTeam == teamThief)
        {
            // check to see if our ability cooldown is not active
            if(!(players[playerNumber].abilityTimer > 0.0f))
            {
                player_thiefAbility(deltaTime, playerNumber);
            }
        }
    }

    t3d_anim_update(&players[playerNumber].animIdle, deltaTime);
    t3d_anim_update(&players[playerNumber].animWalk, deltaTime);

    if(players[playerNumber].animAbility.isPlaying) t3d_anim_update(&players[playerNumber].animAbility, deltaTime);

    t3d_skeleton_blend(&players[playerNumber].skel, &players[playerNumber].skel, &players[playerNumber].skelBlend, players[playerNumber].animBlend);

    if(syncPoint)rspq_syncpoint_wait(syncPoint); // wait for the RSP to process the previous frame

    t3d_skeleton_update(&players[playerNumber].skel);
    // update matrix
    t3d_mat4fp_from_srt_euler(players[playerNumber].modelMatFP,
        (float[3]){DEFAULT_PLAYER_SCALE, DEFAULT_PLAYER_SCALE, DEFAULT_PLAYER_SCALE},
        (float[3]){0.0f, players[playerNumber].rotY, 0},
        players[playerNumber].playerPos.v);
}

void player_draw(int playerNumber)
{
    if(!players[playerNumber].isActive) return;

    rspq_block_run(players[playerNumber].dplPlayer);
}

void objective_init()
{
    objectives[0].isActive = true;
    objectives[0].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    objectives[0].objectiveModel = t3d_model_load("rom:/larcenygame/diamondObjective.t3dm");
    objectives[0].ringModel = t3d_model_load("rom:/larcenygame/ringObjective.t3dm");
    rspq_block_begin();
        t3d_matrix_push(objectives[0].modelMatFP);
        rdpq_set_prim_color(RGBA32(255,255,255,255));
        t3d_model_draw(objectives[0].objectiveModel);
        rdpq_set_prim_color(RGBA32(0,255,0,255));
        t3d_model_draw(objectives[0].ringModel);
        t3d_matrix_pop(1);
    objectives[0].dplObjective = rspq_block_end();
    //objectives[0].objectivePos = (T3DVec3){{96, 0.0f, 96}};
    objectives[0].objectiveRotationY = 0.0f;
    objectives[0].objectivePos = (T3DVec3){{0.0f, 0.0f, 128}};
    
    objectives[1].isActive = true;
    objectives[1].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
    objectives[1].objectiveModel = t3d_model_load("rom:/larcenygame/theKilogramObjective.t3dm");
    objectives[1].ringModel = t3d_model_load("rom:/larcenygame/ringObjective.t3dm");
    rspq_block_begin();
        t3d_matrix_push(objectives[1].modelMatFP);
        rdpq_set_prim_color(RGBA32(255,255,255,255));
        t3d_model_draw(objectives[1].objectiveModel);
        rdpq_set_prim_color(RGBA32(255,0,0,255));
        t3d_model_draw(objectives[1].ringModel);
        t3d_matrix_pop(1);
    objectives[1].dplObjective = rspq_block_end();
    objectives[1].objectiveRotationY = 0.0f;
    //objectives[1].objectivePos = (T3DVec3){{-96, 0.0f, -96}};
    objectives[1].objectivePos = (T3DVec3){{0.0f, 0.0f, -128}};
}

void objective_update(float deltaTime)
{
    if(objectives[0].isActive)
    {
    objectives[0].objectiveRotationY += deltaTime;
    // update matricies
    t3d_mat4fp_from_srt_euler(objectives[0].modelMatFP,
        (float[3]){0.125f, 0.125f, 0.125f},
        (float[3]){0.0f, objectives[0].objectiveRotationY, 0.0f},
        objectives[0].objectivePos.v);
    }
    if(objectives[1].isActive)
    {
        objectives[1].objectiveRotationY += deltaTime;
        t3d_mat4fp_from_srt_euler(objectives[1].modelMatFP,
            (float[3]){0.125f, 0.125f, 0.125f},
            (float[3]){0.0f, objectives[1].objectiveRotationY, 0.0f},
            objectives[1].objectivePos.v);  
    }
}

void objective_draw()
{
    // if objective is active, then go ahead and draw it
    if(objectives[0].isActive) rspq_block_run(objectives[0].dplObjective);
    if(objectives[1].isActive) rspq_block_run(objectives[1].dplObjective);
}

void objective_cleanup()
{
    rspq_block_free(objectives[0].dplObjective);
    t3d_model_free(objectives[0].objectiveModel);
    t3d_model_free(objectives[0].ringModel);
    free_uncached(objectives[0].modelMatFP);

    rspq_block_free(objectives[1].dplObjective);
    t3d_model_free(objectives[1].objectiveModel);
    t3d_model_free(objectives[1].ringModel);
    free_uncached(objectives[1].modelMatFP);
}

void effect_init()
{
    for(int iDx = 0; iDx < MAXPLAYERS; iDx++)
    {
        effectPool[iDx].isActive = false;
        effectPool[iDx].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
        effectPool[iDx].model = modelWallJumpEffect;
        effectPool[iDx].effectPos = (T3DVec3){{128.0f, 1.0f, 0.0f}};
        effectPool[iDx].remainingTimer = 0.0f;
        effectPool[iDx].effectRotationY = 0.0f;
        effectPool[iDx].effectSize = 1.0f;
    }
}

// returns the index of the first unused effect, returns 0 (overwrites the first one) if none free
int effect_getNextEmptyIndex()
{
    int tempReturnValue = 0;

    for(int iDx = 0; iDx < MAXPLAYERS; iDx++)
    {
        if(!effectPool[iDx].isActive)
        {
            tempReturnValue = iDx;
            break;
        }
    }

    return tempReturnValue;
}

void effect_update(float deltaTime)
{
    for(int iDx = 0; iDx < MAXPLAYERS; iDx++)
    {
        if(effectPool[iDx].remainingTimer > 0.0f)
        {
            effectPool[iDx].remainingTimer -= deltaTime;
            if(effectPool[iDx].remainingTimer < 0.0f) 
            {   
                effectPool[iDx].remainingTimer = 0.0f;
                effectPool[iDx].isActive = false;
            }
        }
    }
}

void effect_draw()
{
    for(int iDx = 0; iDx < MAXPLAYERS; iDx++)
    {
        if(effectPool[iDx].isActive)
        {
            t3d_mat4fp_from_srt_euler(effectPool[iDx].modelMatFP,
            (float[3]){0.00625f * effectPool[iDx].effectSize, 0.00625f * effectPool[iDx].effectSize, 0.00625f * effectPool[iDx].effectSize},
            (float[3]){0.0f, effectPool[iDx].effectRotationY, 0.0f},
            effectPool[iDx].effectPos.v);

            t3d_matrix_push(effectPool[iDx].modelMatFP);
            rdpq_set_prim_color(RGBA32(255,255,255,255));
            t3d_model_draw(effectPool[iDx].model);
            t3d_matrix_pop(1);
        }
    }
}

void effect_cleanup()
{
    for(int iDx = 0; iDx < (sizeof(effectPool) / sizeof(effectPool[0])); iDx++)
    {
        free_uncached(effectPool[iDx].modelMatFP);
    }
}

/*==============================
    minigame_init
    The minigame initialization function
==============================*/

void minigame_init()
{
    // keep a const here of the player colours
    const color_t colours[] = {
    PLAYERCOLOR_1,
    PLAYERCOLOR_2,
    PLAYERCOLOR_3,
    PLAYERCOLOR_4,
    };

    // initialise gameplay variables
    lastCountdownNumber = COUNTDOWN_DELAY;
    countdownTimer = COUNTDOWN_DELAY;
    gameStarting = true;
    gameEnding = false;
    gameTimeRemaining = STARTING_GAME_TIME;

    // initialise the display, setting resolution, colour depth and AA
    joypad_buttons_t btn;
    btn = joypad_get_buttons_held(0);
    if(btn.r)
    {
        resolutionDisplayX = 640;
        resolutionDisplayY = 480;
        display_init(RESOLUTION_640x480, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    }
    else
    {  
        resolutionDisplayX = 320;
        resolutionDisplayY = 240;
        display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
    }
    depthBuffer = display_get_zbuf();

    // start tiny3d
    t3d_init((T3DInitParams){});

    // load a font to use for HUD text
    fontDebug = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
    rdpq_text_register_font(FONT_DEBUG, fontDebug);

    // load in the player billboard font
    fontBillboard = rdpq_font_load("rom:/squarewave.font64");
    rdpq_text_register_font(FONT_BILLBOARD, fontBillboard);
    for (size_t i = 0; i < MAXPLAYERS; i++)
    {
        rdpq_font_style(fontBillboard, i, &(rdpq_fontstyle_t){ .color = colours[i] });
    }

    // create the viewport
    viewport = t3d_viewport_create();

    // create a transformation matrix for the map
    mapMatFP = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_from_srt_euler(mapMatFP, (float[3]){0.3f, 0.3f, 0.3f}, (float[3]){0, 0, 0}, (float[3]){0,0,-10});

    // set camera position and target vectors
    camPos = (T3DVec3){{0, 215.0f, 65.0f}};
    camTarget = (T3DVec3){{0, 0, 20}};

    // set up a vector for the directional light
    lightDirVec = (T3DVec3){{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

    // load a model from ROM for the map
    modelMap = t3d_model_load("rom:/larcenygame/map.t3dm");

    // create a command block/display list to optimise drawing the map
    rspq_block_begin();
        t3d_matrix_push(mapMatFP);
        rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
        t3d_model_draw(modelMap);
        t3d_matrix_pop(1);
    dplMap = rspq_block_end();

    // load a model from ROM for the stun weapon effect
    modelWallJumpEffect = t3d_model_load("rom:/larcenygame/dustEffect.t3dm");

    // load a model from ROM for the collision square
    modelCollision = t3d_model_load("rom:/larcenygame/collisionSquare.t3dm");

    // load sprites from ROM for the HUD
    spriteAButton = sprite_load("rom:/core/AButton.sprite");

    // clear the sync point
    syncPoint = 0;

    // load sounds
    wav64_open(&sfx_start, "rom:/core/Start.wav64");
    wav64_open(&sfx_countdown, "rom:/core/Countdown.wav64");
    wav64_open(&sfx_winner, "rom:/core/Winner.wav64");
    
    wav64_open(&sfx_objectiveCompleted, "rom:/core/Countdown.wav64");
    wav64_open(&sfx_guardStunAbility, "rom:/core/Countdown.wav64");
    wav64_open(&sfx_thiefJumpAbility, "rom:/core/Countdown.wav64");
    wav64_open(&sfx_thiefCaught, "rom:/core/Countdown.wav64");

    xm64player_open(&xm_music, "rom:/snake3d/bottled_bubbles.xm64");
    
    mixer_ch_set_vol(31, 0.5f, 0.5f);

    // ensure AI init is done before player_init, otherwise no AI to be assigned to
    ai_init(players, MAXPLAYERS, objectives, sizeof(objectives)/sizeof(objectives[0]), 
            collisionObjects, sizeof(collisionObjects)/sizeof(collisionObjects[0]));

    // load players
    for(int i = 0; i < MAXPLAYERS; i++)
    {
        player_init(i);
    }

    // set up objectives
    objective_init();

    // initialise and setup collisions
    collision_init();

    // initialise effect models
    effect_init();

    return;
}

/*==============================
    minigame_fixedloop
    Code that is called every loop, at a fixed delta time.
    Use this function for stuff where a fixed delta time is 
    important, like physics.
    @param  The fixed delta time for this tick
==============================*/

void minigame_fixedloop(float deltatime)
{
    // update the player entities
    for(int i = 0; i < MAXPLAYERS; i++)
    {
        player_fixedloop(deltatime, i);
    }

    HUD_Update(deltatime);

    effect_update(deltatime);

    // process pre-game countdown timer
    if(gameStarting)
    {
        countdownTimer -= deltatime;

        if(countdownTimer < lastCountdownNumber)
        {
            lastCountdownNumber = countdownTimer;
            wav64_play(&sfx_countdown, 31);
        }

        if(countdownTimer < 1.0f)
        {
            wav64_play(&sfx_start, 31);
            gameStarting = false;

            // TODO: start playing music
            xm64player_play(&xm_music, 0);
        }
    }

    // if there's any value in the debounce, make sure to address it
    if(gamePauseDebounce >= 0.0f)
    {
        gamePauseDebounce -= deltatime;
        if(gamePauseDebounce < 0.0f) gamePauseDebounce = 0.0f;
    }

    if(gameEnding)
    {
        countdownTimer -= deltatime;

        if(countdownTimer < 1.0f)
        {
            minigame_end();
        }
    }
    return;
}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/

void minigame_loop(float deltatime)
{

    // update the player entities
    for(int i = 0; i < MAXPLAYERS; i++)
    {
        player_loop(deltatime, i);
    }

    // run objective updates
    objective_update(deltatime);

    // Check for victory conditions
    if(!gameStarting && !gameEnding && !gamePaused)
    {
        if(gameTimeRemaining < 0) end_game(teamGuard);
        // start this bool as true and set it false as soon as you get a single active objective
        bool tempGameEndFlag = true;
        for(int iDx = 0; iDx < sizeof(objectives) / sizeof(objectives[0]); iDx++)
        {
            if(objectives[iDx].isActive == true)
            {
                tempGameEndFlag = false;
                break;
            }
        }
        // if bool isn't set to false, then all objectives collected, thieves win
        if(tempGameEndFlag) end_game(teamThief);
        // now check for if all thieves have been caught
        tempGameEndFlag = true;
        for(int iDx = 0; iDx < MAXPLAYERS; iDx++)
        {
            if(players[iDx].isActive == true && players[iDx].playerTeam == teamThief)
            {
                tempGameEndFlag = false;
                break;
            }
        }
        // if bool isn't set to false, then all thieves have been caught, guards win
        if(tempGameEndFlag) end_game(teamGuard);
    }

    // set up the ambient light colour and the directional light colour
    uint8_t colorAmbient[4] = {0xAA, 0xAA, 0xAA, 0xFF};
    uint8_t colorDir[4] = {0xFF, 0xAA, 0xAA, 0xFF};

    // set the projection matrix for the viewport
    float aspectRatio = (float)viewport.size[0] / ((float)viewport.size[1]);//*2);
    t3d_viewport_set_perspective(&viewport, T3D_DEG_TO_RAD(90.0f), aspectRatio, 20.0f, 260.0f);
    t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

    // Draw our 3D frame
    // grab the colour and depth buffers and attach
    // the rdp queue in order to dispatch commands to the RDP
    rdpq_attach(display_get(), depthBuffer);

    // tell tiny3d it's time to start a new frame
    t3d_frame_start();
    // attach the desired viewport
    t3d_viewport_attach(&viewport);

    // clear the colour and depth buffers
    t3d_screen_clear_color(RGBA32(224, 180, 96, 0xFF));
    t3d_screen_clear_depth();

    // set the lighting details
    t3d_light_set_ambient(colorAmbient);
    t3d_light_set_directional(0, colorDir, &lightDirVec);
    t3d_light_set_count(1);

    // draw collision squares
    collision_draw();

    // draw the player entities
    for(int i = 0; i < MAXPLAYERS; i++)
    {
        player_draw(i);
    }

    // run the displaylist containing the map draw routine
    rspq_block_run(dplMap);

    // draw the objectives
    objective_draw();

    // draw the effects
    effect_draw();

    // set a sync point
    syncPoint = rspq_syncpoint_new();

    // draws RDP text on top of the scene showing debug info
    debugInfoDraw(deltatime);

    // draws the main game HUD
    HUD_draw();

    // make sure the RDP is sync'd
    rdpq_sync_tile();
    rdpq_sync_pipe(); // Hardware crashes otherwise

    // game starting countdown text draw
    if(gameStarting)
    {
        rdpq_textparms_t textparms = { .align = ALIGN_CENTER, .width = resolutionDisplayX, .disable_aa_fix = true, };
        rdpq_sync_tile(); rdpq_sync_pipe(); // make sure the RDP is sync'd Hardware crashes otherwise
        rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 0, resolutionDisplayY / 2, "Starting in %i...", (int)countdownTimer);
    }

    if(gamePaused)
    {
        rdpq_textparms_t textparms = { .align = ALIGN_CENTER, .width = resolutionDisplayX, .disable_aa_fix = true, };
        rdpq_sync_tile(); rdpq_sync_pipe(); // make sure the RDP is sync'd Hardware crashes otherwise
        rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 0, resolutionDisplayY / 2, "Game Paused \n Press L to Quit");
    }

    if(gameEnding)
    {
        rdpq_textparms_t textparms = { .align = ALIGN_CENTER, .width = resolutionDisplayX, .disable_aa_fix = true, };
        rdpq_sync_tile(); rdpq_sync_pipe(); // make sure the RDP is sync'd Hardware crashes otherwise
        if(winningTeam == teamThief)
        {
            rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 0, resolutionDisplayY / 2, "Thieves Win!");
        }
        else
        {
            rdpq_text_printf(&textparms, FONT_BUILTIN_DEBUG_MONO, 0, resolutionDisplayY / 2, "Guards Win!");
        }
    }
    
    // make sure the RDP is sync'd
    rdpq_sync_tile();
    rdpq_sync_pipe(); // Hardware crashes otherwise
    
    // detach the queue to flip the buffers and show it on screen
    rdpq_detach_show();

    return;
}


/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/

void minigame_cleanup()
{
    // cleanup players
    for(int i = 0; i < MAXPLAYERS; i++)
    {
        player_cleanup(i);
    }

    // destroy collisions
    collision_cleanup();

    // clenaup objectives
    objective_cleanup();

    // cleanup effects
    effect_cleanup();

    wav64_close(&sfx_start);
    wav64_close(&sfx_countdown);
    wav64_close(&sfx_winner);
    wav64_close(&sfx_objectiveCompleted);
    wav64_close(&sfx_guardStunAbility);
    wav64_close(&sfx_thiefJumpAbility);
    wav64_close(&sfx_thiefCaught);
    
    xm64player_close(&xm_music);

    t3d_model_free(modelWallJumpEffect);

    t3d_model_free(modelCollision);

    rspq_block_free(dplMap);

    t3d_model_free(modelMap);

    free_uncached(mapMatFP);
    
    // make sure to free the allocated sprites
    free(spriteAButton);

    rdpq_text_unregister_font(FONT_BILLBOARD);
    rdpq_font_free(fontBillboard);

    rdpq_text_unregister_font(FONT_DEBUG);
    rdpq_font_free(fontDebug);

    t3d_destroy();
    display_close();
    return;
}