#include "player.h"

#define GRAV_ACC 9.81f * 5.f

/*enum EPlayerState PlayerState;

//CapsuleCollider PlayerCapsule;

T3DVec3 DesiredVelocity;
T3DVec3 DesiredVelocityLocal;
float MaxSpeedChange;
T3DVec3 DesiredMovement;

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
float VerticalMovement;

float Friction;//10.f, make a CONST?

T3DAnim animIdle;
T3DAnim animWalk;
T3DAnim animRun;
T3DAnim animEngaged;
T3DAnim animDodge;
T3DAnim animAttack;
float animBlend;
T3DSkeleton skel;
T3DSkeleton skelBlend;*/


//IN MAIN.C, MUST RUN LOOP FUNCTION ON ALL ACTORS, WHICH INCLUDES GROUNDED MOVEMENT FOR PLAYER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void CreatePlayer(PlayerStruct* playerStruct)
{
  T3DMat4 modelMat;
  t3d_mat4_from_srt_euler(&modelMat,
    (float[3]){0.0035f, 0.0035f, 0.0035f},
    (float[3]){0.f, 0.f, 0.f},
    (float[3]){0.f, 0.f, 0.f}
  );

  *playerStruct = (PlayerStruct)
  {
    //.PlayerActor = 
    .PlayerState = EPS_Idle,
    //T3DVec3 DesiredVelocity;
    //T3DVec3 DesiredVelocityLocal;
    //float MaxSpeedChange;
    //T3DVec3 DesiredMovement;
    //T3DVec3 NewDirection;
    //float newAngle;
    //T3DVec3 CurrentVelocityLocal;
    .MaxSpeed = 120.f,//max walk speed ini
    .MaxAcceleration = 500.f*.8,//max accel walk ini
    .AirAcceleration = 400.f,
    .MaxWalkingSpeed = 120.f*.8,
    .MaxWalkingAcceleration = 500.f,
    .MaxRunningSpeed = 50.f,//250.f,
    .MaxRunningAcceleration = 60.f,//2500.f,
    .MaxJumpAcceleration = 8000.f,
    .VerticalAccelerationChange = 300.f,
    .CurrentVerticalAcceleration = 0.f,
    .CurrentVerticalSpeed = 0.f,
    .VerticalMovement = 0.f, 
    .Friction = 10.f
    //T3DAnim animIdle;
    //T3DAnim animWalk;
    //T3DAnim animRun;
    //T3DAnim animEngaged;
    //T3DAnim animDodge;
    //T3DAnim animAttack;
    //float animBlend;
    //T3DSkeleton skel;
    //T3DSkeleton skelBlend;
  };

  playerStruct->PlayerActor = (Actor){
    //.model = ,
    .actorType = EAT_Player,
    .hasCollision = true,
    .isDynamic = true,
    //.initFunc = PlayerInit,
    //.loopFunc = PlayerLoop,
    .collisionType = ECT_Sphere,
    .collisionRadius = 10.f,
    .collisionCenter = (T3DVec3){{0.f, 0.f, 0.f}},
    .CollisionHeight = 0.f,
    .CurrentVelocity = (T3DVec3){{0.f, 0.f, 0.f}},
    .DesiredMovement = (T3DVec3){{0.f, 0.f, 0.f}},
    .Position = (T3DVec3){{0.f, 0.f, 0.f}},
    .PrevPosition = (T3DVec3){{0.f, 0.f, 0.f}},
    .Transform = modelMat
    //T3DMat4FP TransformFP
    //rspq_block_t *dpl
  };

}

void PlayerInit(struct PlayerStruct* playerStruct, enum EPlayerId playerId)
{
  playerStruct->playerId = playerId;
  playerStruct->AIStuckTimer = 2.f;
  playerStruct->AINoGoalTimer = 0.f;
  playerStruct->invincibleTimer = 0.f;
  playerStruct->arrowOffset = 0.f;

  playerStruct->heldPickup = NULL;

  wav64_open(&playerStruct->sfx_woosh, "rom:/snowmen/swing.wav64");
  wav64_open(&playerStruct->sfx_hit, "rom:/snowmen/Punch__007.wav64");

  playerStruct->PlayerActor.TransformFP = malloc_uncached(sizeof(T3DMat4FP));
  t3d_mat4fp_identity(playerStruct->PlayerActor.TransformFP);

  playerStruct->PlayerActor.collisionType = ECT_Sphere;

      playerStruct->attackTrigger =(TriggerStruct){

    };
    CreateTriggerActor(&playerStruct->attackTrigger);
    TriggerActorInit(&playerStruct->attackTrigger);
    playerStruct->attackTrigger.TriggerActor.collisionRadius = 20.f;

  CalcCapsuleAABB(&playerStruct->PlayerActor);
  switch (playerStruct->playerId)
  {
    case EPID_1:
      playerStruct->PlayerActor.model = t3d_model_load("rom:/snowmen/p1.t3dm");
      break;
    case EPID_2:
      playerStruct->PlayerActor.model = t3d_model_load("rom:/snowmen/catherine_blue.t3dm");
      break;
    case EPID_3:
      playerStruct->PlayerActor.model = t3d_model_load("rom:/snowmen/catherine_yellow.t3dm");
      break;
    case EPID_4:
      playerStruct->PlayerActor.model = t3d_model_load("rom:/snowmen/catherine_green.t3dm");
      break;

    default:
      playerStruct->PlayerActor.model = t3d_model_load("rom:/snowmen/p1.t3dm");
      break;
  }

  //playerStruct->PlayerActor.model = t3d_model_load("rom:/snake3d/snake.t3dm");
  //create skeleton from skinned model, also create blend optimized version
  playerStruct->skel = t3d_skeleton_create(playerStruct->PlayerActor.model);
  playerStruct->skelBlend = t3d_skeleton_clone(&playerStruct->skel, false);//false means no matrices?
  playerStruct->animBlend = 0.0f;
  //Create Animation Instances
  playerStruct->animIdle = t3d_anim_create(playerStruct->PlayerActor.model, "Idle");
  //playerStruct->animIdle = t3d_anim_create(playerStruct->PlayerActor.model, "Snake_Idle");
  t3d_anim_attach(&playerStruct->animIdle, &playerStruct->skel);
  playerStruct->animWalk = t3d_anim_create(playerStruct->PlayerActor.model, "Walk");
  //playerStruct->animWalk = t3d_anim_create(playerStruct->PlayerActor.model, "Snake_Walk");
  t3d_anim_attach(&playerStruct->animWalk, &playerStruct->skelBlend);//MUST ADD Animation that is to be blended to the BLED SKELETON
  //playerStruct->animRun = t3d_anim_create(playerStruct->PlayerActor.model, "Run");
  //t3d_anim_attach(&playerStruct->animRun, &playerStruct->skelBlend);
  playerStruct->animAttack = t3d_anim_create(playerStruct->PlayerActor.model, "Attack1");
  //playerStruct->animAttack = t3d_anim_create(playerStruct->PlayerActor.model, "Snake_Attack");
  t3d_anim_set_looping(&playerStruct->animAttack, false);//Don't loop
  t3d_anim_set_playing(&playerStruct->animAttack, false);//Start in Paused state
  t3d_anim_attach(&playerStruct->animAttack, &playerStruct->skel);
  //playerStruct->animDodge = t3d_anim_create(playerStruct->PlayerActor.model, "Roll");
  //t3d_anim_set_looping(&playerStruct->animDodge, false);//Don't loop
  //t3d_anim_set_playing(&playerStruct->animDodge, false);//Start in Paused state
  //t3d_anim_attach(&playerStruct->animDodge, &playerStruct->skel);

  //Create Display List, only done once for each model.
  rspq_block_begin();
    t3d_matrix_push(playerStruct->PlayerActor.TransformFP);//push the transformation matrix we created, 
      //can alernatively push(1) and then set the mat you want t3d_matrix_set(,)
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
    //t3d_model_draw(model);// Draw Static Mesh
    t3d_model_draw_skinned(playerStruct->PlayerActor.model, &playerStruct->skel);// Draw skinned mesh with main skeleton.  
    t3d_matrix_pop(1);// must also pop it when done
  playerStruct->PlayerActor.dpl = rspq_block_end();


  playerStruct->ai_path_index = 0;


    T3DMat4 ArrowTransform;
    T3DMat4FP *ArrowTransformFP;
    T3DModel* modelArrow;
    rspq_block_t* dplArrow;

  playerStruct->ArrowTransformFP = malloc_uncached(sizeof(T3DMat4FP));
  t3d_mat4fp_identity(playerStruct->ArrowTransformFP);


  playerStruct->modelArrow = t3d_model_load("rom:/snowmen/arrow.t3dm");


    rspq_block_begin();
    t3d_matrix_push( playerStruct->ArrowTransformFP);
        rdpq_set_prim_color(RGBA32(70, 70, 70, 255));
        t3d_model_draw(playerStruct->modelArrow);
    t3d_matrix_pop(1);
    playerStruct->dplArrow = rspq_block_end();


}

void PlayerLoop(struct PlayerStruct* playerStruct, joypad_buttons_t* btn, Actor** AllActors, float XStick, float YStick, T3DVec3* cameraFront, T3DVec3* cameraUp, float deltaTime)
{
  CalcCapsuleAABB(&playerStruct->PlayerActor);
  /*if (btn->r && !playerStruct->isAI)
  {
    playerStruct->PlayerActor.Position = (T3DVec3) {{-120.f, 0.f, -150.f}};
    return;
  }*/
  if (playerStruct->invincibleTimer > 0)
  {
    playerStruct->invincibleTimer -= deltaTime;
  }
  int numCollisions = 0;
  //updateController(deltaTime);
  //debugf("state: %d, playerid: %d\n", playerStruct->PlayerState, playerStruct->playerId);
  if (playerStruct->PlayerState == EPS_Idle || playerStruct->PlayerState == EPS_Running)
  {
    playerStruct->attackActive = false;
    if(!playerStruct->isAI && btn->b && playerStruct->PlayerState != EPS_EngagedInAttack && playerStruct->heldPickup == NULL)
    {
      //debugf("let's attack!\n");
        //Player_Jump(playerStruct, deltaTimeFraction);
        //minigame_end();
        PlayerAttackBegin(playerStruct, XStick, YStick);
        //debugf("state(X): %d\n", playerStruct->PlayerState);
        return;
    }

    
    //float deltaTimeFraction = deltaTime * .25f;
    //for (int i = 0; i < 4; i++)//!!!! May not want to do ALL of grounded movement 4 times, just collision check 4 times
    //{
    // ===========================================
    while (numCollisions<3)//I think this is wrong... Need amount of distance left? don't want to double up on grounded movement...
    {
      if (GroundedMovement(playerStruct, AllActors, XStick, YStick, cameraFront, cameraUp, deltaTime))
      {
        numCollisions++;
      }
      else
      {
        break;
      }
    }

    //{
    //  break;
    //}
    // ===========================================
    //}
  }
  else if (playerStruct->PlayerState == EPS_EngagedInAttack)
  {
    debugf("attacking... %f\n", playerStruct->attackTimer);
    playerStruct->attackTimer -= deltaTime;
    playerStruct->attackActive = false;
    if (playerStruct->attackTimer <= 0)
    {
      PlayerAttackEnd(playerStruct);
    }
    else if (playerStruct->attackTimer <= 0.35)
    {
      playerStruct->attackActive = true;
      debugf("ATTACK!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      //Check collision against other players? Maybe do that in main file.
    }
    else
    {
      
    }
  }
  else if (playerStruct->PlayerState == EPS_Stunned)
  {
    debugf("stunned..... %f\n", playerStruct->stunTimer);
    playerStruct->stunTimer -= deltaTime;
    if (playerStruct->stunTimer <= 0)
    {
      PlayerStunEnd(playerStruct);
    }
    else
    {
      playerStruct->newAngle += 0.2f;
    }
  }

    playerStruct->attackTrigger.TriggerActor.Position = playerStruct->PlayerActor.Position;
    T3DVec3 NewDirection = {{playerStruct->PlayerActor.CurrentVelocity.v[0], 0.f, playerStruct->PlayerActor.CurrentVelocity.v[2]}};
    fast_vec3_norm(&NewDirection);
    playerStruct->attackTrigger.TriggerActor.Position.v[0] += NewDirection.v[0] * 20.f;
    playerStruct->attackTrigger.TriggerActor.Position.v[2] += NewDirection.v[2] * 20.f;
}

void PlayerAnimUpdate(struct PlayerStruct* playerStruct, T3DVec3* SnowmanPosition, int snowmanLevel, rspq_syncpoint_t syncPoint, float deltaTime)
{

  float modelScale = 0.0035f;//0.0035f

  //=========== Update Animation ==========//
    //I don't know why current speed is messed up, but I have to recalulate here for now.
  playerStruct->currentSpeed = sqrtf(playerStruct->PlayerActor.CurrentVelocity.v[0]*playerStruct->PlayerActor.CurrentVelocity.v[0] + playerStruct->PlayerActor.CurrentVelocity.v[2]*playerStruct->PlayerActor.CurrentVelocity.v[2]);

  playerStruct->animBlend = playerStruct->currentSpeed / playerStruct->MaxSpeed;
  if (playerStruct->animBlend > 1.0f) playerStruct->animBlend = 1.0f;
  //else if (animBlend > 0.0001f) animBlend += 0.3f;
  t3d_anim_update(&playerStruct->animIdle, deltaTime);
  //debugf("animBlend=%f", playerStruct->animBlend);
  /*if (playerStruct->PlayerState == EPS_Running)
  {
    //debugf("---- 1 ----=");
    t3d_anim_set_speed(&playerStruct->animRun, playerStruct->animBlend);
    t3d_anim_update(&playerStruct->animRun, deltaTime);
  }
  else
  {*/
    //debugf("---- 2 ----=");
    t3d_anim_set_speed(&playerStruct->animWalk, playerStruct->animBlend);
    t3d_anim_update(&playerStruct->animWalk, deltaTime);
  //}
  /*if (t3d_vec3_len2(&CurrentVelocity) > 0.01f)
  {
    t3d_anim_update(&animWalk, deltaTime);//Whichever animation updates last is the one that counts
  }*/
  if(playerStruct->PlayerState == EPS_EngagedInAttack)
  {
    debugf("YAAAAAAAAAAAR ATAAAAAAAAAAAAAAAAAAAAAAAAAAACK!!!!!!!!!!111!\n");
    t3d_anim_update(&playerStruct->animAttack, deltaTime);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    if(!playerStruct->animAttack.isPlaying)
    {
      debugf("attack end bro\n");
      PlayerAttackEnd(playerStruct);// will trigger at end of anim b/c it's not looping!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    } 
  }
  else//attack uses all bones, therfore the blending anim will overwrite EVERYTHING, so attack won't play.
  {
    // We now blend the walk animation with the idle/attack one
    t3d_skeleton_blend(&playerStruct->skel, &playerStruct->skel, &playerStruct->skelBlend, playerStruct->animBlend);
  }

  //if(syncPoint)rspq_syncpoint_wait(syncPoint);

    t3d_skeleton_update(&playerStruct->skel);

  //=========== Update Model =============//
  t3d_mat4_from_srt_euler(&playerStruct->PlayerActor.Transform,
    (float[3]){modelScale, modelScale, modelScale},
    (float[3]){0.f, playerStruct->newAngle, 0.f},
    (float[3]){playerStruct->PlayerActor.Position.v[0], playerStruct->PlayerActor.Position.v[1], playerStruct->PlayerActor.Position.v[2]}
  );
  t3d_mat4_to_fixed(playerStruct->PlayerActor.TransformFP, &playerStruct->PlayerActor.Transform);

if (playerStruct->heldPickup != NULL && snowmanLevel == 0 && playerStruct->heldPickup->pickupType == EPUT_Snowball)
{
    T3DVec3 NewDirection;
    t3d_vec3_diff(&NewDirection, SnowmanPosition, &playerStruct->PlayerActor.Position);//diff snowman and player{{pickupStruct->holdingPlayerStruct->PlayerActor.CurrentVelocity.v[0], 0.f, pickupStruct->holdingPlayerStruct->PlayerActor.CurrentVelocity.v[2]}};
    fast_vec3_norm(&NewDirection);
    //pickupStruct->pickupActor.Position.v[0] += NewDirection.v[0] * 15.f;
    //pickupStruct->pickupActor.Position.v[2] += NewDirection.v[2] * 15.f;
    float arrowAngle = atan2f(-NewDirection.v[0], NewDirection.v[2]);

    t3d_mat4_from_srt_euler(&playerStruct->ArrowTransform,
    (float[3]){.3f, .3f, .3f},
    (float[3]){0.f, arrowAngle, 0.f},
    (float[3]){playerStruct->PlayerActor.Position.v[0] + NewDirection.v[0]*sinf(playerStruct->arrowOffset)*15, playerStruct->PlayerActor.Position.v[1], playerStruct->PlayerActor.Position.v[2] + NewDirection.v[2]*sinf(playerStruct->arrowOffset)*15}
  );
  t3d_mat4_to_fixed(playerStruct->ArrowTransformFP, &playerStruct->ArrowTransform);
  playerStruct->arrowOffset += deltaTime*3;
}

}




bool GroundedMovement(PlayerStruct* ThisPlayer, Actor** AllActors, float XStick, float YStick, T3DVec3* cameraFront, T3DVec3* cameraUp, float deltaTime)
{
    //debugf("       *      *       *        *              grounded movement begin\n");
    ThisPlayer->PlayerActor.PrevPosition = ThisPlayer->PlayerActor.Position;
    T3DVec3 PlayerStickInput = {{
        XStick / 83.f,
        YStick / 83.f,
        0.0f
    }};
    float mag = sqrtf(PlayerStickInput.v[0]*PlayerStickInput.v[0] + PlayerStickInput.v[1]*PlayerStickInput.v[1]);
    if (t3d_vec3_len2(&PlayerStickInput) > 1.f)
    {
        fast_vec3_norm(&PlayerStickInput);
    }
    if (fabs(mag) > 0.05f)//only update acceleration if stick is beyond deadzone
    {
        T3DVec3 RightVector;
        t3d_vec3_cross(&RightVector, cameraFront, cameraUp);//Get right vector from up vector and front pointing vector
        fast_vec3_norm(&RightVector);
        T3DVec3 RightVectorNorm = RightVector;
        RightVectorNorm.v[1] = 0.f;
        fast_vec3_norm(&RightVectorNorm);
        T3DVec3 CameraFrontNorm = *cameraFront;
        CameraFrontNorm.v[1] = 0.f;
        fast_vec3_norm(&CameraFrontNorm);

        //================= Grounded Input ===================
        float Acceleration;
        if (ThisPlayer->PlayerState == EPS_Airborne)
        {
            Acceleration = ThisPlayer->AirAcceleration;
        }
        else
        {
            Acceleration = ThisPlayer->MaxAcceleration;
        }
            ThisPlayer->DesiredVelocityLocal = (T3DVec3){{//Need to project camera right and front onto ground
            (PlayerStickInput.v[1]) * ThisPlayer->MaxSpeed,
            0.f,
            (PlayerStickInput.v[0]) * ThisPlayer->MaxSpeed
            }};

            ThisPlayer->MaxSpeedChange = Acceleration * deltaTime;
            //use movetowardvector instead???
            ThisPlayer->CurrentVelocityLocal.v[0] = MoveTowards(ThisPlayer->CurrentVelocityLocal.v[0], ThisPlayer->DesiredVelocityLocal.v[0], ThisPlayer->MaxSpeedChange, ThisPlayer->Friction, deltaTime);//why isn't pass by reference working?!?
            ThisPlayer->CurrentVelocityLocal.v[2] = MoveTowards(ThisPlayer->CurrentVelocityLocal.v[2], ThisPlayer->DesiredVelocityLocal.v[2], ThisPlayer->MaxSpeedChange, ThisPlayer->Friction, deltaTime);

            if(!ThisPlayer->isAI)
            {
              ThisPlayer->PlayerActor.CurrentVelocity = (T3DVec3){{
              CameraFrontNorm.v[0] * ThisPlayer->CurrentVelocityLocal.v[0]  + RightVectorNorm.v[0] * ThisPlayer->CurrentVelocityLocal.v[2],
              0.f,
              CameraFrontNorm.v[2] * ThisPlayer->CurrentVelocityLocal.v[0]  + RightVectorNorm.v[2] * ThisPlayer->CurrentVelocityLocal.v[2]
              }};
            }
            else
            {
              ThisPlayer->PlayerActor.CurrentVelocity = ThisPlayer->CurrentVelocityLocal;
            }

    }
    else// needs to be changed, more specific for states
    {
    ThisPlayer->currentSpeed = sqrtf(ThisPlayer->PlayerActor.CurrentVelocity.v[0]*ThisPlayer->PlayerActor.CurrentVelocity.v[0] + ThisPlayer->PlayerActor.CurrentVelocity.v[2]*ThisPlayer->PlayerActor.CurrentVelocity.v[2]);
    //debugf("currentSpeed (1) =%f\n", ThisPlayer->currentSpeed);
    ThisPlayer->currentSpeed = ApplyFriction(ThisPlayer->currentSpeed, ThisPlayer->Friction, deltaTime);
    //debugf("currentSpeed (2) =%f\n", ThisPlayer->currentSpeed);

    fast_vec3_norm(&ThisPlayer->PlayerActor.CurrentVelocity);

    ThisPlayer->PlayerActor.CurrentVelocity = (T3DVec3){{
    ThisPlayer->PlayerActor.CurrentVelocity.v[0] * ThisPlayer->currentSpeed,
    0.f,
    ThisPlayer->PlayerActor.CurrentVelocity.v[2] * ThisPlayer->currentSpeed
    }};

    fast_vec3_norm(&ThisPlayer->CurrentVelocityLocal);
    ThisPlayer->CurrentVelocityLocal = (T3DVec3){{
    ThisPlayer->CurrentVelocityLocal.v[0] * ThisPlayer->currentSpeed,
    0.f,
    ThisPlayer->CurrentVelocityLocal.v[2] * ThisPlayer->currentSpeed
    }};
    }

    ThisPlayer->CurrentVerticalSpeed -= GRAV_ACC*10.f * deltaTime;//10.f is unit
    ThisPlayer->VerticalMovement = ThisPlayer->CurrentVerticalSpeed * deltaTime;



    ThisPlayer->PlayerActor.DesiredMovement = (T3DVec3){{
    ThisPlayer->PlayerActor.CurrentVelocity.v[0] * deltaTime,
    0.f,//ThisPlayer->VerticalMovement,
    ThisPlayer->PlayerActor.CurrentVelocity.v[2] * deltaTime
    }};

    //t3d_vec3_add(&ThisPlayer->PlayerActor.Position, &ThisPlayer->PlayerActor.Position, &ThisPlayer->PlayerActor.DesiredMovement);***********************

    //debugf("End of Grounded Movement\n");

    /*if (t3d_vec3_len2(&ThisPlayer->PlayerActor.DesiredMovement) != 0.f)
    {
        //debugf("Desired Movement: %.2f, %.2f, %.2f", playerActor->DesiredMovement.v[0], playerActor->DesiredMovement.v[1], playerActor->DesiredMovement.v[2]);
        //debugf("we're in boys, len=%f", t3d_vec3_len2(&playerActor->DesiredMovement));
        T3DVec3 NewDirection = {{-ThisPlayer->PlayerActor.CurrentVelocity.v[0], 0.f, ThisPlayer->PlayerActor.CurrentVelocity.v[2]}};
        //debugf("NewDirection=%f, %f", NewDirection.v[0], NewDirection.v[2]);
        ThisPlayer->newAngle = atan2f(NewDirection.v[0], NewDirection.v[2]);
    }*/

/*****************************************************
-------------------Collision-------------------------
*******************************************************/
    //debugf("Collision Begin\n");
    T3DVec3 penetration_normal;
    float penetration_depth = 0.f;
    bool collide = false;
    float deltaTimeFraction = deltaTime * .25f;
    T3DVec3 QuarterMovement;
    t3d_vec3_scale(&QuarterMovement, &ThisPlayer->PlayerActor.DesiredMovement, .25f);

  for(int i = 0; i < 4; i++)
  {
    t3d_vec3_add(&ThisPlayer->PlayerActor.Position, &ThisPlayer->PlayerActor.Position, &QuarterMovement);
    ThisPlayer->PlayerActor.collisionCenter = (T3DVec3){{
        ThisPlayer->PlayerActor.Position.v[0],
        ThisPlayer->PlayerActor.Position.v[1] + ThisPlayer->PlayerActor.CollisionHeight,
        ThisPlayer->PlayerActor.Position.v[2]
    }};
    //debugf("Movement Collision Test:\n");
    if (TestAllCollision(&ThisPlayer->PlayerActor, AllActors, &penetration_normal, &penetration_depth, deltaTimeFraction))
    {
      collide = true;
      //debugf("Well then... Penetration Depth = %f\n", penetration_depth);
        //debugf("Collision Detected! Normal: %f %f %f, \nDepth: %f\n",penetration_normal.v[0], penetration_normal.v[1], penetration_normal.v[2],
        //  penetration_depth);
        ThisPlayer->PlayerActor.Position = ThisPlayer->PlayerActor.PrevPosition;
        penetration_normal.v[1] = 0.f;
        fast_vec3_norm(&penetration_normal);
        CapsuleRespondCollideNSlide(&ThisPlayer->PlayerActor, &penetration_normal, penetration_depth, deltaTimeFraction);
        break;
    }
    //debugf("HA HA HA WHAT... Penetration Depth = %f\n", penetration_depth);

    /*ThisPlayer->PlayerActor.Position = (T3DVec3){{!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        ThisPlayer->PlayerActor.Position.v[0],
        0.f,
        ThisPlayer->PlayerActor.Position.v[2]
    }};*/


    /*ThisPlayer->PlayerActor.PrevPosition = ThisPlayer->PlayerActor.Position;

    ThisPlayer->PlayerActor.DesiredMovement = (T3DVec3){{
    0.f,//PlayerActor->CurrentVelocity.v[0] * deltaTime,
    0.f,//ThisPlayer->VerticalMovement,
    0.f//PlayerActor->CurrentVelocity.v[2] * deltaTime
    }};

    t3d_vec3_add(&ThisPlayer->PlayerActor.Position, &ThisPlayer->PlayerActor.Position, &ThisPlayer->PlayerActor.DesiredMovement);
    
    ThisPlayer->PlayerActor.collisionCenter = (T3DVec3){{
        ThisPlayer->PlayerActor.Position.v[0],
        ThisPlayer->PlayerActor.Position.v[1] + ThisPlayer->PlayerActor.CollisionHeight,
        ThisPlayer->PlayerActor.Position.v[2]
    }};*/


  /*
    //debugf("Gravity Collision Test:\n");
    if (TestAllCollision(&ThisPlayer->PlayerActor, AllActors, &penetration_normal, &penetration_depth, deltaTime))
    {
        float dot_norm_camup = t3d_vec3_dot(&penetration_normal, cameraUp);
        //debugf("dot norm camup = %f\n", dot_norm_camup);
        if (dot_norm_camup < 0.7f)
        {//Sliding:
            ThisPlayer->PlayerActor.Position = ThisPlayer->PlayerActor.PrevPosition;
            CapsuleRespondCollideNSlide(&ThisPlayer->PlayerActor, &penetration_normal, penetration_depth, deltaTime);
            ThisPlayer->CurrentVerticalSpeed = 0.f;
            ThisPlayer->VerticalMovement = 0.f;
        }
        else
        {
            ThisPlayer->PlayerActor.Position = ThisPlayer->PlayerActor.PrevPosition;
            ThisPlayer->CurrentVerticalSpeed = 0.f;
            ThisPlayer->VerticalMovement = 0.f;
        }
    }

    //t3d_vec3_add(&CapsuleActor->Position, &CapsuleActor->Position, &CapsuleActor->DesiredMovement);
    ThisPlayer->PlayerActor.collisionCenter = (T3DVec3){{
        ThisPlayer->PlayerActor.Position.v[0],
        ThisPlayer->PlayerActor.Position.v[1] + ThisPlayer->PlayerActor.CollisionHeight,
        ThisPlayer->PlayerActor.Position.v[2]
    }};*/

    /*****************************************************
    -------------------Collision End----------------------
    *******************************************************/
  }

    if (t3d_vec3_len2(&ThisPlayer->PlayerActor.CurrentVelocity) >= 0.1f)
    {
        //debugf("Desired Movement: %.2f, %.2f, %.2f", playerActor->DesiredMovement.v[0], playerActor->DesiredMovement.v[1], playerActor->DesiredMovement.v[2]);
        //debugf("we're in boys, len=%f", t3d_vec3_len2(&playerActor->DesiredMovement));
        //T3DVec3 NewDirection = {{-ThisPlayer->PlayerActor.CurrentVelocity.v[0], 0.f, ThisPlayer->PlayerActor.CurrentVelocity.v[2]}};
        //debugf("NewDirection=%f, %f", NewDirection.v[0], NewDirection.v[2]);
        T3DVec3 NewDirection;
        if (collide)
        {
          NewDirection = (T3DVec3) {{-PlayerStickInput.v[0], 0.f, -PlayerStickInput.v[1]}};
        }
        else
        {
          NewDirection = (T3DVec3) {{-ThisPlayer->PlayerActor.CurrentVelocity.v[0], 0.f, ThisPlayer->PlayerActor.CurrentVelocity.v[2]}};
        }


        ThisPlayer->newAngle = atan2f(NewDirection.v[0], NewDirection.v[2]);
    }
    else
    {
        //debugf("OH NO D:, len=%f", t3d_vec3_len2(&playerActor->DesiredMovement));
        //debugf("Desired Movement: %.2f, %.2f, %.2f", playerActor->DesiredMovement.v[0], playerActor->DesiredMovement.v[1], playerActor->DesiredMovement.v[2]);
    }


  return collide;
}


void Player_Jump(PlayerStruct* ThisPlayer, float deltaTime)
{
    if (ThisPlayer->PlayerState == EPS_Idle)
    {
        ThisPlayer->CurrentVerticalSpeed += sqrtf(-2.f * -GRAV_ACC*10.f * 6.f*10.f);//10.f is unit
        //ThisPlayer->VerticalMovement = ThisPlayer->CurrentVerticalSpeed * deltaTime;
        ThisPlayer->PlayerState = EPS_Airborne;
    } 
}

void Player_RunBegin(PlayerStruct* ThisPlayer)
{
    ThisPlayer->PlayerState = EPS_Running;
    ThisPlayer->MaxSpeed = ThisPlayer->MaxRunningSpeed;
    ThisPlayer->MaxAcceleration = ThisPlayer->MaxRunningAcceleration;
}

void Player_RunEnd(PlayerStruct* ThisPlayer)
{
    ThisPlayer->PlayerState = EPS_Idle;
    ThisPlayer->MaxSpeed = ThisPlayer->MaxWalkingSpeed;
    ThisPlayer->MaxAcceleration = ThisPlayer->MaxWalkingAcceleration;
}


void PlayerCleanup(PlayerStruct* playerStruct)
{
  rspq_block_free(playerStruct->PlayerActor.dpl);
  free_uncached(playerStruct->PlayerActor.TransformFP);

  t3d_skeleton_destroy(&playerStruct->skel);
  t3d_skeleton_destroy(&playerStruct->skelBlend);
  
  t3d_anim_destroy(&playerStruct->animIdle);
  t3d_anim_destroy(&playerStruct->animWalk);
  t3d_anim_destroy(&playerStruct->animRun);
  t3d_anim_destroy(&playerStruct->animIdle);
  t3d_anim_destroy(&playerStruct->animEngaged);
  t3d_anim_destroy(&playerStruct->animAttack);
}

void PlayerAttackBegin(PlayerStruct* playerStruct, float XStick, float YStick)
{
    T3DVec3 PlayerStickInput = {{
      XStick / 83.f,
      YStick / 83.f,
      0.0f
  }};
  T3DVec3 NewDirection = {{-PlayerStickInput.v[0], 0.f, -PlayerStickInput.v[1]}};
  playerStruct->newAngle = atan2f(NewDirection.v[0], NewDirection.v[2]);

  t3d_anim_set_playing(&playerStruct->animAttack, true);
  t3d_anim_set_time(&playerStruct->animAttack, 0.0f);
  playerStruct->PlayerState = EPS_EngagedInAttack;
  playerStruct->attackTimer = .6f;
  wav64_play(&playerStruct->sfx_woosh, 30);
  //playerStruct->attackActive = true;
}

void PlayerAttackEnd(PlayerStruct* playerStruct)
{
  playerStruct->PlayerState = EPS_Idle;
  //playerStruct->attackTimer = 0.5f;
  playerStruct->attackActive = false;
}

void PlayerStun(PlayerStruct* playerStruct)
{
  debugf("player has been stunned!!!\n");
  playerStruct->PlayerState = EPS_Stunned;
  playerStruct->stunTimer = 1.8f;
  if (playerStruct->heldPickup != NULL)
  {
    PlayerDrops(playerStruct->heldPickup, playerStruct);
  }
}

void PlayerStunEnd(PlayerStruct* playerStruct)
{
  debugf("stun end\n");
  playerStruct->PlayerState = EPS_Idle;
  playerStruct->invincibleTimer = 2.f;
}


void PlayerFree(PlayerStruct* playerStruct)
{

  t3d_skeleton_destroy(&playerStruct->skel);
  t3d_skeleton_destroy(&playerStruct->skelBlend);

  t3d_anim_destroy(&playerStruct->animIdle);
  t3d_anim_destroy(&playerStruct->animWalk);
  t3d_anim_destroy(&playerStruct->animAttack);


  ActorFree(&playerStruct->PlayerActor);
  /*free_uncached(playerStruct->PlayerActor.TransformFP);
  t3d_model_free(playerStruct->PlayerActor.model);
  rspq_block_free(playerStruct->PlayerActor.dpl);*/

if(playerStruct->dplArrow != NULL)
{
  rspq_block_free(playerStruct->dplArrow);
}
if(playerStruct->ArrowTransformFP != NULL)
{
  free_uncached(playerStruct->ArrowTransformFP);
}
if(playerStruct->modelArrow != NULL)
{
  t3d_model_free(playerStruct->modelArrow);
}


  free(playerStruct->AIPath.nodeArray);

  TriggerFree(&playerStruct->attackTrigger);

    wav64_close(&playerStruct->sfx_woosh);
    wav64_close(&playerStruct->sfx_hit);
}


  //modelMatFP3

  /*PlayerActor.Transform.m[3][0] = PlayerPosition.v[0];
  PlayerActor.Transform.m[3][1] = PlayerPosition.v[1];
  PlayerActor.Transform.m[3][2] = PlayerPosition.v[2];*/








 //PlayerSphere.center = PlayerPosition;

  /*T3DVec3 playerTip = (T3DVec3)
  {
    PlayerActor.collisionCenter.v[0] + PlayerActor.CollisionHeight;
    PlayerActor.collisionCenter.v[1] + PlayerActor.CollisionHeight;
    PlayerActor.collisionCenter.v[2] + PlayerActor.CollisionHeight;
  }
    T3DVec3 playerBase = (T3DVec3)
  {
    PlayerActor.collisionCenter.v[0] - PlayerActor.CollisionHeight;
    PlayerActor.collisionCenter.v[1] - PlayerActor.CollisionHeight;
    PlayerActor.collisionCenter.v[2] - PlayerActor.CollisionHeight;
  }

  CapsuleCollider PlayerCapsule = (CapsuleCollider){
    PlayerActor.collisionRadius,
    PlayerBase,
    PlayerTip
  };

  PlayerCapsule.base = PlayerPosition;
  PlayerCapsule.tip = PlayerPosition;
  PlayerCapsule.tip.v[1] += 50.f;*/


