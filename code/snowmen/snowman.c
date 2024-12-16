#include "snowman.h"


void CreateSnowman(struct SnowmanStruct* snowmanStruct)
{
    //debugf("please work\n");

    T3DMat4 modelMat;
    t3d_mat4_from_srt_euler(&modelMat,
        (float[3]){1.f, 1.f, 1.f},
        (float[3]){0.f, 0.f, 0.f},
        (float[3]){0.f, 0.f, 0.f}
    );

    *snowmanStruct = (SnowmanStruct){
        .snowmanLevel = ESL_level0,
        .snowmanState = ESS_Empty
        //enum ESnowmanOwner snowmanOwner;
        //Actor snowmanActor;
    };

    snowmanStruct->snowmanActor = (Actor){
    //.model = ,
    .actorType = EAT_Pickup,//!!!!!!!!!!!!!!!!!!!!!!is this wrong???
    .hasCollision = true,
    .isDynamic = false,
    //.initFunc = PlayerInit,
    //.loopFunc = PlayerLoop,
    .collisionType = ECT_Sphere,
    .collisionRadius = 20.f,
    .collisionCenter = (T3DVec3){{0.f, 0.f, 0.f}},
    .CollisionHeight = 50.f,
    .CurrentVelocity = (T3DVec3){{0.f, 0.f, 0.f}},
    .DesiredMovement = (T3DVec3){{0.f, 0.f, 0.f}},
    .Position = (T3DVec3){{-105.f, 0.f, -100.f}},
    .PrevPosition = (T3DVec3){{0.f, 0.f, 0.f}},
    .Transform = modelMat
    //T3DMat4FP TransformFP
    //rspq_block_t *dpl
    };
}

void SnowmanInit(struct SnowmanStruct* snowmanStruct, enum EPlayerId newOwner)
{
    snowmanStruct->snowmanOwner = newOwner;
    snowmanStruct->snowmanLevel = ESL_level0;
    snowmanStruct->snowmanState = ESS_Empty;
    snowmanStruct->decorations = 0;
    snowmanStruct->Rotation = 0.f;

    wav64_open(&snowmanStruct->sfx_collect, "rom:/snowmen/chainmail1.wav64");

    snowmanStruct->snowmanActor.TransformFP = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_identity(snowmanStruct->snowmanActor.TransformFP);

    snowmanStruct->HeadTransformFP = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_identity(snowmanStruct->HeadTransformFP);
    snowmanStruct->TorsoTransformFP = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_identity(snowmanStruct->TorsoTransformFP);
    snowmanStruct->StickTransformFP = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_identity(snowmanStruct->StickTransformFP);

    snowmanStruct->snowmanActor.collisionType = ECT_Sphere;

    CalcCapsuleAABB(&snowmanStruct->snowmanActor);

    snowmanStruct->snowmanActor.model = t3d_model_load("rom:/snowmen/totalSnowmanTest_5.t3dm");
    snowmanStruct->snowmanActor.collisionModelPath = "rom:/snowmen/box.col";
    
    snowmanStruct->models[SNOWBALL0] = t3d_model_load("rom:/snowmen/snowman_snowball0.t3dm");
    snowmanStruct->models[SNOWBALL1] = t3d_model_load("rom:/snowmen/snowman_snowball1.t3dm");
    snowmanStruct->models[SNOWBALL2] = t3d_model_load("rom:/snowmen/snowman_snowball2.t3dm");

    switch (newOwner)
    {
    case EPID_1:
        snowmanStruct->models[HAT] = t3d_model_load("rom:/snowmen/snowman_hat.t3dm");
        snowmanStruct->modelBase = t3d_model_load("rom:/snowmen/snowman_face_pic_red.t3dm");
        break;
    case EPID_2:
        snowmanStruct->models[HAT] = t3d_model_load("rom:/snowmen/snowman_hat_blue.t3dm");
        snowmanStruct->modelBase = t3d_model_load("rom:/snowmen/snowman_face_pic_blue.t3dm");
        break;
    case EPID_3:
        snowmanStruct->models[HAT] = t3d_model_load("rom:/snowmen/snowman_hat_yellow.t3dm");
        snowmanStruct->modelBase = t3d_model_load("rom:/snowmen/snowman_face_pic_yellow.t3dm");
        break;
    case EPID_4:
        snowmanStruct->models[HAT] = t3d_model_load("rom:/snowmen/snowman_hat_green.t3dm");
        snowmanStruct->modelBase = t3d_model_load("rom:/snowmen/snowman_face_pic_green.t3dm");
        break;
    default:
        snowmanStruct->models[HAT] = t3d_model_load("rom:/snowmen/snowman_hat.t3dm");
        snowmanStruct->modelBase = t3d_model_load("rom:/snowmen/snowman_face_pic_red.t3dm");
        break;
    }
    
    snowmanStruct->models[CARROT] = t3d_model_load("rom:/snowmen/snowman_carrot.t3dm");
    snowmanStruct->models[FACE] = t3d_model_load("rom:/snowmen/snowman_face.t3dm");
    snowmanStruct->models[SCARF] = t3d_model_load("rom:/snowmen/snowman_scarf.t3dm");
    snowmanStruct->models[MITT] = t3d_model_load("rom:/snowmen/snowman_mitt.t3dm");
    snowmanStruct->models[STICK] = t3d_model_load("rom:/snowmen/snowman_stick.t3dm");
    snowmanStruct->models[STONESTORSO] = t3d_model_load("rom:/snowmen/snowman_stonestorso.t3dm");
    snowmanStruct->models[STONESBOTTOM] = t3d_model_load("rom:/snowmen/snowman_stonesbottom.t3dm");

    snowmanStruct->triggerStruct =(TriggerStruct){
    };
    CreateTriggerActor(&snowmanStruct->triggerStruct);
    TriggerActorInit(&snowmanStruct->triggerStruct);

    snowmanStruct->triggerStruct.TriggerActor.collisionRadius = 74.f;

    //Create Display List, only done once for each model.
    rspq_block_begin();
    t3d_matrix_push(snowmanStruct->snowmanActor.TransformFP);//push the transformation matrix we created, 
        //can alernatively push(1) and then set the mat you want t3d_matrix_set(,)
        rdpq_set_prim_color(RGBA32(255, 255, 255, 255));//SnowmanColors[snowmanStruct->snowmanOwner]);
        t3d_model_draw(snowmanStruct->snowmanActor.model);// Draw Static Mesh
        //t3d_model_draw_skinned(playerStruct->PlayerActor.model, &playerStruct->skel);// Draw skinned mesh with main skeleton.  
    t3d_matrix_pop(1);// must also pop it when done
    snowmanStruct->snowmanActor.dpl = rspq_block_end();

    rspq_block_begin();
    t3d_matrix_push(snowmanStruct->snowmanActor.TransformFP);
        rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
        t3d_model_draw(snowmanStruct->modelBase);
    t3d_matrix_pop(1);
    snowmanStruct->dplBase = rspq_block_end();

    color_t SnowmanColour;

    switch (snowmanStruct->snowmanOwner)
    {
        case EPID_1:
        SnowmanColour = RED;
        break;
        case EPID_2:
        SnowmanColour = BLUE;
        break;
        case EPID_3:
        SnowmanColour = YELLOW;
        break;
        case EPID_4:
        SnowmanColour = GREEN;
        break;

        default:
        SnowmanColour = color_from_packed32(0x000000<<8);
        break;
    }
    

    rspq_block_begin();
    t3d_matrix_push(snowmanStruct->snowmanActor.TransformFP);
        rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
        t3d_model_draw(snowmanStruct->models[SNOWBALL0]);
    t3d_matrix_pop(1);
    snowmanStruct->dpls[SNOWBALL0] = rspq_block_end();
    rspq_block_begin();
    t3d_matrix_push(snowmanStruct->HeadTransformFP);
        rdpq_set_prim_color(color_from_packed32(0xdb741a<<8));
        t3d_model_draw(snowmanStruct->models[CARROT]);
    t3d_matrix_pop(1);
    snowmanStruct->dpls[CARROT] = rspq_block_end();
    rspq_block_begin();
    t3d_matrix_push(snowmanStruct->HeadTransformFP);
        rdpq_set_prim_color(RGBA32(255, 255, 255, 255));//!!!!!!!!!!!!!!!!!!!!!!!
        t3d_model_draw(snowmanStruct->models[HAT]);
    t3d_matrix_pop(1);
    snowmanStruct->dpls[HAT] = rspq_block_end();
    rspq_block_begin();
    t3d_matrix_push(snowmanStruct->HeadTransformFP);
        rdpq_set_prim_color(SnowmanColour);
        t3d_model_draw(snowmanStruct->models[SCARF]);
    t3d_matrix_pop(1);
    snowmanStruct->dpls[SCARF] = rspq_block_end();
    rspq_block_begin();
    t3d_matrix_push(snowmanStruct->HeadTransformFP);
        rdpq_set_prim_color(RGBA32(0, 0, 0, 255));
        t3d_model_draw(snowmanStruct->models[FACE]);
    t3d_matrix_pop(1);
    snowmanStruct->dpls[FACE] = rspq_block_end();

    rspq_block_begin();
    t3d_matrix_push(snowmanStruct->snowmanActor.TransformFP);
        rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
        t3d_model_draw(snowmanStruct->models[SNOWBALL2]);
    t3d_matrix_pop(1);
    snowmanStruct->dpls[SNOWBALL2] = rspq_block_end();
    rspq_block_begin();
    t3d_matrix_push(snowmanStruct->snowmanActor.TransformFP);
        rdpq_set_prim_color(RGBA32(0, 0, 0, 255));
        t3d_model_draw(snowmanStruct->models[STONESBOTTOM]);
    t3d_matrix_pop(1);
    snowmanStruct->dpls[STONESBOTTOM] = rspq_block_end();
    rspq_block_begin();
    t3d_matrix_push(snowmanStruct->HeadTransformFP);
        rdpq_set_prim_color(RGBA32(0, 0, 0, 255));
        t3d_model_draw(snowmanStruct->models[STONESTORSO]);
    t3d_matrix_pop(1);
    snowmanStruct->dpls[STONESTORSO] = rspq_block_end();

    rspq_block_begin();
    t3d_matrix_push(snowmanStruct->StickTransformFP);
        rdpq_set_prim_color(SnowmanColour);
        t3d_model_draw(snowmanStruct->models[MITT]);
    t3d_matrix_pop(1);
    snowmanStruct->dpls[MITT] = rspq_block_end();
    rspq_block_begin();
    t3d_matrix_push(snowmanStruct->StickTransformFP);
        rdpq_set_prim_color(color_from_packed32(0x704022<<8));
        t3d_model_draw(snowmanStruct->models[STICK]);
    t3d_matrix_pop(1);
    snowmanStruct->dpls[STICK] = rspq_block_end();

    rspq_block_begin();
    t3d_matrix_push(snowmanStruct->snowmanActor.TransformFP);
        rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
        t3d_model_draw(snowmanStruct->models[SNOWBALL1]);
    t3d_matrix_pop(1);
    snowmanStruct->dpls[SNOWBALL1] = rspq_block_end();

    float scale = .4f;
    //ONLY TEMPORARY REALLY SNOWMAN SHOULD BE STATIC 
    t3d_mat4_from_srt_euler(&snowmanStruct->HeadTransform,
    (float[3]){1.f, 1.f, 1.f},
    (float[3]){0.f, 0.f, 0.f},
    (float[3]){snowmanStruct->snowmanActor.Position.v[0], snowmanStruct->snowmanActor.Position.v[1], snowmanStruct->snowmanActor.Position.v[2]}
    );
    t3d_mat4_to_fixed(snowmanStruct->HeadTransformFP, &snowmanStruct->HeadTransform);
        t3d_mat4_from_srt_euler(&snowmanStruct->HeadTransform,
    (float[3]){scale, scale, scale},
    (float[3]){0.f, snowmanStruct->height, 0.f},
    (float[3]){snowmanStruct->snowmanActor.Position.v[0], snowmanStruct->snowmanActor.Position.v[1], snowmanStruct->snowmanActor.Position.v[2]}
  );
  t3d_mat4_to_fixed(snowmanStruct->HeadTransformFP, &snowmanStruct->HeadTransform);
    t3d_mat4_from_srt_euler(&snowmanStruct->TorsoTransform,
    (float[3]){scale, scale, scale},
    (float[3]){0.f, snowmanStruct->height2nd, 0.f},
    (float[3]){snowmanStruct->snowmanActor.Position.v[0], snowmanStruct->snowmanActor.Position.v[1], snowmanStruct->snowmanActor.Position.v[2]}
  );
  t3d_mat4_to_fixed(snowmanStruct->TorsoTransformFP, &snowmanStruct->TorsoTransform);
    t3d_mat4_from_srt_euler(&snowmanStruct->StickTransform,
    (float[3]){scale, scale, scale},
    (float[3]){0.f, snowmanStruct->stickHeight, 0.f},
    (float[3]){snowmanStruct->snowmanActor.Position.v[0], snowmanStruct->snowmanActor.Position.v[1], snowmanStruct->snowmanActor.Position.v[2]}
  );
  t3d_mat4_to_fixed(snowmanStruct->StickTransformFP, &snowmanStruct->StickTransform);

    ActorInit(&snowmanStruct->snowmanActor);
}

void SnowmanLoop(struct SnowmanStruct* snowmanStruct, float deltatime)
{
    //float scale = .4f;
    CalcCapsuleAABB(&snowmanStruct->snowmanActor);

    snowmanStruct->height = 0.f;//on the floor, Head Transform
    snowmanStruct->height2nd = 0.f;//as normal, Torso Transform
    snowmanStruct->stickHeight = 0.f;//as normal, Stick Transform
    //Lastly, bottom snowball uses actors transform

    if(snowmanStruct->snowmanLevel == ESL_level0 || snowmanStruct->snowmanLevel == ESL_level1)
    {
        snowmanStruct->stickHeight = -25.f;
        snowmanStruct->height = -40.f;
    }
    else if (snowmanStruct->snowmanLevel == ESL_level2)
    {
        snowmanStruct->height = -25.f;
        snowmanStruct->stickHeight = -15.f;
        snowmanStruct->height2nd = -25.f;
    }
    else
    {
        //snowmanStruct->height = 60.f;
    }

    //=========== Update Model =============//
  /*t3d_mat4_from_srt_euler(&snowmanStruct->snowmanActor.Transform,
    (float[3]){scale, scale, scale},
    (float[3]){0.f, snowmanStruct->Rotation, 0.f},
    (float[3]){snowmanStruct->snowmanActor.Position.v[0], snowmanStruct->snowmanActor.Position.v[1], snowmanStruct->snowmanActor.Position.v[2]}
  );
  
  t3d_mat4_to_fixed(snowmanStruct->snowmanActor.TransformFP, &snowmanStruct->snowmanActor.Transform);


    t3d_mat4_from_srt_euler(&snowmanStruct->HeadTransform,
    (float[3]){scale, scale, scale},
    (float[3]){0.f, snowmanStruct->Rotation, 0.f},
    (float[3]){snowmanStruct->snowmanActor.Position.v[0], snowmanStruct->snowmanActor.Position.v[1] + snowmanStruct->height, snowmanStruct->snowmanActor.Position.v[2]}
  );
  t3d_mat4_to_fixed(snowmanStruct->HeadTransformFP, &snowmanStruct->HeadTransform);
    t3d_mat4_from_srt_euler(&snowmanStruct->TorsoTransform,
    (float[3]){scale, scale, scale},
    (float[3]){0.f, snowmanStruct->Rotation, 0.f},
    (float[3]){snowmanStruct->snowmanActor.Position.v[0], snowmanStruct->snowmanActor.Position.v[1] + snowmanStruct->height2nd, snowmanStruct->snowmanActor.Position.v[2]}
  );
  t3d_mat4_to_fixed(snowmanStruct->TorsoTransformFP, &snowmanStruct->TorsoTransform);
    t3d_mat4_from_srt_euler(&snowmanStruct->StickTransform,
    (float[3]){scale, scale, scale},
    (float[3]){0.f, snowmanStruct->Rotation, 0.f},
    (float[3]){snowmanStruct->snowmanActor.Position.v[0], snowmanStruct->snowmanActor.Position.v[1] + snowmanStruct->stickHeight, snowmanStruct->snowmanActor.Position.v[2]}
  );
  t3d_mat4_to_fixed(snowmanStruct->StickTransformFP, &snowmanStruct->StickTransform);*/

  snowmanStruct->triggerStruct.TriggerActor.Position = snowmanStruct->snowmanActor.Position;
  
  //snowmanStruct->snowmanActor.BillboardPosition = snowmanStruct->snowmanActor.Position;
}

void SnowmanUpdateModel(struct SnowmanStruct* snowmanStruct)
{
    float scale = .4f;
        //=========== Update Model =============//
  t3d_mat4_from_srt_euler(&snowmanStruct->snowmanActor.Transform,
    (float[3]){scale, scale, scale},
    (float[3]){0.f, snowmanStruct->Rotation, 0.f},
    (float[3]){snowmanStruct->snowmanActor.Position.v[0], snowmanStruct->snowmanActor.Position.v[1], snowmanStruct->snowmanActor.Position.v[2]}
  );
  
  t3d_mat4_to_fixed(snowmanStruct->snowmanActor.TransformFP, &snowmanStruct->snowmanActor.Transform);


    t3d_mat4_from_srt_euler(&snowmanStruct->HeadTransform,
    (float[3]){scale, scale, scale},
    (float[3]){0.f, snowmanStruct->Rotation, 0.f},
    (float[3]){snowmanStruct->snowmanActor.Position.v[0], snowmanStruct->snowmanActor.Position.v[1] + snowmanStruct->height, snowmanStruct->snowmanActor.Position.v[2]}
  );
  t3d_mat4_to_fixed(snowmanStruct->HeadTransformFP, &snowmanStruct->HeadTransform);
    t3d_mat4_from_srt_euler(&snowmanStruct->TorsoTransform,
    (float[3]){scale, scale, scale},
    (float[3]){0.f, snowmanStruct->Rotation, 0.f},
    (float[3]){snowmanStruct->snowmanActor.Position.v[0], snowmanStruct->snowmanActor.Position.v[1] + snowmanStruct->height2nd, snowmanStruct->snowmanActor.Position.v[2]}
  );
  t3d_mat4_to_fixed(snowmanStruct->TorsoTransformFP, &snowmanStruct->TorsoTransform);
    t3d_mat4_from_srt_euler(&snowmanStruct->StickTransform,
    (float[3]){scale, scale, scale},
    (float[3]){0.f, snowmanStruct->Rotation, 0.f},
    (float[3]){snowmanStruct->snowmanActor.Position.v[0], snowmanStruct->snowmanActor.Position.v[1] + snowmanStruct->stickHeight, snowmanStruct->snowmanActor.Position.v[2]}
  );
  t3d_mat4_to_fixed(snowmanStruct->StickTransformFP, &snowmanStruct->StickTransform);
}

bool SnowmanAttemptAdd(struct SnowmanStruct* snowmanStruct, struct PlayerStruct* playerStruct, struct PickupStruct* pickupStruct, bool* GameEnd)
{
    debugf("&&&&&&&&&&&&&&&&&& Player ID: %d\n", snowmanStruct->snowmanOwner);
    debugf("&&&&&&&&&&&&&&&&&& Snowman ID: %d\n", playerStruct->playerId);
    if (snowmanStruct->snowmanOwner != playerStruct->playerId) 
    {
        debugf("not my snowman!!!\n");
        return false;
    }
    
    debugf("STARTING snowman decorations: %d\n", snowmanStruct->decorations);
    debugf("STARTING snowman level: %d\n", snowmanStruct->snowmanLevel);
    
    if (pickupStruct->pickupType == EPUT_Snowball)
    {
        if (snowmanStruct->snowmanLevel != ESL_level3)
        {
            PlayerDrops(pickupStruct, playerStruct);
            PickupDelete(pickupStruct);
            snowmanStruct->snowmanLevel++;
            debugf("Added snowball!!! Snowman level: %d\n", snowmanStruct->snowmanLevel);
            
            snowmanStruct->BillboardValue = 2;
            snowmanStruct->snowmanActor.BillboardTimer = 1.5f;
            snowmanStruct->snowmanActor.BillboardPosition = snowmanStruct->snowmanActor.Position;
            if (snowmanStruct->decorations == 63 && snowmanStruct->snowmanLevel == 3)
            {
                //All decorations have been added
                debugf("Snowman completed for player %d!\n", snowmanStruct->snowmanOwner);
                *GameEnd = true;
            }
            wav64_play(&snowmanStruct->sfx_collect, 30);
            return true;
        }
        else
        {
            //Already at 3 snowballs, inform player?
            debugf("Snowman level Unchanged\n");
            return false;
        }
    }
    else//is Decoration:
    {
        if (snowmanStruct->snowmanLevel == ESL_level0)
        {
            debugf("Snowman level 0, cannot add decorations yet.\n");
            return false;
        }
        int typeToAdd = 0 | (1 << (pickupStruct->decorationType - 1));//no need
        debugf("Decoration to add: %d\n", typeToAdd);
        if (pickupStruct->decorationType - 1 >= 0)
        {
            if(!(snowmanStruct->decorations & (1 << (pickupStruct->decorationType - 1))))
            {
                snowmanStruct->decorations = snowmanStruct->decorations | (1 << (pickupStruct->decorationType - 1));
                PlayerDrops(pickupStruct, playerStruct);
                PickupDelete(pickupStruct);
                debugf("Added Decoration: %d\n", (snowmanStruct->decorations & (1 << (pickupStruct->decorationType - 1))));
                snowmanStruct->BillboardValue = 1;
                snowmanStruct->snowmanActor.BillboardTimer = 1.5f;
                snowmanStruct->snowmanActor.BillboardPosition = snowmanStruct->snowmanActor.Position;
                if (snowmanStruct->decorations == 63 && snowmanStruct->snowmanLevel == 3)
                {
                    //All decorations have been added
                    debugf("Snowman completed for player %d!\n", snowmanStruct->snowmanOwner);
                    *GameEnd = true;
                }
                wav64_play(&snowmanStruct->sfx_collect, 30);
                return true;
            }
            else
            {
                debugf("Already Added\n");
                return false;
            }
        }
        else
        {
            debugf("invalid type\n");
            return false;
        }
        debugf("Current Decorations: %d\n", snowmanStruct->decorations);
    }





    /*if (snowmanStruct->snowmanState == ESS_Empty)
    {
        if (pickupStruct->pickupType == EPUT_Snowball)
        {
            snowmanStruct->snowmanState = ESS_SnowballOnly;
            PlayerDrops(pickupStruct, playerStruct);
            PickupDelete(pickupStruct);
        }
        else if(pickupStruct->pickupType == EPUT_Decoration)
        {
            snowmanStruct->snowmanState = ESS_DecorationsOnly;
            PlayerDrops(pickupStruct, playerStruct);
            PickupDelete(pickupStruct);
        }
        debugf("snowman state: %d\n", snowmanStruct->snowmanState);
    }
    //if snowmanState != same state as pickup struct and is not empty, state=empty and level increase
    else if ((snowmanStruct->snowmanState == ESS_SnowballOnly && pickupStruct->pickupType == EPUT_Decoration)
        || (snowmanStruct->snowmanState == ESS_DecorationsOnly && pickupStruct->pickupType == EPUT_Snowball))
    {
        //  if level is already level 3, don't increase, end the game as we've won
        if (snowmanStruct->snowmanLevel == ESL_level3)
        {
            debugf("End game, WINNER!\n");
        }
        else
        {
            snowmanStruct->snowmanLevel++;
            snowmanStruct->snowmanState = ESS_Empty;
        }
        PlayerDrops(pickupStruct, playerStruct);
        PickupDelete(pickupStruct);
        debugf("snowman state: %d\n", snowmanStruct->snowmanState);
        debugf("snowman level: %d\n", snowmanStruct->snowmanLevel);
    }*/

}

void SnowmanDraw(struct SnowmanStruct* snowmanStruct)
{
    
    //rdpq_mode_antialias(AA_STANDARD);
    if(snowmanStruct->snowmanLevel == ESL_level0)
    {
        rspq_block_run(snowmanStruct->dplBase);
    }
    else if(snowmanStruct->snowmanLevel == ESL_level1)
    {
        //rspq_block_run(snowmanStruct->dplBase);
        rspq_block_run(snowmanStruct->dpls[SNOWBALL0]);
        if((snowmanStruct->decorations & (1 << 0)))
        {
            rspq_block_run(snowmanStruct->dpls[FACE]);
        }
    }
    else if (snowmanStruct->snowmanLevel == ESL_level2)
    {
        //rspq_block_run(snowmanStruct->dpls[SNOWBALL0]);
         rspq_block_run(snowmanStruct->dpls[SNOWBALL1]);
        if((snowmanStruct->decorations & (1 << 0)))
        {
            rspq_block_run(snowmanStruct->dpls[STONESTORSO]);
        }
    }
    else if(snowmanStruct->snowmanLevel == ESL_level3)
    {
        //rspq_block_run(snowmanStruct->dpls[SNOWBALL0]);
        //rspq_block_run(snowmanStruct->dpls[SNOWBALL1]);
       rspq_block_run(snowmanStruct->dpls[SNOWBALL2]);
       if((snowmanStruct->decorations & (1 << 0)))
        {
            rspq_block_run(snowmanStruct->dpls[STONESBOTTOM]);
        }
    }


    if((snowmanStruct->decorations & (1 << 1)))
    {
        rspq_block_run(snowmanStruct->dpls[CARROT]);
    }
    if((snowmanStruct->decorations & (1 << 2)))
    {
        rspq_block_run(snowmanStruct->dpls[MITT]);
    }
    if((snowmanStruct->decorations & (1 << 3)))
    {
        rspq_block_run(snowmanStruct->dpls[HAT]);
    }
    if((snowmanStruct->decorations & (1 << 4)))
    {
        rspq_block_run(snowmanStruct->dpls[SCARF]);
    }   
    if((snowmanStruct->decorations & (1 << 5)))
    {
        rspq_block_run(snowmanStruct->dpls[STICK]);
    }
       
    //rdpq_mode_antialias(AA_STANDARD);
    
}

void SnowmanDrawAll(struct SnowmanStruct* snowmanStruct)
{
    
    //rdpq_mode_antialias(AA_STANDARD);

        rspq_block_run(snowmanStruct->dplBase);

        rspq_block_run(snowmanStruct->dpls[SNOWBALL0]);

            rspq_block_run(snowmanStruct->dpls[FACE]);

         rspq_block_run(snowmanStruct->dpls[SNOWBALL1]);

            rspq_block_run(snowmanStruct->dpls[STONESTORSO]);

       rspq_block_run(snowmanStruct->dpls[SNOWBALL2]);

            rspq_block_run(snowmanStruct->dpls[STONESBOTTOM]);
        

        rspq_block_run(snowmanStruct->dpls[CARROT]);
    

        rspq_block_run(snowmanStruct->dpls[MITT]);
    
        rspq_block_run(snowmanStruct->dpls[HAT]);

    
        rspq_block_run(snowmanStruct->dpls[SCARF]);

    
        rspq_block_run(snowmanStruct->dpls[STICK]);
    
       
    //rdpq_mode_antialias(AA_STANDARD);
    
}

void SnowmanFree(struct SnowmanStruct* snowmanStruct)
{
    ActorFree(&snowmanStruct->snowmanActor);
    TriggerFree(&snowmanStruct->triggerStruct);

    free_uncached(snowmanStruct->HeadTransformFP);
    free_uncached(snowmanStruct->TorsoTransformFP);
    free_uncached(snowmanStruct->StickTransformFP);

    for(int i = 0; i < 11; i++)
    {
        //T3DModel* models[11];
        t3d_model_free(snowmanStruct->models[i]);
        //rspq_block_t* dpls[11];
        rspq_block_free(snowmanStruct->dpls[i]);
    }

    t3d_model_free(snowmanStruct->modelBase);
    rspq_block_free(snowmanStruct->dplBase);

    wav64_close(&snowmanStruct->sfx_collect);
}