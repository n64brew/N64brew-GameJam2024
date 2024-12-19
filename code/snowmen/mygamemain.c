#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

 #include "collision.h"
 #include "camera.h"
 #include "actor.h"
 #include "player.h"
 #include "crate.h"
 #include "pickup.h"
 #include "snowman.h"
 #include "DecorationSpawner.h"
 #include "AStar.h"


#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>



const MinigameDef minigame_def = {
    .gamename = "May the Best Snowman Win",
    .developername = "Razz",
    .description = "Gather snowballs and decorations \nto make the best snowman!",
    .instructions = "Press A to pickup/drop, press B to attack!"
};

#define FONT_TEXT           1
#define FONT_BILLBOARD      2
#define TEXT_COLOR          0x6CBB3CFF
#define TEXT_OUTLINE        0x30521AFF

#define TESTING             false

rspq_syncpoint_t syncPoint;

T3DViewport viewports[4];
T3DViewport viewportFullScreen;
surface_t *depthBuffer;

rdpq_font_t *font;
rdpq_font_t *fontBillboard;

//T3DVec3 camPos;
//T3DVec3 damPos;
//T3DVec3 camTarget;
T3DVec3 lightDirVec;

Camera camera1;
Camera camera2;
Camera camera3;
Camera camera4;
//Actor playerActor1;
//PlayerStruct playerStruct1;
PlayerStruct players[4];

PickupStruct snowballs[3];
//PickupStruct decorations[3];
SnowmanStruct snowmen[4];

T3DMat4FP* treeMatFP;
rspq_block_t *dplTree;
T3DModel *treeModel;


wav64_t sfx_start;
wav64_t sfx_countdown;
wav64_t sfx_stop;
wav64_t sfx_winner;
xm64player_t music;

bool fullScreen;

float dist;

Actor* AllActors[14];//13

Actor EnvActor;

float GameTimer;
float StartTimer;

float prevTime;
float prevStartTime;

int seed;
int numAPresses;

int pizza;

bool PlayerControl;
bool GameEnd;
bool TitleScreen;

float EndDelay;
float WinnerDelay;

int numWinners;

int points[4]; 
bool winners[4];

bool pause;

//bool testing;

SpawnLocation spawnLocations[6];

DecorationSpawnerStruct spawners[6];

T3DVec3 DecorationSpawnerLocations[6];

NodeDynamicArray AllNodes;

//NodeDynamicArray testpath;

node snowman1Node;
node snowman2Node;
node snowman3Node;
node snowman4Node;
/*-101.684120 0.000000 -141.909805 snowman1
-31.307348 0.000000 -140.445984 snowman2
35.772903 0.000000 -139.050888 snowman3
106.547562 0.000000 -137.578674 snowman4*/

node C0Node;
node C1Node;
/*0.135193 0.000000 -93.605042 C0 -> C1,snowmen
0.892233 0.000000 -57.207928 C1*/// -> C0,L0,R0

node L0Node;
node L1Node;
node L2Node;
node L3Node;
node L4Node;
node L5Node;//-166.653915f, 0.000000f, 82.444191f ->L4,D3
node L6Node;
/*-111.639977 0.000000 -56.476646 L0 ->l1,C1
-109.684357 0.000000 12.494196 L1 ->l2,L3,L0
-162.150879 0.000000 5.277177 L2 ->L1
-48.901482 0.000000 13.950903 L3 ->L1,L4
-49.950420 0.000000 64.379837 L4*/ // -> L3,C2, L5

node R0Node;
node R1Node;
node R2Node;
node R3Node;
node R4Node;//169.024796f, 0.000000f, 78.730408f ->R2, D6
node R5Node;//58.913193f, 0.000000f, 79.480339f ->C2, D5
/*110.508446 0.000000 -51.856239 R0 ->R1,C1
109.186836 0.000000 11.676569 R1 ->R0,R2,R3
168.890137 0.000000 -2.212346 R2 ->R1, R4
52.365280 0.000000 14.744545 R3*/ // -> R1

node C2Node;
/*-1.835996 0.000000 72.676994 C2*/ // -> L4, R5

node decospawn1Node;//-180.469894f, 0.000000f, -24.916225f -> L2
node decospawn2Node;//180.107773f, 0.000000f, -21.741064f -> R2
node decospawn3Node;//-150.182495f, 0.000000f, 147.382156f -> D4, L5
node decospawn4Node;//-57.233727f, 0.000000f, 149.315521f -> D3
node decospawn5Node;//64.739517f, 0.000000f, 151.852951f -> R5
node decospawn6Node;//170.096863f, 0.000000f, 156.108505f -> R4


void BadNoGoodNodeCreation()//You're not allowed to make fun of me for doing this.
{
    NodeDA_Create(&AllNodes);
    snowman1Node = (node) {.location = (T3DVec3) {{-101.684120f, 0.000000f, -141.909805f}}, .id = 91};
    NodeDA_Add(&AllNodes, &snowman1Node);
    NodeDA_Add(&snowman1Node.neighbors, &snowman2Node);
    NodeDA_Add(&snowman1Node.neighbors, &snowman3Node);
    NodeDA_Add(&snowman1Node.neighbors, &snowman4Node);
    NodeDA_Add(&snowman1Node.neighbors, &C0Node);

    snowman2Node = (node) {.location = (T3DVec3) {{-31.307348f, 0.000000f, -140.445984f}}, .id = 92};
    NodeDA_Add(&AllNodes, &snowman2Node);
    NodeDA_Add(&snowman2Node.neighbors, &snowman1Node);
    NodeDA_Add(&snowman2Node.neighbors, &snowman3Node);
    NodeDA_Add(&snowman2Node.neighbors, &snowman4Node);
    NodeDA_Add(&snowman2Node.neighbors, &C0Node);

    snowman3Node = (node) {.location = (T3DVec3) {{35.772903f, 0.000000f, -139.050888f}}, .id = 93};
    NodeDA_Add(&AllNodes, &snowman3Node);
    NodeDA_Add(&snowman3Node.neighbors, &snowman1Node);
    NodeDA_Add(&snowman3Node.neighbors, &snowman2Node);
    NodeDA_Add(&snowman3Node.neighbors, &snowman4Node);
    NodeDA_Add(&snowman3Node.neighbors, &C0Node);

    snowman4Node = (node) {.location = (T3DVec3) {{106.547562f, 0.000000f, -137.578674f}}, .id = 94};
    NodeDA_Add(&AllNodes, &snowman4Node);
    NodeDA_Add(&snowman4Node.neighbors, &snowman1Node);
    NodeDA_Add(&snowman4Node.neighbors, &snowman2Node);
    NodeDA_Add(&snowman4Node.neighbors, &snowman3Node);
    NodeDA_Add(&snowman4Node.neighbors, &C0Node);


    C0Node = (node) {.location = (T3DVec3) {{0.135193f, 0.000000f, -93.605042f}}, .id = 00};
    NodeDA_Add(&AllNodes, &C0Node);
    NodeDA_Add(&C0Node.neighbors, &C1Node);
    NodeDA_Add(&C0Node.neighbors, &snowman1Node);
    NodeDA_Add(&C0Node.neighbors, &snowman2Node);
    NodeDA_Add(&C0Node.neighbors, &snowman3Node);
    NodeDA_Add(&C0Node.neighbors, &snowman4Node);

    C1Node = (node) {.location = (T3DVec3) {{0.892233f, 0.000000f, -57.207928f}}, .id = 01};
    NodeDA_Add(&AllNodes, &C1Node);
    NodeDA_Add(&C1Node.neighbors, &C0Node);
    NodeDA_Add(&C1Node.neighbors, &L0Node);
    NodeDA_Add(&C1Node.neighbors, &R0Node);

    L0Node = (node) {.location = (T3DVec3) {{-111.639977f, 0.000000f, -56.476646f}}, .id = 10};
    NodeDA_Add(&AllNodes, &L0Node);
    NodeDA_Add(&L0Node.neighbors, &C1Node);
    NodeDA_Add(&L0Node.neighbors, &L1Node);

    L1Node = (node) {.location = (T3DVec3) {{-109.684357f, 0.000000f, 12.494196f}}, .id = 11};
    NodeDA_Add(&AllNodes, &L1Node);
    NodeDA_Add(&L1Node.neighbors, &L2Node);
    NodeDA_Add(&L1Node.neighbors, &L3Node);
    NodeDA_Add(&L1Node.neighbors, &L0Node);

    L2Node = (node) {.location = (T3DVec3) {{-162.150879, 0.000000f, 5.277177f}}, .id = 12};
    NodeDA_Add(&AllNodes, &L2Node);
    NodeDA_Add(&L2Node.neighbors, &L1Node);
    NodeDA_Add(&L2Node.neighbors, &decospawn1Node);

    L3Node = (node) {.location = (T3DVec3) {{-48.901482f, 0.000000f, 13.950903f}}, .id = 13};
    NodeDA_Add(&AllNodes, &L3Node);
    NodeDA_Add(&L3Node.neighbors, &L1Node);
    NodeDA_Add(&L3Node.neighbors, &L4Node);

    L4Node = (node) {.location = (T3DVec3) {{-49.950420f, 0.000000f, 64.379837f}}, .id = 14};
    NodeDA_Add(&AllNodes, &L4Node);
    NodeDA_Add(&L4Node.neighbors, &L3Node);
    NodeDA_Add(&L4Node.neighbors, &L5Node);
    NodeDA_Add(&L4Node.neighbors, &C2Node);
    NodeDA_Add(&L4Node.neighbors, &L6Node);

    R0Node = (node) {.location = (T3DVec3) {{110.508446f, 0.000000f, -51.856239f}}, .id = 20};
    NodeDA_Add(&AllNodes, &R0Node);
    NodeDA_Add(&R0Node.neighbors, &R1Node);
    NodeDA_Add(&R0Node.neighbors, &C1Node);

    R1Node = (node) {.location = (T3DVec3) {{109.186836f, 0.000000f, 11.676569f}}, .id = 21};
    NodeDA_Add(&AllNodes, &R1Node);
    NodeDA_Add(&R1Node.neighbors, &R0Node);
    NodeDA_Add(&R1Node.neighbors, &R2Node);
    NodeDA_Add(&R1Node.neighbors, &R3Node);

    R2Node = (node) {.location = (T3DVec3) {{168.890137f, 0.000000f, -2.212346f}}, .id = 22};
    NodeDA_Add(&AllNodes, &R2Node);
    NodeDA_Add(&R2Node.neighbors, &R1Node);
    NodeDA_Add(&R2Node.neighbors, &R4Node);
    NodeDA_Add(&R2Node.neighbors, &decospawn2Node);

    R3Node = (node) {.location = (T3DVec3) {{52.365280f, 0.000000f, 14.744545f}}, .id = 23};
    NodeDA_Add(&AllNodes, &R3Node);
    NodeDA_Add(&R3Node.neighbors, &R1Node);

    C2Node = (node) {.location = (T3DVec3) {{-1.835996f, 0.000000f, 72.676994f}}, .id = 02};
    NodeDA_Add(&AllNodes, &C2Node);
    NodeDA_Add(&C2Node.neighbors, &L4Node);
    NodeDA_Add(&C2Node.neighbors, &R5Node);


    L5Node = (node) {.location = (T3DVec3) {{-166.653915f, 0.000000f, 82.444191f}}, .id = 15};
    NodeDA_Add(&AllNodes, &L5Node);
    NodeDA_Add(&L5Node.neighbors, &L4Node);
    NodeDA_Add(&L5Node.neighbors, &decospawn3Node);
    NodeDA_Add(&L5Node.neighbors, &L6Node);

    R4Node = (node) {.location = (T3DVec3) {{169.024796f, 0.000000f, 78.730408f}}, .id = 24};
    NodeDA_Add(&AllNodes, &R4Node);
    NodeDA_Add(&R4Node.neighbors, &R2Node);
    NodeDA_Add(&R4Node.neighbors, &decospawn6Node);

    R5Node = (node) {.location = (T3DVec3) {{58.913193f, 0.000000f, 79.480339f}}, .id = 25};
    NodeDA_Add(&AllNodes, &R5Node);
    NodeDA_Add(&R5Node.neighbors, &C2Node);
    NodeDA_Add(&R5Node.neighbors, &decospawn5Node);

    L6Node = (node) {.location = (T3DVec3) {{-111.5f, 0.000000f, 106.9f}}, .id = 16};
    NodeDA_Add(&AllNodes, &L6Node);
    NodeDA_Add(&L6Node.neighbors, &L4Node);
    NodeDA_Add(&L6Node.neighbors, &decospawn3Node);
    NodeDA_Add(&L6Node.neighbors, &L5Node);
    NodeDA_Add(&L6Node.neighbors, &decospawn4Node);

    


    decospawn1Node = (node) {.location = (T3DVec3) {{-180.469894f, 0.000000f, -24.916225f}}, .id = 81};
    NodeDA_Add(&AllNodes, &decospawn1Node);
    NodeDA_Add(&decospawn1Node.neighbors, &L2Node);

    decospawn2Node = (node) {.location = (T3DVec3) {{180.107773f, 0.000000f, -21.741064f}}, .id = 82};
    NodeDA_Add(&AllNodes, &decospawn2Node);
    NodeDA_Add(&decospawn2Node.neighbors, &R2Node);

    decospawn3Node = (node) {.location = (T3DVec3) {{-150.182495f, 0.000000f, 147.382156f}}, .id = 83};
    NodeDA_Add(&AllNodes, &decospawn3Node);
    NodeDA_Add(&decospawn3Node.neighbors, &decospawn4Node);
    NodeDA_Add(&decospawn3Node.neighbors, &L5Node);
    NodeDA_Add(&decospawn3Node.neighbors, &L6Node);

    decospawn4Node = (node) {.location = (T3DVec3) {{-57.233727f, 0.000000f, 149.315521f}}, .id = 84};
    NodeDA_Add(&AllNodes, &decospawn4Node);
    NodeDA_Add(&decospawn4Node.neighbors, &decospawn3Node);
    NodeDA_Add(&decospawn4Node.neighbors, &L6Node);

    decospawn5Node = (node) {.location = (T3DVec3) {{64.739517f, 0.000000f, 151.852951f}}, .id = 85};
    NodeDA_Add(&AllNodes, &decospawn5Node);
    NodeDA_Add(&decospawn5Node.neighbors, &R5Node);

    decospawn6Node = (node) {.location = (T3DVec3) {{170.096863f, 0.000000f, 156.108505f}}, .id = 86};
    NodeDA_Add(&AllNodes, &decospawn6Node);
    NodeDA_Add(&decospawn6Node.neighbors, &R4Node);


    debugf("length: %d\n", AllNodes.length);



    //start snoman1 dest l2
    //NodeDynamicArray testpath;
    //AStarRun(&decospawn3Node, &snowman3Node, &testpath);

    //NodeDA_Add(&testpath, &decospawn3Node);

    //players[1].ai_path_index = testpath.length - 1;

    /*for (int i = 0; i < testpath.length; i++)
    {
        debugf("Final id: %d\n", testpath.nodeArray[i]->id);
    }*/

}


void AITestWalk(PlayerStruct* player, NodeDynamicArray* path, float* x_out, float* y_out)
{
    player->isAI = true;
    //debugf("current node: %d\n",player->ai_path_index);
    if (player->ai_path_index < 0)
    {
        *x_out = 0.f;
        *y_out = 0.f;
        return;
    }
    T3DVec3 currentDest = path->nodeArray[player->ai_path_index]->location;
    T3DVec3 currentPosition = player->PlayerActor.Position;

    if (t3d_vec3_distance2(&currentDest, &currentPosition) <= 10.f)
    {
        player->ai_path_index--;
        if (player->ai_path_index < 0)
        {
            //debugf("at destination!\n"); 
            *x_out = 0.f;
            *y_out = 0.f;
            return;
        }
    }

    T3DVec3 NewDirection;// = {{-ThisPlayer->PlayerActor.CurrentVelocity.v[0], 0.f, ThisPlayer->PlayerActor.CurrentVelocity.v[2]}};
    t3d_vec3_diff(&NewDirection, &currentDest, &currentPosition);
    fast_vec3_norm(&NewDirection);
    //debugf("NewDirection=%f, %f, ", NewDirection.v[0], NewDirection.v[2]);

    *x_out = NewDirection.v[2];
    *y_out = NewDirection.v[0];

    //debugf("x and y (should be the same)=%f, %f\n", *x_out, *y_out);
    //debugf("player pos = %f %f %f\n", player->PlayerActor.Position.v[0], player->PlayerActor.Position.v[1],  player->PlayerActor.Position.v[2]);
    //debugf("dest pos = %f %f %f\n", path->nodeArray[path->length - 1]->location.v[0], path->nodeArray[path->length - 1]->location.v[1],  path->nodeArray[path->length - 1]->location.v[2]);
    //float Angle = atan2f(NewDirection.v[0], NewDirection.v[2]);
    //debugf("Finally, the angle: %f\n", Angle);
}


node* GetNodeForGoalPickup(PickupStruct* GoalPickup, enum EAIGoalType type)
{
    if (type == EAIGT_SpawnerPickup && GoalPickup->pickupType == EPUT_Decoration)
    {
        switch (GoalPickup->decorationType)
        {
            case 1:
                return &decospawn1Node;
            case 2:
                return &decospawn2Node;
            case 3:
                return &decospawn3Node;
            case 4:
                return &decospawn4Node;
            case 5:
                return &decospawn5Node;
            case 6:
                return &decospawn6Node;
            default:
                return NULL;
        }
    }
    else// if (type == EAIGT_PickupIdle)
    {
        //get closest node
        return NodeDA_GetClosestNode(&AllNodes, GoalPickup->pickupActor.Position);
    }
    /*else
    {
        return NULL;//shouldn't get here
    }*/
}

node* GetNodePlayerSnowman(int id)
{
    switch (id)
    {
        case 0:
            return &snowman1Node;
        case 1:
            return &snowman2Node;
        case 2:
            return &snowman3Node;
        case 3:
            return &snowman4Node;
        default:
            return NULL;//shouldn't get here
    }
}

PickupStruct* GetRandomSnowball(PlayerStruct* playerStruct, int seed)
{
    //find available snowballs, pick one as goal
    int q = 0;
    while(q < 3)
    {
        int randNum = (int) fabs(seed % 3);
        if(snowballs[randNum].pickupState == EPUS_Idle)
        {
            playerStruct->AIGoalType = EAIGT_PickupIdle;//all snowballs are pickupidle
            return &snowballs[randNum];//playerStruct->AIGoalPickup
            break;
        }
        seed++;
        q++;
    }
    
    return NULL;

}

PickupStruct* GetRandomDecoration(PlayerStruct* playerStruct, int seed)
{
    //try to find decos instead
    int q = 0;
    while(q < 6)
    {//(snowmanStruct->decorations & (1 << (pickupStruct->decorationType - 1)))
        int randNum = (int) fabs(seed % 6);
        int typeToCheck = 0 | (1 << (spawners[randNum].decorations[0].decorationType - 1));
        //if (!(snowmen[playerStruct->playerId].decorations & (1 << )))//Don't already have deco?
        if (!(snowmen[playerStruct->playerId].decorations & (1 << (spawners[randNum].decorations[0].decorationType - 1))))
        {
            if(spawners[randNum].decorations[0].pickupState != EPUS_PickedUp)
            {
                if (spawners[randNum].decorations[0].pickupState == EPUS_Inactive)
                {
                    playerStruct->AIGoalType = EAIGT_SpawnerPickup;
                }
                else
                {
                    playerStruct->AIGoalType = EAIGT_PickupIdle;
                }
                return &spawners[randNum].decorations[0];
                break;
            }
            else if (spawners[randNum].decorations[1].pickupState != EPUS_PickedUp)
            {
                if (spawners[randNum].decorations[1].pickupState == EPUS_Inactive)
                {
                    playerStruct->AIGoalType = EAIGT_SpawnerPickup;
                }
                else
                {
                    playerStruct->AIGoalType = EAIGT_PickupIdle;
                }
                return &spawners[randNum].decorations[1];
                break;
            }
        }
        else
        {
            debugf("gosh darn, already got a %d\n", (spawners[randNum].decorations[0].decorationType - 1));
        }

        seed++;
        q++;
    }
        //no available pickups at all! stay in idle state, maybe add a wait?
        return NULL;
}

bool AIShouldWasteTime(int seed)
{
    int diff = core_get_aidifficulty();
    if (diff == DIFF_EASY)
    {
        if (((int)fabs(seed) % 10)< 5)
        {
            seed += numAPresses;
            return true;
        }
    }
    else if(diff == DIFF_MEDIUM)
    {
        if (((int)fabs(seed) % 10) < 3)
        {
            seed += numAPresses;
            return true;
        }
    }
    else
    {
        if (((int)fabs(seed) % 10) < 1)
        {
            seed += numAPresses;
            return true;
        }
    }
    seed += numAPresses;
    return false;
}


void AIPlayerLoop(PlayerStruct* playerStruct, int seed, float deltaTime)
{
    //debugf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^_0\n");
    if (playerStruct->stunTimer > 0)
    {
        playerStruct->AIState = EPAIS_Idle;
    }
    //PickupStruct* AIGoalPickup = NULL;//should be member of playerstruct
    //bool isAIGoalPickup//don't like have them being seperate, if false then Goal must be snowman
    if (playerStruct->AIState == EPAIS_Idle)
    {
        //NodeDA_Free(&playerStruct->AIPath);
        //node **array = playerStruct->AIPath.nodeArray;
        free(playerStruct->AIPath.nodeArray);
        playerStruct->isDestGoalPickupDirectAI = false;
        //maybe a timer for waiting to make ai easier?
        //We want to walk towards a goal, choose which from goals not already achieved and are available on map (random)
            // Also check if we are holding something, if we are, go to snowman as goal
        if (playerStruct->heldPickup == NULL)
        {
            //does snowman not have any snowballs?
            if (AIShouldWasteTime(seed)) //should choose random location (waste time)?
            {
                //playerStruct->AIGoalPickup = AllNodes[seed % AllNodes.length];///
                playerStruct->AIGoalType = EAIGT_Random;// 
            }
            else if(snowmen[playerStruct->playerId].snowmanLevel == ESL_level0)
            {
                //find available snowballs, pick one as goal
                playerStruct->AIGoalPickup = GetRandomSnowball(playerStruct, seed);
                if (playerStruct->AIGoalPickup == NULL)
                {
                    //try to find decos instead
                    playerStruct->AIGoalPickup = GetRandomDecoration(playerStruct, seed);
                    if (playerStruct->AIGoalPickup == NULL)
                    {
                        //no available pickups at all! stay in idle state, maybe add a wait?
                        debugf("No available pickups, within if snowmen[playerStruct->playerId].snowmanLevel == ESL_level0\n");
                        return;
                    }
                }  
            }
            else if (snowmen[playerStruct->playerId].snowmanLevel == ESL_level3)
            {
                //find available decorations, pick one as goal
                    //try to find decos instead
                    playerStruct->AIGoalPickup = GetRandomDecoration(playerStruct, seed);
                    if (playerStruct->AIGoalPickup == NULL)
                    {
                        //no available pickups at all! stay in idle state, maybe add a wait?
                        debugf("No available pickups, within else if (snowmen[playerStruct->playerId].snowmanLevel == ESL_level3)\n");
                        return;
                    }
            }
            else
            {
                //choose either deco or snowball from random, pick as goal
                int randFlip = (int) fabs(seed % 2);
                seed++;
                switch(randFlip)
                {
                    debugf(">>>>>>>>>>>>>>>>>> a\n");
                    case 0://snowball
                        debugf(">>>>>>>>>>>>>>>>>> b\n");
                        debugf("        Before: %d\n", playerStruct->AIGoalType);
                        playerStruct->AIGoalPickup = GetRandomSnowball(playerStruct, seed);
                        debugf("        After: %d\n", playerStruct->AIGoalType);
                        if (playerStruct->AIGoalPickup == NULL)
                        {
                            debugf(">>>>>>>>>>>>>>>>>> c\n");
                            //try to find decos instead
                            debugf("        Before: %d\n", playerStruct->AIGoalType);
                            playerStruct->AIGoalPickup = GetRandomDecoration(playerStruct, seed);
                            debugf("        After: %d\n", playerStruct->AIGoalType);
                            if (playerStruct->AIGoalPickup == NULL)
                            {
                                //no available pickups at all! stay in idle state, maybe add a wait?
                                debugf("No available pickups, within randFlip 0\n");
                                return;
                            }
                        }
                        break;


                    case 1://deco
                        debugf(">>>>>>>>>>>>>>>>>> d\n");
                        debugf("        Before: %d\n", playerStruct->AIGoalType);
                        playerStruct->AIGoalPickup = GetRandomDecoration(playerStruct, seed);
                        debugf("        After: %d\n", playerStruct->AIGoalType);
                        if (playerStruct->AIGoalPickup == NULL)
                        {
                            debugf(">>>>>>>>>>>>>>>>>> e\n");
                            //try to find decos instead
                            debugf("        Before: %d\n", playerStruct->AIGoalType);
                            playerStruct->AIGoalPickup = GetRandomSnowball(playerStruct, seed);
                            debugf("        After: %d\n", playerStruct->AIGoalType);
                            if (playerStruct->AIGoalPickup == NULL)
                            {
                                //no available pickups at all! stay in idle state, maybe add a wait?
                                debugf("No available pickups, within randFlip 1\n");
                                return;
                            }
                        }
                        break;
                    default:
                        debugf("No available pickups, within randFlip SHOULDN'T HAPPEN\n");
                        return;
                }

                //playerStruct->AIGoalType set in getrandomfunctions
                //debugf("Hey, uh, are we getting here?\n");
            }
            //debugf("bonjour, tis null ideed\n");
        }
        else
        {
            //debugf("-------------------------------what are you? %d, %d\n", playerStruct->heldPickup->pickupType, playerStruct->heldPickup->decorationType);
            playerStruct->AIGoalType = EAIGT_Snowman;//snowman is goal
        }

        node* GoalNode;
        //now that we know our goal, get node related to that goal
        if (playerStruct->AIGoalType == EAIGT_PickupIdle || playerStruct->AIGoalType == EAIGT_SpawnerPickup)
        {
            GoalNode = GetNodeForGoalPickup(playerStruct->AIGoalPickup, playerStruct->AIGoalType);
            debugf("GetNodeForGoalPickup, aka it'a pickup\n");
        }
        else if (playerStruct->AIGoalType == EAIGT_Snowman)
        {
            GoalNode = GetNodePlayerSnowman(playerStruct->playerId);
            debugf("GetNodePlayerSnowman, aka it's a snowman.\n");
        }
        else if (playerStruct->AIGoalType == EAIGT_Random)
        {
            GoalNode = AllNodes.nodeArray[3+((int)fabs(seed) % (AllNodes.length-3))];///
            debugf("Random Node\n");
        }
        else
        {
            //it should never reach this point
            debugf("you shouldn't be seeing this\n");
            GoalNode = GetNodePlayerSnowman(playerStruct->playerId);
        }
        //And get closest node to self
        //debugf("AI player pos = %f %f %f\n", playerStruct->PlayerActor.Position.v[0], playerStruct->PlayerActor.Position.v[1],  playerStruct->PlayerActor.Position.v[2]);
        node* startNode = NodeDA_GetClosestNode(&AllNodes, playerStruct->PlayerActor.Position);

        //Run Astar with these two nodes, add the origin node to the path after it returns. Set ai_path index for player
        pizza++;
        debugf("            Player %d Running AStarRun, times: %d\n", playerStruct->playerId+1, pizza);
        debugf("            start node: %d\n", startNode->id);
        debugf("            End node: %d\n", GoalNode->id);
        debugf("            Goal type: %d\n", playerStruct->AIGoalType);
        debugf("            oh, and are we good? %d\n", playerStruct->AIGoalPickup != NULL);
        AStarRun(startNode, GoalNode, &playerStruct->AIPath);
        NodeDA_Add(&playerStruct->AIPath, startNode);
        playerStruct->ai_path_index = playerStruct->AIPath.length - 1;

    for (int i = 0; i < playerStruct->AIPath.length; i++)
    {
        debugf("Final id: %d\n", playerStruct->AIPath.nodeArray[i]->id);
    }

        playerStruct->AIState = EPAIS_Walking;
        playerStruct->AIStuckTimer = 2.f;
        playerStruct->AINoGoalTimer = 10.f;
        playerStruct->AI_InitialPosition = playerStruct->PlayerActor.Position;
        debugf("          Return, We're now walking\n");
        return;
    }
    else if (playerStruct->AIState == EPAIS_Walking)
    {
        playerStruct->AIStuckTimer -= deltaTime;
        playerStruct->AINoGoalTimer -= deltaTime;
        if (playerStruct->AINoGoalTimer <= 0)
        {
            //debugf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^_1\n");
            node* backupNode = NodeDA_GetClosestNode(&AllNodes, playerStruct->PlayerActor.Position);
            playerStruct->PlayerActor.Position = backupNode->location;
            playerStruct->AIState = EPAIS_Idle;
            if (playerStruct->heldPickup != NULL)
            {
                PlayerDrops(playerStruct->heldPickup, playerStruct);
            }
            debugf("return to idle from no goal\n");
            return;
        }
        if (playerStruct->AIStuckTimer <= 0)
        {
            //debugf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^_2\n");
            float dist = t3d_vec3_distance2(&playerStruct->PlayerActor.Position, &playerStruct->AI_InitialPosition);
            //debugf("dist is: %f\n", dist);
            if (dist < 1000.f)
            {
                //debugf("dist is super small!");
                node* backupNode = NodeDA_GetClosestNode(&AllNodes, playerStruct->PlayerActor.Position);
                playerStruct->PlayerActor.Position = backupNode->location;
                playerStruct->AIState = EPAIS_Idle;
                if (playerStruct->heldPickup != NULL)
                {
                    PlayerDrops(playerStruct->heldPickup, playerStruct);
                }
                debugf("return to idle from stuck removal\n");
                return;
            }
            else
            {
                playerStruct->AIStuckTimer = 2.f;
                playerStruct->AI_InitialPosition = playerStruct->PlayerActor.Position;
            }

        }

        //is goal still valid? if not, return to idle state, we'll find a new goal next tick
        if (playerStruct->AIGoalType == EAIGT_SpawnerPickup)
        {
            //debugf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^_3\n");
            if (playerStruct->AIGoalPickup->pickupState != EPUS_Inactive || playerStruct->AIGoalPickup->holdingPlayerStruct != NULL)
            {
                playerStruct->AIState = EPAIS_Idle;
                debugf("return to idle from Goal Not Valid Spawner\n");
                return;
            }
        }
        else if(playerStruct->AIGoalType == EAIGT_PickupIdle)
        {
            //debugf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^_4\n");
            if (playerStruct->AIGoalPickup != NULL
            && playerStruct->isDestGoalPickupDirectAI && t3d_vec3_distance2(&playerStruct->AIGoalPickup->pickupActor.Position, &playerStruct->PlayerActor.Position) >= 20000.f)
            {
                playerStruct->AIState = EPAIS_Idle;
                debugf("return to idle from Goal Not Valid Idle Pickup 1\n");
                return;
            }
            if (playerStruct->AIGoalPickup->pickupState == EPUS_Inactive || playerStruct->AIGoalPickup->pickupState == EPUS_PickedUp || playerStruct->AIGoalPickup->holdingPlayerStruct != NULL )
            {
                playerStruct->AIState = EPAIS_Idle;
                debugf("return to idle from Goal Not Valid Idle Pickup 2\n");
                return;
            }
        }
        else if (playerStruct->AIGoalType == EAIGT_Snowman)
        {
            if (!(playerStruct->heldPickup != NULL && playerStruct->heldPickup->holdingPlayerStruct == playerStruct))
            {
                playerStruct->AIState = EPAIS_Idle;
                debugf("return to idle from no longer holding pickup\n");
                return;
            }
        }
        else if (playerStruct->AIGoalType == EAIGT_Random)
        {
            //debugf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^_5\n");
            //Does anything need to be here?
        }
        //If still valid, Walk and check if we are in range of goal (currently part of AITestWalk)

        T3DVec3 currentDest;
        if (playerStruct->isDestGoalPickupDirectAI)
        {
            //debugf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^_6\n");
            currentDest = playerStruct->AIGoalPickup->pickupActor.Position;
        }
        else
        {
            //debugf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^_7\n");
            currentDest = playerStruct->AIPath.nodeArray[playerStruct->ai_path_index]->location;
        }
        T3DVec3 currentPosition = playerStruct->PlayerActor.Position;

            if (t3d_vec3_distance2(&currentDest, &currentPosition) <= 500.f)//Are we in range?
            {
                //debugf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^_8\n");
                playerStruct->ai_path_index--;
                if (playerStruct->ai_path_index < 0)
                {
                    //debugf("at destination!\n"); 
                    playerStruct->AI_x = 0.f;
                    playerStruct->AI_y = 0.f;

                    //Great, we have reached our goal.
                    //if goal is a spawner, try to pickup, if was successful set snowman as new goal and run AStar like before
                    if (playerStruct->AIGoalType == EAIGT_SpawnerPickup)
                    {
                        PlayerPicksUp(playerStruct->AIGoalPickup, playerStruct);
                        playerStruct->AIState = EPAIS_Idle;
                        debugf("return to idle from Reached Goal Spawner\n");
                        return;
                    }
                    else if (playerStruct->AIGoalType == EAIGT_PickupIdle)
                    {//if goal is a standing pickup, move towards it like a node.
                        if (playerStruct->isDestGoalPickupDirectAI == false)
                        {
                            currentDest = playerStruct->AIGoalPickup->pickupActor.Position;
                            playerStruct->isDestGoalPickupDirectAI = true;
                        }
                        else
                        {
                            PlayerPicksUp(playerStruct->AIGoalPickup, playerStruct);
                            playerStruct->AIState = EPAIS_Idle;
                            playerStruct->isDestGoalPickupDirectAI = false;
                            debugf("return to idle from Reached Goal Idle Pickup\n");
                            return;
                        }

                    }
                    else if (playerStruct->AIGoalType == EAIGT_Snowman)
                    {
                        int typeToAdd = 0 | (1 << (playerStruct->AIGoalPickup->decorationType - 1));

                        //if goal was snowman, check if snowman absorbed our pickup. If so, then return to idle, next tick we'll decide next goal.
                        if (playerStruct->AIGoalPickup->holdingPlayerStruct == NULL)
                        {
                            playerStruct->AIState = EPAIS_Idle;
                            debugf("return to idle from Snowman Absorbed pickup\n");
                            return;
                        }//if still holding pickup check if snowman already has it. If so, then drop it and return to idle.
                        else if ((snowmen[playerStruct->playerId].snowmanLevel == ESL_level3 && playerStruct->AIGoalPickup->pickupType == EPUT_Snowball))
                        {//snowman already has this type of deco or level is already at max
                            PlayerDrops(playerStruct->AIGoalPickup, playerStruct);
                            playerStruct->AIState = EPAIS_Idle;
                            debugf("return to idle from Snowman full of snowballs\n");
                            return;
                        }//
                        else if ((playerStruct->AIGoalPickup->pickupType == EPUT_Decoration && (snowmen[playerStruct->playerId].decorations & (1 << (playerStruct->AIGoalPickup->decorationType - 1)))))
                        {//snowman already has this type of deco or level is already at max
                            PlayerDrops(playerStruct->AIGoalPickup, playerStruct);
                            playerStruct->AIState = EPAIS_Idle;
                            debugf("return to idle from Snowman already has deco, so drop it\n");
                            return;
                        }
                        else if(snowmen[playerStruct->playerId].snowmanLevel == ESL_level0 && playerStruct->AIGoalPickup->pickupType == EPUT_Decoration)
                        {
                            PlayerDrops(playerStruct->AIGoalPickup, playerStruct);
                            playerStruct->AIState = EPAIS_Idle;
                            debugf("level0, can't add deco yet...\n");
                            return;
                        }
                        else//if snowman doesn't have it, then this may be an error. Could just force give to snowman since already at proper node.  
                        {
                            debugf("had to force add pickup to snowman...\n");
                            bool isSucc = SnowmanAttemptAdd(&snowmen[playerStruct->playerId], playerStruct, playerStruct->AIGoalPickup, &GameEnd);
                            if (!isSucc)
                            {
                                debugf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Failed to Add to Snowman\n");
                            }
                            if (GameEnd)
                            {
                                xm64player_stop(&music);
                                wav64_play(&sfx_stop, 31);
                            }
                            playerStruct->AIState = EPAIS_Idle;
                            debugf("return to idle from Force add snowman\n");
                            return;
                        }
                    }
                    else if (playerStruct->AIGoalType == EAIGT_Random)
                    {
                            playerStruct->AIState = EPAIS_Idle;
                            debugf("return to idle from reached random node\n");
                            return;
                    }
        /*int typeToCheck = 0 | (1 << (spawners[randNum].decorations[0].decorationType - 1));
        if (!(snowmen[playerStruct->playerId].decorations & (1 << typeToCheck)))//Don't already have deco?*/
                }
            }
        //debugf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^_9\n");
        //Not yet in range, keep walking:
        T3DVec3 NewDirection;
        t3d_vec3_diff(&NewDirection, &currentDest, &currentPosition);
        fast_vec3_norm(&NewDirection);

        playerStruct->AI_x = NewDirection.v[2];
        playerStruct->AI_y = NewDirection.v[0];
        //Must add to grounded movement!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    }
}












void SpawnSnowball()
{
    PickupStruct* tempPickup = NULL;
    for (int i = 0; i < 3; i++)
    {
        if (snowballs[i].pickupState == EPUS_Inactive) tempPickup = &snowballs[i];
    }
    if (tempPickup == NULL)
    {
        //debugf("dang, no available snowballs\n");
        return;
    }
    for (int i = 0; i < 6; i++)
    {
        SpawnLocation* newLocation = GetRandomLocation(spawnLocations, seed + i);
        //debugf("%f, %f, %f\n", newLocation->Pos.v[0], newLocation->Pos.v[1], newLocation->Pos.v[2]);
        //debugf("num is %d\n", (int) fabs((seed + 1) % 6));
        if (!newLocation->isOccupied)//!!!!!!!!!!!!!!MAKE LOOP THROUGH ALL LOCATIONS!!!!!!!!!!!!!!!
        {
            //debugf("Hey, this place ain't occupied!\n");
            //debugf("Hey, this place ain't occupied!\n", newLocation->Pos);
            //set snowball location to here
            tempPickup->pickupActor.Position = newLocation->Pos;
            //begin spawn animation if we decide to make one
            //set state to idle
            PickupActivate(tempPickup, newLocation);
            newLocation->isOccupied = true;
            newLocation->pickupPtr = tempPickup;
            return;
        }
    }

    //debugf("dang, no available spots!\n");
}

void setSpawnerLocations()
{
    DecorationSpawnerLocations[0] = (T3DVec3) {{ -210.f, 0.f, -37.f}};
    DecorationSpawnerLocations[1] = (T3DVec3) {{ 210.f, 0.f, -37.f}};
    DecorationSpawnerLocations[2] = (T3DVec3) {{ -170.f, 0.f, 160.f}};
    DecorationSpawnerLocations[3] = (T3DVec3) {{ -48.f, 0.f, 173.f}};
    DecorationSpawnerLocations[4] = (T3DVec3) {{ 50.f, 0.f, 170.f}};
    DecorationSpawnerLocations[5] = (T3DVec3) {{ 170.f, 0.f, 155.f}};

}

void SetSnowballSpawnLocations(SpawnLocation locations[6])
{
    //hard code location values
    locations[0] = (SpawnLocation){
        .isOccupied = false,
        .pickupPtr = NULL,
        .Pos = R3Node.location
    };
    locations[1] = (SpawnLocation){
        .isOccupied = false,
        .pickupPtr = NULL,
        .Pos = (T3DVec3) {{-204.f, 0.f, 21.f}}
    };
    locations[2] = (SpawnLocation){
        .isOccupied = false,
        .pickupPtr = NULL,
        .Pos = (T3DVec3) {{188.f, 0.f, 57.4f}}
    };
    locations[3] = (SpawnLocation){
        .isOccupied = false,
        .pickupPtr = NULL,
        .Pos = R5Node.location
    };
    locations[4] = (SpawnLocation){
        .isOccupied = false,
        .pickupPtr = NULL,
        .Pos = L5Node.location
    };
    locations[5] = (SpawnLocation){
        .isOccupied = false,
        .pickupPtr = NULL,
        .Pos = L3Node.location
    };
}


void player_draw_billboard(PlayerStruct* playerStruct)
{
  rdpq_sync_pipe();// Hardware crashes otherwise
  rdpq_sync_tile(); // Hardware crashes otherwise

  T3DVec3 billboardPos = (T3DVec3){{
    playerStruct->PlayerActor.Position.v[0],
    playerStruct->PlayerActor.Position.v[1] + 15.f,
    playerStruct->PlayerActor.Position.v[2]
  }};

  T3DVec3 billboardScreenPos;
  t3d_viewport_calc_viewspace_pos(&viewportFullScreen, &billboardScreenPos, &billboardPos);

  int x = floorf(billboardScreenPos.v[0]);
  int y = floorf(billboardScreenPos.v[1]);

  rdpq_sync_pipe(); // Hardware crashes otherwise
  rdpq_sync_tile(); // Hardware crashes otherwise

    rdpq_text_printf(&(rdpq_textparms_t){ .style_id = playerStruct->playerId }, FONT_BILLBOARD, x-5, y-16, "P%d", playerStruct->playerId+1);
    //rdpq_text_printf(NULL, FONT_BILLBOARD, x-5, y-16, "P%d", playerStruct->playerId+1);

}

void draw_billboard_generic(T3DVec3* billboardPos, char* string, float yOffset)
{
  rdpq_sync_pipe(); // Hardware crashes otherwise
  rdpq_sync_tile(); // Hardware crashes otherwise

  /*T3DVec3 billboardPos = (T3DVec3){{
    playerStruct->PlayerActor.Position.v[0],
    playerStruct->PlayerActor.Position.v[1] + 15.f,
    playerStruct->PlayerActor.Position.v[2]
  }};*/

  T3DVec3 billboardScreenPos;
  t3d_viewport_calc_viewspace_pos(&viewportFullScreen, &billboardScreenPos, billboardPos);

  int x = floorf(billboardScreenPos.v[0]);
  int y = floorf(billboardScreenPos.v[1] + yOffset);

  rdpq_sync_pipe(); // Hardware crashes otherwise
  rdpq_sync_tile(); // Hardware crashes otherwise
    //char string1[32];
  //snprintf(string1, sizeof(string1), "%d potatoe", -1);

    rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, FONT_BILLBOARD, x-5, y-16, "%s", string);
    //rdpq_text_printf(NULL, FONT_BILLBOARD, x-5, y-16, "P%d", playerStruct->playerId+1);

}

void spawn_temp_billboard(T3DVec3* billboardPos, char* string, bool moveUp, float deltaTime)
{
    float temp = 0;
    if (moveUp)
    {
        temp = deltaTime*10;
    }
    draw_billboard_generic(billboardPos, string, temp);
}




/*==============================
    minigame_init
    The minigame initialization function    
==============================*/
void minigame_init()
{
    TitleScreen = true;
    numWinners = -1;
    syncPoint = 0;
    PlayerControl = false;
    GameTimer = 85.f;
    StartTimer = 3.f;
    prevTime = -1.f;
    EndDelay = 2.f;
    WinnerDelay = 1.f;

    seed = 0;
    numAPresses = 0;
    pizza = 0;

        BadNoGoodNodeCreation();


    

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS_DEDITHER);
    depthBuffer = display_get_zbuf();

    int sizeX = display_get_width();
    int sizeY = display_get_height();

    // Here we allocate multiple viewports to render to different parts of the screen
    // This isn't really any different to other examples, just that we have 3 of them now
    //viewports[0] = t3d_viewport_create();
    //viewports[1] = t3d_viewport_create();
    //viewports[2] = t3d_viewport_create();
    //viewports[3] = t3d_viewport_create();

    /*t3d_viewport_set_area(&viewports[0], 0,       0,       sizeX/2, sizeY/2);
    t3d_viewport_set_area(&viewports[1], sizeX/2, 0,       sizeX/2, sizeY/2);
    t3d_viewport_set_area(&viewports[2], 0,       sizeY/2, sizeX/2,   sizeY/2-2);
    t3d_viewport_set_area(&viewports[3], sizeX/2,       sizeY/2, sizeX/2,   sizeY/2-2);
*/
    fullScreen = true;
    t3d_init((T3DInitParams){});
    viewportFullScreen = t3d_viewport_create();

    font = rdpq_font_load("rom:/snowmen/m6x11plus.font64");
    rdpq_text_register_font(FONT_TEXT, font);
    rdpq_font_style(font, 0, &(rdpq_fontstyle_t){.color = color_from_packed32(TEXT_COLOR) });

    const color_t colors[] = {
        color_from_packed32(0xFF0000<<8),
        color_from_packed32(0x0000FF<<8),
        color_from_packed32(0xFFFF00<<8),
        color_from_packed32(0x00FF00<<8),
    };

    fontBillboard = rdpq_font_load("rom:/squarewave.font64");
    rdpq_text_register_font(FONT_BILLBOARD, fontBillboard);
    for (size_t i = 0; i < MAXPLAYERS; i++)
    {
        rdpq_font_style(fontBillboard, i, &(rdpq_fontstyle_t){ .color = colors[i] });
    }
    rdpq_font_style(fontBillboard, 4, &(rdpq_fontstyle_t){ .color = color_from_packed32(0xFFFFFF<<8) });

    

    //camPos = (T3DVec3){{0, 125.0f, 100.0f}};
    //camTarget = (T3DVec3){{0, 0, 40}};

    lightDirVec = (T3DVec3){{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);



    /*const char *path = "rom:/mygame/TEST_ENV_4.col";

    int size = 0;

    CollisionStruct* model = asset_load(path, &size);

    if(memcmp(model, "COL", 3) != 0) 
    {
        assertf(false, "Invalid collision file: %s", path);
    }
    else
    {
        assertf(model->magic[3] == 42,
        "Invalid T3D model version: %d != %d\n"
        "Please make a clean build of t3d and your project",
        42, model->magic[3]);

        debugf("Is indeed a collision file! Size: %d\n", size);
        debugf("%d \n", model->totalTriCount);//uint16_t
        debugf("%d \n", model->totalIndexCount);
        for(int i = 0; i < model->totalTriCount; i ++)
        {
            debugf("    i = %d \n", i);
            for(int j = 0; j < 3; j++)
            {
                debugf("%d \n", model->tris[i].verts[j].v[0]);
                debugf("%d \n", model->tris[i].verts[j].v[1]);
                debugf("%d \n", model->tris[i].verts[j].v[2]);
            }
        }
    }

    const char *pathBox = "rom:/mygame/box.col";

    size = 0;

    CollisionStruct* modelBox = asset_load(pathBox, &size);

    if(memcmp(modelBox, "COL", 3) != 0) 
    {
        assertf(false, "Invalid collision file: %s", pathBox);
    }
    else
    {
        assertf(modelBox->magic[3] == 42,
        "Invalid T3D modelBox version: %d != %d\n"
        "Please make a clean build of t3d and your project",
        42, modelBox->magic[3]);

        debugf("Is indeed a collision file! Size: %d\n", size);
        debugf("%d \n", modelBox->totalTriCount);//uint16_t
        debugf("%d \n", modelBox->totalIndexCount);
        for(int i = 0; i < modelBox->totalTriCount; i ++)
        {
            debugf("    i = %d \n", i);
            for(int j = 0; j < 3; j++)
            {
                debugf("%d \n", modelBox->tris[i].verts[j].v[0]);
                debugf("%d \n", modelBox->tris[i].verts[j].v[1]);
                debugf("%d \n", modelBox->tris[i].verts[j].v[2]);
            }
        }
        
    }*/

   SetSnowballSpawnLocations(spawnLocations);
   setSpawnerLocations();





    camera1 = CreateCamera();
    //camera2 = CreateCamera();
    //camera3 = CreateCamera();
    //camera4 = CreateCamera();

    camera1.cameraFront = (T3DVec3) {{0.f, -1.f, -0.27f}};
    fast_vec3_norm(&camera1.cameraFront);


    //CreatePlayer(&playerStruct1);
    for(int i = 0; i < 4; i++)
    {
        CreatePlayer(&players[i]);
    }
    for(int i = 0; i < 4; i++)
    {
        PlayerInit(&players[i], i);
        //T3DVec3 temp = players[i].PlayerActor.Position;
        //temp.v[0] += 50.f * i;
        //players[i].PlayerActor.Position = temp;
    }
    players[0].PlayerActor.Position = snowman1Node.location;
    players[1].PlayerActor.Position = snowman2Node.location;
    players[2].PlayerActor.Position = snowman3Node.location;
    players[3].PlayerActor.Position = snowman4Node.location;

    for(int i = 0; i < 4; i++)
    {
        if ( i >= core_get_playercount())
        {
            players[i].isAI = true;
        }
    }

    

    for(int i = 0; i < 3; i++)
    {
        CreatePickup(&snowballs[i]);
        PickupInit(&snowballs[i], EDT_Empty);
        T3DVec3 temp = snowballs[i].pickupActor.Position;
        temp.v[0] += 50.f * i;
        PickupSetLocation(&snowballs[i], &temp);//Does this even do anything???
        AllActors[i] = &snowballs[i].pickupActor;

        PickupDeactivate(&snowballs[i]);
    }




    /*for(int i = 0; i < 3; i++)
    {
        CreatePickup(&decorations[i]);
        PickupInit(&decorations[i], EPUT_Decoration);
        T3DVec3 temp = decorations[i].pickupActor.Position;
        temp.v[0] += 50.f * i;
        temp.v[2] += 50.f * (i+1);
        PickupSetLocation(&decorations[i], &temp);
    }*/

    for(int i = 0; i < 4; i++)
    { 
        CreateSnowman(&snowmen[i]);
        snowmen[i].snowmanActor.Position.v[2] -= 60.f;
        snowmen[i].snowmanActor.Position.v[0] += i * 70.f;//has to be done first, collision doesn't update after init!!!
        SnowmanInit(&snowmen[i], players[i].playerId);

        AllActors[3+i] = &snowmen[i].snowmanActor;
    }

    snowmen[0].Rotation = -3.14159f/7.f;
    snowmen[3].Rotation = 3.14159f/7.f;

    for(int i = 0; i < 6; i++)
    {
        CreateSpawner(&spawners[i]);
        SpawnerInit(&spawners[i], i+1);
        AllActors[7+i] = &spawners[i].spawnerActor;
        spawners[i].spawnerActor.Position = DecorationSpawnerLocations[i];
    }

    

    //PlayerInit(&playerStruct1);
    CameraInit(&camera1, &players[0].PlayerActor);
    //CameraInit(&camera2, &players[1].PlayerActor);
    //CameraInit(&camera3, &players[2].PlayerActor);
    //CameraInit(&camera4, &players[3].PlayerActor);










///////////////////////////

    T3DMat4 envMat;// = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4_from_srt_euler(&envMat, (float[3]){1.f, 1.f, 1.f}, (float[3]){0.f, 0.f, 0.f}, (float[3]){0.f, 0.f, 0.f});

  EnvActor = (Actor){
    //.model = ,
    .actorType = EAT_Crate,
    .hasCollision = true,
    .isDynamic = false,
    //.initFunc = PlayerInit,
    //.loopFunc = PlayerLoop,
    .collisionType = ECT_Mesh,
    .collisionRadius = 10.f,
    .collisionCenter = (T3DVec3){{0.f, 0.f, 0.f}},
    .CollisionHeight = 50.f,
    .CurrentVelocity = (T3DVec3){{0.f, 0.f, 0.f}},
    .DesiredMovement = (T3DVec3){{0.f, 0.f, 0.f}},
    .Position = (T3DVec3){{0.f, 0.f, -40.f}},
    .PrevPosition = (T3DVec3){{0.f, 0.f, 0.f}},
    .Transform = envMat
    //T3DMat4FP TransformFP
    //rspq_block_t *dpl
  };

  EnvActor.TransformFP = malloc_uncached(sizeof(T3DMat4FP));
  t3d_mat4fp_identity(EnvActor.TransformFP);

  EnvActor.collisionType = ECT_Mesh;

  CalcCapsuleAABB(&EnvActor);


    EnvActor.model = t3d_model_load("rom:/snowmen/SnowyMapTest6_7.t3dm");//("rom:/mygame/SnowyMapTest6_4.t3dm");
    EnvActor.collisionModelPath = "rom:/snowmen/SnowyMapTest6_4_Collision.col";
    rspq_block_begin();
        t3d_matrix_push(EnvActor.TransformFP);
        rdpq_set_prim_color(color_from_packed32(0x17752c));
        t3d_model_draw(EnvActor.model);
        t3d_matrix_pop(1);
    EnvActor.dpl = rspq_block_end();



    t3d_mat4_from_srt_euler(&EnvActor.Transform,
    (float[3]){1.f, 1.f, 1.f},
    (float[3]){0.f, 0.f, 0.f},
    (float[3]){EnvActor.Position.v[0], EnvActor.Position.v[1], EnvActor.Position.v[2]}
  );
  t3d_mat4_to_fixed(EnvActor.TransformFP, &EnvActor.Transform);

    ActorInit(&EnvActor);

    AllActors[13] = &EnvActor;
//////////////////////////////////////treeModel, treeMatFP, treeMat
/*T3DMat4FP* treeMatFP;
rspq_block_t *dplTree;
T3DModel *treeModel;*/
    treeMatFP = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_identity(treeMatFP);
    T3DMat4 treeMat;

    treeModel = t3d_model_load("rom:/snowmen/tree.t3dm");
    rspq_block_begin();
        t3d_matrix_push(treeMatFP);
        rdpq_set_prim_color(RGBA32(21, 115, 42, 255));
        t3d_model_draw(treeModel);
        t3d_matrix_pop(1);
    dplTree = rspq_block_end();



    t3d_mat4_from_srt_euler(&treeMat,
    (float[3]){1.f, 1.f, 1.f},
    (float[3]){0.f, 0.f, 0.f},
    (float[3]){0.f, 0.f, -45.f}
  );
  t3d_mat4_to_fixed(treeMatFP, &treeMat);








    display_set_fps_limit(30);

    syncPoint = 0;
    mixer_ch_set_limits(30, 0, 44100.0, 0);
    wav64_open(&sfx_start, "rom:/core/Start.wav64");
    wav64_open(&sfx_countdown, "rom:/core/Countdown.wav64");
    wav64_open(&sfx_stop, "rom:/core/Stop.wav64");
    wav64_open(&sfx_winner, "rom:/core/Winner.wav64");
    //xm64player_open(&music, "rom:/snake3d/bottled_bubbles.xm64");
     xm64player_open(&music, "rom:/snowmen/christmas_day.xm64");
    //xm64player_play(&music, 0);
    rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); 
}

void draw_loop(bool showText, float deltaTime)
{
    //rdpq_mode_antialias(AA_STANDARD);
    if (TitleScreen)
    {
        t3d_mat4_from_srt_euler(&snowballs[0].pickupActor.Transform,
            (float[3]){.5f, .5f, .5f}, 
            (float[3]){0.f, 0.f, 0.f},
            (float[3]){0.f, 0.f, -20.f}
        );
        t3d_mat4_to_fixed(snowballs[0].pickupActor.TransformFP, &snowballs[0].pickupActor.Transform);

        t3d_mat4_from_srt_euler(&spawners[3].spawnerActor.Transform,
            (float[3]){.5f, .5f, .5f}, 
            (float[3]){0.f, 0.f, 0.f},
            (float[3]){0.f, 0.f, 60.f}
        );
        t3d_mat4_to_fixed(spawners[3].spawnerActor.TransformFP, &spawners[3].spawnerActor.Transform);

        t3d_mat4_from_srt_euler(&snowmen[0].snowmanActor.Transform,
            (float[3]){1.f, 1.f, 1.f}, 
            (float[3]){3.14159f/4, 3.14159f/10, 0.f},
            (float[3]){130.f, 0.f, 50.f}
        );
        t3d_mat4_to_fixed(snowmen[0].snowmanActor.TransformFP, &snowmen[0].snowmanActor.Transform);
        t3d_mat4_from_srt_euler(&snowmen[0].snowmanActor.Transform,
            (float[3]){1.f, 1.f, 1.f}, 
            (float[3]){3.14159f/4, 3.14159f/10, 0.f},
            (float[3]){150.f, -100.f, 50.f}
        );
        t3d_mat4_to_fixed(snowmen[0].snowmanActor.TransformFP, &snowmen[0].snowmanActor.Transform);
          t3d_mat4_to_fixed(snowmen[0].HeadTransformFP,  &snowmen[0].snowmanActor.Transform);
            t3d_mat4_to_fixed(snowmen[0].TorsoTransformFP,  &snowmen[0].snowmanActor.Transform);
                t3d_mat4_to_fixed(snowmen[0].StickTransformFP,  &snowmen[0].snowmanActor.Transform);


        rspq_block_run(snowballs[0].pickupActor.dpl);
        rspq_block_run(spawners[3].spawnerActor.dpl);
        SnowmanDrawAll(&snowmen[0]);

        rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise

        //rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, 10, 10,"%.2f", display_get_fps());
        rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, 20, display_get_height()*.10f,"Grab decorations and Snowballs");// decorations and Snowballs \nwith the A Button!");
            //rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, 45, display_get_height()*.10f," decorations and Snowballs");//A Button!");
                rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, 20, display_get_height()*.17f,"with the ");//A Button!");
                    rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 1 }, 2, 70, display_get_height()*.17f,"A Button!");
        rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, 20, display_get_height()*.25f,"Then");//, bring them back to your \nsmiley face base!");
            rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 2 }, 2, 50, display_get_height()*.25f,"bring them back");// to your \nsmiley face base!");
                rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, 140, display_get_height()*.25f,"to your");// \nsmiley face base!");
                    rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 2 }, 2, 20, display_get_height()*.32f,"smiley face base!");
        rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, 20, display_get_height()*.45f,"Snowballs = 2 points");
        rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, 20, display_get_height()*.55f,"Decorations = 1 point");
        rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 3 }, 2, 20, display_get_height()*.68f,"Stun");// opponents with the B button \nto steal what they're holding!");
            rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, 50, display_get_height()*.68f,"opponents with the ");//B button \nto steal what they're holding!");
                rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 3 }, 2, 160, display_get_height()*.68f,"B button");// \nto steal what they're holding!");
                    rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, 20, display_get_height()*.75f,"to");
                        rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 3 }, 2, 40, display_get_height()*.75f,"steal");
                            rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, 75, display_get_height()*.75f,"what they're holding!");
        rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, 20, display_get_height()*.85f,"The highest scoring snowman wins!");
        rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, display_get_width()/2 - 25.f, display_get_height()*.94f,"Press Start!");
        return;
    }
    //rdpq_mode_antialias(AA_REDUCED);
    if(EndDelay > 0)
    {
        rspq_block_run(dplTree);
        rspq_block_run(EnvActor.dpl);
    }
    //    rdpq_mode_antialias(AA_STANDARD);

    //t3d_matrix_push_pos(1);
        //t3d_matrix_set(envMatFP, true);

        for(int i = 0; i < 4; i++)
        {
            SnowmanUpdateModel(&snowmen[i]);
            SnowmanDraw(&snowmen[i]);
            //rspq_block_run(snowmen[i].snowmanActor.dpl);

        }
        if(EndDelay > 0)
        {
            //rdpq_mode_antialias(AA_REDUCED);
            //t3d_matrix_set(playerActor1.TransformFP, true);
            for(int i = 0; i < 4; i++)
            {
                //rspq_block_run(players[0].PlayerActor.dpl);
                
                rspq_block_run(players[i].PlayerActor.dpl);
                if (players[i].heldPickup != NULL && snowmen[i].snowmanLevel == 0 && players[i].heldPickup->pickupType == EPUT_Snowball)
                {
                    rspq_block_run(players[i].dplArrow);
                }
            }
            //rdpq_mode_antialias(AA_STANDARD);
            for(int i = 0; i < 3; i++)
            {
                if (snowballs[i].pickupState != EPUS_Inactive)
                {
                    PickupUpdateModel(&snowballs[i]);
                    if ((int) GameTimer % 2 == 1 && snowballs[i].pickupState == EPUS_Idle)
                    {
                        rspq_block_run(snowballs[i].dplAltSnowball);
                    }
                    else
                    {
                        rspq_block_run(snowballs[i].pickupActor.dpl);
                    }
                }
                //rspq_block_run(decorations[i].pickupActor.dpl);
            }
            for(int i = 0; i < 6; i++)
            {
                SpawnerUpdateModel(&spawners[i]);
                if(spawners[i].decorations[0].pickupState == EPUS_Inactive || spawners[i].decorations[1].pickupState == EPUS_Inactive)
                {
                    rspq_block_run(spawners[i].spawnerActor.dpl);
                }
                rspq_block_run(spawners[i].dplBase);
                //if both inactive, render empty version (or for now, do not render)
                
                for(int j = 0; j < 2; j++)
                {
                    if (spawners[i].decorations[j].pickupState != EPUS_Inactive)
                    {
                        PickupUpdateModel(&spawners[i].decorations[j]);
                        rspq_block_run(spawners[i].decorations[j].pickupActor.dpl);
                    }
                }
            }
        }

    //t3d_matrix_pop(1);
        
        


    if (showText)
    {
        syncPoint = rspq_syncpoint_new();


        for(int i = 0; i < 4; i++)
        {
            if (EndDelay > 0)
            {
                player_draw_billboard(&players[i]);
            }
        }
        
        for(int i = 0; i < 4; i++)
        {
            if (snowmen[i].snowmanActor.BillboardTimer > 0)
            {
                char string1[32];
                snprintf(string1, sizeof(string1), "+%d", snowmen[i].BillboardValue);
                spawn_temp_billboard(&snowmen[i].snowmanActor.BillboardPosition, string1, false, deltaTime);
                snowmen[i].snowmanActor.BillboardTimer -= deltaTime;
                if (snowmen[i].snowmanActor.BillboardTimer <= 0)
                {
                    snowmen[i].snowmanActor.BillboardTimer = 0.f;
                    snowmen[i].snowmanActor.BillboardPosition = snowmen[i].snowmanActor.Position;
                }
            }
              rdpq_sync_pipe(); // Hardware crashes otherwise
                rdpq_sync_tile(); // Hardware crashes otherwise
        }

        

        for(int i = 0; i < 6; i++)
        {
            if (spawners[i].spawnerActor.BillboardTimer > 0)
            {
                char string1[32];
                snprintf(string1, sizeof(string1), "%d/2", (spawners[i].decorations[0].pickupState == EPUS_Inactive) + (spawners[i].decorations[1].pickupState == EPUS_Inactive));
                spawn_temp_billboard(&spawners[i].spawnerActor.BillboardPosition, string1, false, deltaTime);
                spawners[i].spawnerActor.BillboardTimer -= deltaTime;
                if (spawners[i].spawnerActor.BillboardTimer <= 0)
                {
                    spawners[i].spawnerActor.BillboardTimer = 0.f;
                    spawners[i].spawnerActor.BillboardPosition = spawners[i].spawnerActor.Position;
                }
            }
              rdpq_sync_pipe(); // Hardware crashes otherwise
  rdpq_sync_tile(); // Hardware crashes otherwise
        }


        /*char string1[32];
        snprintf(string1, sizeof(string1), "+%d", 2);
        draw_billboard_generic(&players[0].PlayerActor.Position, string1);*/

        rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise


        if (TESTING) rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, 10, 10,"%.2f", display_get_fps());
        rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, display_get_width() - 70.f, 10,"TIME: %.2f", GameTimer);
        //rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, 10, 30,"Camera front = %f %f %f\n", camera1.cameraFront.v[0], camera1.cameraFront.v[1], camera1.cameraFront.v[2]);

        if (numWinners != -1)
        {
            if (numWinners == 1)
            {
                for(int i = 0; i < 4; i++)
                {
                    if (winners[i])
                    {
                        rdpq_text_printf(NULL, 1, display_get_width()/2 - 70, display_get_height()/2 + 50,"Player %d Wins!\n", i+1);//&(rdpq_textparms_t){ .style_id = winner }
                        break;
                    }
                }
                //winners[0] ? TheWinner = 1 : (winners[1] ? TheWinner = 2 : (winners[2] ? TheWinner = 3 : (winners[3] ? TheWinner = 4 : (TheWinner = -2))));
            }
            else if (numWinners == 4)
            {
                rdpq_text_printf(NULL, 1, display_get_width()/2 - 20, display_get_height()/2 + 50,"Draw!\n");//&(rdpq_textparms_t){ .style_id = winner }
            }
            else if (numWinners == 2)
            {
                int winnerArray[numWinners];
                int winnerIndex = 0;
                for(int i = 0; i < 4; i++)
                {
                    if (winners[i])
                    {
                        winnerArray[winnerIndex] = i+1;
                        winnerIndex++;
                        if(winnerIndex >= numWinners)
                        {
                            break;
                        }
                    }
                }
                rdpq_text_printf(NULL, 1, display_get_width()/2 - 110, display_get_height()/2 + 50,"Players %d and %d Win!\n", winnerArray[0], winnerArray[1]);//&(rdpq_textparms_t){ .style_id = winner }
            }
            else if (numWinners == 3)
            {
                int winnerArray[numWinners];
                int winnerIndex = 0;
                for(int i = 0; i < 4; i++)
                {
                    if (winners[i])
                    {
                        winnerArray[winnerIndex] = i+1;
                        winnerIndex++;
                        if(winnerIndex >= numWinners)
                        {

                            break;
                        }
                    }
                }
                rdpq_text_printf(NULL, 1, display_get_width()/2 - 140, display_get_height()/2 + 50,"Players %d, %d, and %d Win!\n", winnerArray[0], winnerArray[1], winnerArray[2]);//&(rdpq_textparms_t){ .style_id = winner }
            }
        }

        if (pause)
        {
            rdpq_text_printf(NULL, 1, display_get_width()/2 - 60, display_get_height()/2,"PAUSE");
            rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, display_get_width()/2 - 60, display_get_height()/2 + 30,"Press R to Quit");//&(rdpq_textparms_t){ .style_id = winner }
        }
        //rdpq_text_printf(&(rdpq_textparms_t){ .style_id = 4 }, 2, 10, 20,"%.2f",  players[0].newAngle);
        //rdpq_text_printf(NULL, 2, 10, 30,"player pos = %f %f %f\n", players[0].PlayerActor.Position.v[0], players[0].PlayerActor.Position.v[1],  players[0].PlayerActor.Position.v[2]);
        //rdpq_text_printf(NULL, 2, 10, 30,"snowball 1 pos = %f %f %f\n", snowballs[0].pickupActor.Position.v[0], snowballs[0].pickupActor.Position.v[1],  snowballs[0].pickupActor.Position.v[2]);
        //rdpq_text_printf(NULL, 2, 10, 40,"snowball 2 pos = %f %f %f\n", snowballs[1].pickupActor.Position.v[0], snowballs[1].pickupActor.Position.v[1],  snowballs[1].pickupActor.Position.v[2]);
        //rdpq_text_printf(NULL, 2, 10, 50,"snowball 3 pos = %f %f %f\n", snowballs[2].pickupActor.Position.v[0], snowballs[2].pickupActor.Position.v[1],  snowballs[2].pickupActor.Position.v[2]);

    
    
    //debugf("End of Fixed Loop, player pos = %f %f %f\n", players[0].PlayerActor.Position.v[0], players[0].PlayerActor.Position.v[1],  players[0].PlayerActor.Position.v[2]);

        /*rdpq_text_printf(NULL, 2, 10, 30,"%d, %d, %d, %d, %d, %d",  (snowmen[1].decorations & (1 << 0)), (snowmen[1].decorations & (1 << 1)), (snowmen[1].decorations & (1 << 2)),
            (snowmen[1].decorations & (1 << 3)), (snowmen[1].decorations & (1 << 4)), (snowmen[1].decorations & (1 << 5)));
        if (players[1].heldPickup != NULL && players[1].heldPickup->pickupType == EPUT_Decoration)
        {
            rdpq_text_printf(NULL, 2, 10, 40,"P2 current: %d",  0 | (1 << (players[1].heldPickup->decorationType - 1)));
        }
        rdpq_text_printf(NULL, 2, 10, 50,"%d, %d, %d, %d, %d, %d",  (snowmen[0].decorations & (1 << 0)), (snowmen[0].decorations & (1 << 1)), (snowmen[0].decorations & (1 << 2)),
            (snowmen[0].decorations & (1 << 3)), (snowmen[0].decorations & (1 << 4)), (snowmen[0].decorations & (1 << 5)));
        if (players[0].heldPickup != NULL)
        {
            rdpq_text_printf(NULL, 2, 10, 60,"P1 current: %d",  0 | (1 << (players[0].heldPickup->decorationType - 1)));
        }*/

        /*        int typeToAdd = 0 | (1 << (pickupStruct->decorationType - 1));
        debugf("Decoration to add: %d\n", typeToAdd);
        if (typeToAdd >= 0)
        {
            if(!(snowmanStruct->decorations & (1 << typeToAdd)))*/
    }
}

int DecideWinner()//int* points[4], bool* winners[4])
{
    //int points[4];
    //int tempPoints[4];
    //bool tempWinners[4];
    for (int playernum = 0; playernum < 4; playernum++)
    {
        points[playernum] = snowmen[playernum].snowmanLevel * 2;
        debugf("player %d snowball points: %d\n", playernum+1,  points[playernum]);
        for (int i = 0; i < 6; i++)
        {
            if((snowmen[playernum].decorations & (1 << i)))
            {//if (!(snowmen[playerStruct->playerId].decorations & (1 << (spawners[randNum].decorations[0].decorationType - 1))))
                points[playernum]++;
            }
        }
        debugf("player %d final points: %d\n", playernum+1,  points[playernum]);
    }
    
    winners[0] = true;
    int currentLeader = 0;
    for (int i = 1; i < 4; i++)
    {
        if (points[i] > points[currentLeader])
        {
            for(int j = 0; j < 4; j++)
            {
                winners[j] = false;
            }
            currentLeader = i;
            winners[i] = true;
        }
        else if (points[i] == points[currentLeader])
        {
            winners[i] = true;//current leader can stay the same
        }
    }

    int temp = 0;
    for (int i = 0; i < 4; i++)
    {
        if (winners[i] == true)
        {
            temp++;
        }
    }

    //winners = tempWinners;
    //points = tempPoints;
    debugf("snowmen levels: %d, %d, %d, %d\n", snowmen[0].snowmanLevel, snowmen[1].snowmanLevel, snowmen[2].snowmanLevel, snowmen[3].snowmanLevel);
    debugf("snowmen decorations: %d, %d, %d, %d\n", snowmen[0].decorations, snowmen[1].decorations, snowmen[2].decorations, snowmen[3].decorations);
    debugf("points:   %d, %d, %d, %d\n", points[0], points[1], points[2], points[3]);
    debugf("winners:  %d, %d, %d, %d\n", winners[0], winners[1], winners[2], winners[3]);
    
    return temp;
}


void GameOver(float deltatime)
{
    //if (numWinners != -1)
    //{
        camera1.cameraFront = (T3DVec3) {{0.f, -0.5f, -0.9f}};
        fast_vec3_norm(&camera1.cameraFront);
        //(T3DVec3){{ 0.f, 40.f, 25.f }};
        camera1.camTarget.v[2] = -195.f;
        camera1.camTarget.v[1] = -20.f;
        if (WinnerDelay <= -10.f)
        {
            minigame_end();
        }
        if (WinnerDelay <= 0.f)//core_set_winner(winner);
        {
            if (numWinners == -1)
            {
                numWinners = DecideWinner(points, winners);
                int tempNumWinners = numWinners;
                int i = 0;
                while (tempNumWinners > 0)
                {
                    if (winners[i] == true)
                    {
                        core_set_winner(i);
                        tempNumWinners--;
                    }
                    i++;
                }
                wav64_play(&sfx_winner, 31);
            }
            WinnerDelay -= deltatime;
        }
        else
        {
            WinnerDelay -= deltatime;
        }
    //}


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
    if (TitleScreen)
    {
        //joypad_inputs_t joypad[4];
        joypad_buttons_t btn[4];
        //joypad_buttons_t held[4];

        for(int i = 0; i < 4; i++)
        {
            //joypad[i] = joypad_get_inputs(core_get_playercontroller(i));
            btn[i] = joypad_get_buttons_pressed(core_get_playercontroller(i));
            //held[i] = joypad_get_buttons_held(core_get_playercontroller(i));
        }

        if (btn[0].start || btn[1].start || btn[2].start || btn[3].start)
        {
            TitleScreen = false;
            //minigame_end();
        }
        return;
    }
    if (GameEnd == true) 
    {
                    //xm64player_stop(&music);
                    //wav64_play(&sfx_stop, 31);
        if (EndDelay <= 0.f)
        {
            GameOver(deltatime);
        }
        else
        {
            EndDelay -= deltatime;
        }
           return;
        
    }
    if (pause)
    {
        debugf("in pause\n");
        joypad_inputs_t joypad[4];
        joypad_buttons_t btn[4];
        joypad_buttons_t held[4];

        for(int i = 0; i < 4; i++)
        {
            joypad[i] = joypad_get_inputs(core_get_playercontroller(i));
            btn[i] = joypad_get_buttons_pressed(core_get_playercontroller(i));
            held[i] = joypad_get_buttons_held(core_get_playercontroller(i));
        }
        for(int j = 0; j < 4; j++)
        {
            if ( j >= core_get_playercount())
            {
                continue;
            }
            if (btn[j].start) 
            {
                pause = false;
            }
            if (btn[j].r) 
            {
                minigame_end();
            }
        }
        return;
    }
    if (StartTimer > 0.f)
    {
        prevStartTime = StartTimer;
        StartTimer -= deltatime;
        if (StartTimer <= 0.f)
        {
            debugf("Start!\n");
            wav64_play(&sfx_start, 31);
            xm64player_play(&music, 0);
            PlayerControl = true;
        }
        else
        {
            if ((int) prevStartTime != (int) (StartTimer))
            {
                wav64_play(&sfx_countdown, 31);
                
                SpawnSnowball();
            }
            return;//!?!?!!?
        }
    }
    else
    {
        prevTime = GameTimer;
        GameTimer -= deltatime;
        if (GameTimer <= 0.f)
        {
            GameTimer = 0.f;
            debugf("TIME!\n");
            wav64_play(&sfx_stop, 31);
            xm64player_stop(&music);
            GameEnd = true;

            return;
        }
        else
        {
            //debugf("Time: %f\n", GameTimer);
        }
        //debugf("Timer int: %d\n", (int) (GameTimer));
        if ((int) GameTimer % 5 == 0 && (int) prevTime % 5 != 0)//make more complex
        {
            T3DVec3 zeros = {{0.f, 0.f, 0.f}};
            for(int i = 0; i < 4; i++)
            {
            if(t3d_vec3_distance2(&players[0].PlayerActor.Position, &zeros) > 100000.f)
            {
                players[i].PlayerActor.Position = snowman1Node.location;
                debugf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
            }

        }
            //get active snowballs
            //if active snowballs < 3 then spawn one at random location
            SpawnSnowball();
        }
        /*if ((int) GameTimer != (int) (prevTime))//make more complex
        {
            color_t newColor = RGBA32(255, 255, 255, 255);
            if ((int) GameTimer % 2 == 1)
            {
                newColor = color_from_packed32(0xfca9dd<<8);
            }

            for(int i = 0; i < 3; i++)
            {
                if (snowballs[i].pickupState == EPUS_Idle)
                SnowballSwapColours(&snowballs[i], newColor);
            }
        }*/
    }



    joypad_inputs_t joypad[4];
    joypad_buttons_t btn[4];
    joypad_buttons_t held[4];

    for(int i = 0; i < 4; i++)
    {
        joypad[i] = joypad_get_inputs(core_get_playercontroller(i));
        btn[i] = joypad_get_buttons_pressed(core_get_playercontroller(i));
        held[i] = joypad_get_buttons_held(core_get_playercontroller(i));
    }

    if (TESTING)
    {
        if (btn[0].start)
        {
            //fullScreen = !fullScreen;
            //minigame_end();
        }
        if (btn[0].z)
        {
            //PickupActivate(&snowballs[0]);
            SpawnSnowball();
        }
        if (btn[0].r)
        {
            //PickupActivate(&snowballs[0]);
            //SpawnSnowball();
            //!!!!!!!!!!!!Try spawn decoration from spawner
            //SpawnDecoration(&spawners[0], &players[0]);
            //debugf("state: %d\n", spawners[0].decorations[0].pickupState);
            //debugf("location: = %f %f %f\n", spawners[0].decorations[0].pickupActor.Position.v[0], spawners[0].decorations[0].pickupActor.Position.v[1],  spawners[0].decorations[0].pickupActor.Position.v[2]);
            node* backupNode = NodeDA_GetClosestNode(&AllNodes, players[0].PlayerActor.Position);
            players[0].PlayerActor.Position = backupNode->location;
        }
        if (btn[0].l)
        {
            /*players[1].ai_path_index = testpath.length - 1;
            players[1].PlayerActor.Position = players[0].PlayerActor.Position;
            players[3].PlayerActor.Position = testpath.nodeArray[testpath.length - 1]->location;*/
            //minigame_cleanup();
            //minigame_end();
        }
    }


    for(int j = 0; j < 4; j++)
    {
        if ( j >= core_get_playercount())
        {
            continue;
        }
        if (btn[j].start)
        {
            pause = true;
            return;
        }
        if (btn[j].r)
        {
            //node* backupNode = NodeDA_GetClosestNode(&AllNodes, players[i].PlayerActor.Position);
            //players[i].PlayerActor.Position = backupNode->location;
        }
        if (btn[j].a) 
        {
            numAPresses++;
            debugf("OH YEAH LET'S-A-GO! player %d Pressed A Button\n", j);
            if(players[j].heldPickup != NULL)
            {
                /*for(int i = 0; i < 3; i++)
                {
                    if (snowballs[i].holdingPlayerStruct == &players[j])
                    {
                        PlayerDrops(&snowballs[i], &players[j]); 
                    }
                }
                for(int spawnNum = 0; spawnNum < 6; spawnNum++)//each spawner
                {
                    for(int decNum = 0; decNum < 2; decNum++)//each spawner's decorations (2)
                    {
                        //replaced decorations[i] with spawner decorations
                        if (spawners[spawnNum].decorations[decNum].holdingPlayerStruct == &players[j])
                        {
                            PlayerDrops(&spawners[spawnNum].decorations[decNum], &players[j]);
                        }
                    }
                }*/
               PlayerDrops(players[j].heldPickup, &players[j]);
            }
            else
            {
                debugf("no held pickup\n");
                for(int i = 0; i < 3; i++)
                {
                    T3DVec3 penetration_normal;
                    float penetration_depth;

                    if (players[j].heldPickup == NULL && snowballs[i].pickupState == EPUS_Idle && TestCollision(&players[j].PlayerActor, &snowballs[i].triggerStruct.TriggerActor, &penetration_normal, &penetration_depth, deltatime))
                    {
                        //if (snowballs[i].holdingPlayerStruct == NULL)
                        //{
                            PlayerPicksUp(&snowballs[i], &players[j]);
                            
                        //}
                    }
                }
                for(int spawnNum = 0; spawnNum < 6; spawnNum++)//each spawner
                {
                    for(int decNum = 0; decNum < 2; decNum++)//each spawner's decorations (2)
                    {
                    //replaced decorations[i] with spawner decorations
                        //debugf("holding player is null\n");
                        T3DVec3 penetration_normal;
                        float penetration_depth;

                        if (players[j].heldPickup == NULL && spawners[spawnNum].decorations[decNum].pickupState == EPUS_Idle && TestCollision(&players[j].PlayerActor, &spawners[spawnNum].decorations[decNum].triggerStruct.TriggerActor, &penetration_normal, &penetration_depth, deltatime))
                        {
                            //debugf("decoration collision good :)\n");
                            //if (decorations[i].holdingPlayerStruct == NULL)
                            //{
                                PlayerPicksUp(&spawners[spawnNum].decorations[decNum], &players[j]);
                                continue;
                            //}
                        }
                        else{
                            //debugf("decoration collision bad :(\n");//player pos, deco position, deco trigger pos
                            //debugf("Player Position: %f %f %f\n", players[j].PlayerActor.Position.v[0], players[j].PlayerActor.Position.v[1],  players[j].PlayerActor.Position.v[2]);
                            //debugf("Deco Position: %f %f %f\n", spawners[spawnNum].decorations[decNum].pickupActor.Position.v[0], spawners[spawnNum].decorations[decNum].pickupActor.Position.v[1],  spawners[spawnNum].decorations[decNum].pickupActor.Position.v[2]);
                            //debugf("DecoTrigger Position: %f %f %f\n", spawners[spawnNum].decorations[decNum].triggerStruct.TriggerActor.Position.v[0], spawners[spawnNum].decorations[decNum].triggerStruct.TriggerActor.Position.v[1],  spawners[spawnNum].decorations[decNum].triggerStruct.TriggerActor.Position.v[2]);
                        }
                    }
                }
                for(int i = 0; i < 6; i++)
                {
                    T3DVec3 penetration_normal;
                    float penetration_depth;
                    //this player collide with this spawner
                    if (players[j].heldPickup == NULL && TestCollision(&players[j].PlayerActor, &spawners[i].spawnerTrigger.TriggerActor, &penetration_normal, &penetration_depth, deltatime))
                    {
                        debugf("collision good!\n");
                        //for(int decNum = 0; decNum < 2; decNum++)
                        //{
                            //check if either decoration is available
                            //if(spawners[i].decorations[decNum].holdingPlayerStruct == NULL)//!!!!!!!!Is this right? should there be a var just for if active???
                            //{
                                debugf("spawn decoration %d for player %d\n", 69, j);
                                SpawnDecoration(&spawners[i], &players[j]);
                                debugf("Deco 0 state: %d, and is holding? = %d\n", spawners[i].decorations[0].pickupState, spawners[i].decorations[0].holdingPlayerStruct != NULL);
                                debugf("Deco 1 state: %d, and is holding? = %d\n", spawners[i].decorations[1].pickupState, spawners[i].decorations[1].holdingPlayerStruct != NULL);
                                break;
                            //}
                        //}
                        
                    }
                }
            }
        }
    }
        if (TESTING)
        {
            UpdateCameraFromInput(&camera1, &held[0]);
            UpdateCameraFromInput(&camera2, &held[1]);
            UpdateCameraFromInput(&camera3, &held[2]);
            UpdateCameraFromInput(&camera4, &held[3]);
        }

        float stickX[4]; 
        float stickY[4];
        for(int i = 0; i < 4; i++)
        {
            if (!players[i].isAI)  
            {
                //debugf("player %d:\n", i);
                //debugf("playerloop begin!\n");
                stickX[i] = joypad[i].stick_x;
                stickY[i] = joypad[i].stick_y;
                T3DVec3 PlayerStickInput = {{
                    stickX[i] / 83.f,
                    stickY[i] / 83.f,
                    0.0f
                }};
                float mag = sqrtf(PlayerStickInput.v[0]*PlayerStickInput.v[0] + PlayerStickInput.v[1]*PlayerStickInput.v[1]);

                if (fabs(mag) <= 0.2f)
                {
                    players[i].AIStuckTimer = 2.f;
                    players[i].AI_InitialPosition = players[i].PlayerActor.Position;
                }
                else if (players[i].AIStuckTimer <= 0)//might be stuck
                {
                    float dist = t3d_vec3_distance2(&players[i].PlayerActor.Position, &players[i].AI_InitialPosition);
                    //debugf("dist is: %f\n", dist);
                    if ((dist < 2000.f && players[i].PlayerState != EPS_Running) || (dist < 750.f && players[i].PlayerState == EPS_Running))
                    {
                        //debugf("dist is super small!");
                        node* backupNode = NodeDA_GetClosestNode(&AllNodes, players[i].PlayerActor.Position);
                        players[i].PlayerActor.Position = backupNode->location;
                        players[i].AIStuckTimer = 2.f;
                        players[i].AI_InitialPosition = players[i].PlayerActor.Position;
                        continue;
                    }
                    else
                    {
                        players[i].AIStuckTimer = 2.f;
                        players[i].AI_InitialPosition = players[i].PlayerActor.Position;
                    }
                }
                else
                {
                    players[i].AIStuckTimer -= deltatime;
                }
                PlayerLoop(&players[i], &btn[i], AllActors, stickX[i], stickY[i], &camera1.cameraFront, &camera1.cameraUp, deltatime);
            }
            else
            {
                //testpath, get direction to next node
                //float x_dir;
                //float y_dir;
                //AITestWalk(&players[i], &testpath, &x_dir, &y_dir);
                //debugf("player 1 x and y values: %f, %f\n", stickX[0], stickY[0]);
                AIPlayerLoop(&players[i], seed, deltatime);
                seed++;
                stickX[i] = players[i].AI_x * 83.f;
                stickY[i] = players[i].AI_y * 83.f;
                //debugf("Player 2 x and y values: %f, %f\n", stickX[i], stickY[i]);
                PlayerLoop(&players[i], &btn[i], AllActors, stickX[i], stickY[i], &camera1.cameraFront, &camera1.cameraUp, deltatime);
                //T3DVec3 newMove = (T3DVec3) {{x_dir, 0.f, y_dir}};
                //t3d_vec3_add(&players[i].PlayerActor.Position, &players[i].PlayerActor.Position, &newMove);
                
            }
        }



    for(int i = 0; i < 3; i++)
    {

        PickupLoop(&snowballs[i], &camera1, deltatime, &viewportFullScreen);
        //PickupLoop(&decorations[i], deltatime);

        if (snowballs[i].holdingPlayerStruct != NULL && snowballs[i].snowballSize >= snowballs[i].maxSnowballSize)
        {
            T3DVec3 penetration_normal;
            float penetration_depth;
                //debugf("holding %.2f\n", sqrt(t3d_vec3_distance2(&snowman1.snowmanActor.Position, &snowballs[i].pickupActor.Position)));
            if (TestCollision(&snowballs[i].pickupActor, &snowmen[snowballs[i].holdingPlayerStruct->playerId].triggerStruct.TriggerActor, &penetration_normal, &penetration_depth, deltatime))
            {
                //debugf("snowman attempt add\n");
                SnowmanAttemptAdd(&snowmen[snowballs[i].holdingPlayerStruct->playerId], snowballs[i].holdingPlayerStruct, &snowballs[i], &GameEnd);
                if (GameEnd)
                {
                    xm64player_stop(&music);
                    wav64_play(&sfx_stop, 31);
                }
            }
        }
        /*if (decorations[i].holdingPlayerStruct != NULL)
        {
            T3DVec3 penetration_normal;
            float penetration_depth;
                //debugf("holding %.2f\n", sqrt(t3d_vec3_distance2(&snowman1.snowmanActor.Position, &decorations[i].pickupActor.Position)));
            if (TestCollision(&decorations[i].pickupActor, &snowmen[decorations[i].holdingPlayerStruct->playerId].triggerStruct.TriggerActor, &penetration_normal, &penetration_depth, deltatime))
            {
                //debugf("snowman attempt add\n");
                SnowmanAttemptAdd(&snowmen[decorations[i].holdingPlayerStruct->playerId], decorations[i].holdingPlayerStruct, &decorations[i]);
            }
        }    */
    }

    for(int i = 0; i < 6; i++)
    {
        for(int j = 0; j < 2; j++)
        {
            if (spawners[i].decorations[j].holdingPlayerStruct != NULL)
            {
                T3DVec3 penetration_normal;
                float penetration_depth;
                    //debugf("holding %.2f\n", sqrt(t3d_vec3_distance2(&snowman1.snowmanActor.Position, &decorations[i].pickupActor.Position)));
                if (TestCollision(&spawners[i].decorations[j].pickupActor, &snowmen[spawners[i].decorations[j].holdingPlayerStruct->playerId].triggerStruct.TriggerActor, &penetration_normal, &penetration_depth, deltatime))
                {
                    //debugf("snowman attempt add\n");
                    SnowmanAttemptAdd(&snowmen[spawners[i].decorations[j].holdingPlayerStruct->playerId], spawners[i].decorations[j].holdingPlayerStruct, &spawners[i].decorations[j], &GameEnd);
                    if (GameEnd)
                    {
                        xm64player_stop(&music);
                        wav64_play(&sfx_stop, 31);
                    }
                }
            } 
        }
    }



    for(int j = 0; j < 4; j++)
    {
        SnowmanLoop(&snowmen[j], deltatime);

    }

    for(int j = 0; j < 6; j++)
    {
        SpawnerLoop(&spawners[j], &camera1, deltatime, &viewportFullScreen);

    }


    for(int i = 0; i < 4; i++)
    {
        if (players[i].attackActive)
        {
            T3DVec3 penetration_normal;
            float penetration_depth;
            for(int j = 0; j < 4; j++)
            {
                debugf("attack attempt\n");
                //debugf("atk dist %.2f\n", sqrt(t3d_vec3_distance2(&players[i].attackTrigger.TriggerActor.Position, &players[j].PlayerActor.Position)));
                if (i != j && players[j].PlayerState != EPS_Stunned && players[j].invincibleTimer <=0
                && TestCollision(&players[i].attackTrigger.TriggerActor, &players[j].PlayerActor, &penetration_normal, &penetration_depth, deltatime))
                {
                    debugf("Hit player %d!\n", j);
                    wav64_play(&players[j].sfx_hit, 30);
                    PlayerStun(&players[j]);
                }
            }
        }
    }





    seed += numAPresses * (60 - (int) GameTimer) + (int) players[0].newAngle + (int) players[1].newAngle + (int) players[2].newAngle + (int) players[3].newAngle;
    //debugf("+++++++++++++++++++++++seed: %d\n", seed);
    //debugf("++++++++++++++++++++++++++++++seed mod 5: %d\n", seed % 5);

    //dist = sqrt(t3d_vec3_distance2(&playerStruct1.PlayerActor.Position, &snowball1.pickupActor.Position));

    //!!!!!!!!!!!!!!Check if any snowman has been completed
    //if so, call game end giving winning player id
    //debugf("End of Fixed Loop, spawn location occupied:\n%d\n%d\n%d\n", spawnLocations[0].isOccupied, spawnLocations[1].isOccupied, spawnLocations[2].isOccupied);
    //debugf("End of Fixed Loop, player pos = %f %f %f\n", players[0].PlayerActor.Position.v[0], players[0].PlayerActor.Position.v[1],  players[0].PlayerActor.Position.v[2]);
}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/
void minigame_loop(float deltatime)
{
    uint8_t colorAmbient[4] = {0x8B, 0xBC, 0xC9, 0xFF};
    
    for(int i = 0; i < 4; i++)
    {
        PlayerAnimUpdate(&players[i], &snowmen[i].snowmanActor.Position, snowmen[i].snowmanLevel, syncPoint, deltatime);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!maybe remove syncpoint, may not be doing much
    }

    // ======== Draw (3D) ======== //


    //rdpq_sync_pipe();
    //rdpq_mode_zbuf(false, false);

    if (fullScreen)
    {
        CameraLoop(&camera1, &players[0].PlayerActor, &viewportFullScreen);
            rdpq_attach(display_get(), depthBuffer);
            t3d_frame_start();
            t3d_light_set_ambient(colorAmbient);
            if (TitleScreen || EndDelay <= 0)
            {
                t3d_screen_clear_color(color_from_packed32(0x9ab3ed<<8));
            }
            else
            {
                t3d_screen_clear_color(color_from_packed32(0xdedede<<8));
            }
            t3d_screen_clear_depth();
            uint8_t colorDir[4]     = {0xFF, 0xAA, 0xAA, 0xFF};

            t3d_light_set_directional(0, colorDir, &lightDirVec);
            t3d_light_set_count(1);
        draw_loop(true, deltatime);
    }
    else
    {
        CameraLoop(&camera1, &players[0].PlayerActor, &viewports[0]);
        draw_loop(false, deltatime);
        CameraLoop(&camera2, &players[1].PlayerActor, &viewports[1]);
        draw_loop(false, deltatime);
        CameraLoop(&camera3, &players[2].PlayerActor, &viewports[2]);
        draw_loop(false, deltatime);
        CameraLoop(&camera4, &players[3].PlayerActor, &viewports[3]);
        draw_loop(false, deltatime);

        rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
        rdpq_text_printf(NULL, 2, display_get_width()/2 + 10, display_get_height()/2 + 10,"%.2f", display_get_fps());
        //rdpq_text_printf(NULL, 2, display_get_width()/2 + 10, display_get_height()/2 + 20,"%.2f", dist);
        //rdpq_text_printf(NULL, 2, 10, 20,"%ld", core_get_playercount());
    }

    /*if(fullScreen)
    {
        t3d_viewport_attach(&viewportFullScreen);
    }
    else
    {
        t3d_viewport_attach(&viewports[0]);
        t3d_viewport_attach(&viewports[1]);
        t3d_viewport_attach(&viewports[2]);
        t3d_viewport_attach(&viewports[3]);
    }*/






    rdpq_detach_show();
    //debugf("End of Frame\n");
}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup()
{
    /*for(int i = 0; i < 4; i++)
    {
        PlayerCleanup(&players[0]);
    }*/
//MUST STOP MY MUSIC IT WILL INVADE THE OTHER GAMES MY GOD
    wav64_close(&sfx_start);
    wav64_close(&sfx_countdown);
    wav64_close(&sfx_stop);
    wav64_close(&sfx_winner);
    xm64player_stop(&music);
    xm64player_close(&music);

    //main file free mem
    free_uncached(treeMatFP); 
    t3d_model_free(treeModel); 
    rspq_block_free(dplTree);

    ActorFree(&EnvActor);

    //players free mem
    for(int i = 0; i < 4; i++)
    {
        PlayerFree(&players[i]);
    }

    //snowballs and decos free mem
    for(int i = 0; i < 3; i++)
    {
        PickupFree(&snowballs[i]);
    }

    //spawner free
    for(int i = 0; i < 6; i++)
    {
        SpawnerFree(&spawners[i]);
    }

    //snowman free
    for(int i = 0; i < 4; i++)
    {
        SnowmanFree(&snowmen[i]);
    }

    rdpq_text_unregister_font(FONT_TEXT);
    rdpq_text_unregister_font(FONT_BILLBOARD);
    rdpq_font_free(font);
    rdpq_font_free(fontBillboard);


    t3d_destroy(); 
    display_close();

    NodeDA_Free(&AllNodes);
}



    //rspq_block_free(dplEnv);

    //t3d_model_free(modelEnv);

    //free_uncached(envMatFP);