#include "DecorationSpawner.h"


void CreateSpawner(struct DecorationSpawnerStruct* decorationSpawner)
{
    T3DMat4 modelMat;
    t3d_mat4_from_srt_euler(&modelMat,
        (float[3]){1.f, 1.f, 1.f},
        (float[3]){0.f, 0.f, 0.f},
        (float[3]){0.f, 0.f, 0.f}
    );

    *decorationSpawner = (DecorationSpawnerStruct){
        //.pickupState = EPUS_Idle
        //.pickupType,
        //.pickupActor,
        //.holdingPlayerStruct
    };

    decorationSpawner->spawnerActor = (Actor){
    //.model = ,
    .actorType = EAT_SPAWNER,
    .hasCollision = true,
    .isDynamic = false,
    //.initFunc = PlayerInit,
    //.loopFunc = PlayerLoop,
    .collisionType = ECT_Sphere,
    .collisionRadius = 10.f,
    .collisionCenter = (T3DVec3){{0.f, 0.f, 0.f}},
    .CollisionHeight = 50.f,
    .CurrentVelocity = (T3DVec3){{0.f, 0.f, 0.f}},
    .DesiredMovement = (T3DVec3){{0.f, 0.f, 0.f}},
    .Position = (T3DVec3){{100.f, 0.f, 100.f}},
    .PrevPosition = (T3DVec3){{0.f, 0.f, 0.f}},
    .Transform = modelMat
    //T3DMat4FP TransformFP
    //rspq_block_t *dpl
  };

decorationSpawner->numInactive = 2;

}

void SpawnerInit(struct DecorationSpawnerStruct* decorationSpawner, enum EDecorationType decorationType)
{
    /*pickupStruct->pickupType = type;
    pickupStruct->snowballSize = 5.f;
    pickupStruct->maxSnowballSize = 10.f;
    pickupStruct->holdingPlayerStruct = NULL;*/

    decorationSpawner->decorationType = decorationType;

    struct PickupStruct* decorations[2];

    for(int i = 0; i < 2; i++)
    {
        CreatePickup(&decorationSpawner->decorations[i]);
        PickupInit(&decorationSpawner->decorations[i], decorationType);
        //PickupSetType(&decorationSpawner->decorations[i], decorationType);

        PickupDeactivate(&decorationSpawner->decorations[i]);
    }




    decorationSpawner->spawnerActor.TransformFP = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_identity(decorationSpawner->spawnerActor.TransformFP);

    decorationSpawner->spawnerActor.collisionType = ECT_Sphere;

    CalcCapsuleAABB(&decorationSpawner->spawnerActor);

    //decorationSpawner->spawnerActor.model = t3d_model_load("rom:/mygame/box.t3dm");
    decorationSpawner->spawnerTrigger =(TriggerStruct){

    };
    CreateTriggerActor(&decorationSpawner->spawnerTrigger);
    TriggerActorInit(&decorationSpawner->spawnerTrigger);
    decorationSpawner->spawnerTrigger.TriggerActor.collisionRadius = 50.f;
    //decorationSpawner->decorationType = EDT_Decoration1;

        //////////////////////////////////////
    color_t color = color_from_packed32(0x000000<<8);

    switch (decorationSpawner->decorations[0].decorationType)
    {
    case EDT_Decoration1:
        decorationSpawner->spawnerActor.model = t3d_model_load("rom:/snowmen/spawner_stones.t3dm");
        color = color_from_packed32(0x4a4a4a<<8);
        break;
    case EDT_Decoration2:
        decorationSpawner->spawnerActor.model = t3d_model_load("rom:/snowmen/spawner_carrot.t3dm");
        color = color_from_packed32(0xdb741a<<8);
        break;
    case EDT_Decoration3:
        decorationSpawner->spawnerActor.model = t3d_model_load("rom:/snowmen/spawner_mitt.t3dm");
        color = color_from_packed32(0xc75292<<8);
        break;
    case EDT_Decoration4:
        decorationSpawner->spawnerActor.model = t3d_model_load("rom:/snowmen/spawner_hat.t3dm");
        color = color_from_packed32(0xffffff<<8);
        break;
    case EDT_Decoration5:
        decorationSpawner->spawnerActor.model = t3d_model_load("rom:/snowmen/spawner_scarf.t3dm");
        color = color_from_packed32(0xb02e20<<8);
        break;
    case EDT_Decoration6:
        decorationSpawner->spawnerActor.model = t3d_model_load("rom:/snowmen/spawner_sticks.t3dm");
        color = color_from_packed32(0x704022<<8);
        break;
    
    default:
        decorationSpawner->spawnerActor.model = t3d_model_load("rom:/snowmen/spawner_stones.t3dm");
        color = color_from_packed32(0x4a4a4a<<8);
        break;
    }

        rspq_block_begin();
        t3d_matrix_push(decorationSpawner->spawnerActor.TransformFP);//push the transformation matrix we created, 
        //can alernatively push(1) and then set the mat you want t3d_matrix_set(,)
        rdpq_set_prim_color(color);
        t3d_model_draw(decorationSpawner->spawnerActor.model);// Draw Static Mesh
        //t3d_model_draw_skinned(playerStruct->PlayerActor.model, &playerStruct->skel);// Draw skinned mesh with main skeleton.  
    t3d_matrix_pop(1);// must also pop it when done
    decorationSpawner->spawnerActor.dpl = rspq_block_end();
    //////////////////////////////////
    decorationSpawner->modelBase = t3d_model_load("rom:/snowmen/base_star.t3dm");

    rspq_block_begin();
    t3d_matrix_push(decorationSpawner->spawnerActor.TransformFP);
        rdpq_set_prim_color(RGBA32(216, 153, 255, 150));
        t3d_model_draw(decorationSpawner->modelBase);
    t3d_matrix_pop(1);
    decorationSpawner->dplBase = rspq_block_end();


    //Create Display List, only done once for each model.
    /*rspq_block_begin();
    t3d_matrix_push(decorationSpawner->spawnerActor.TransformFP);//push the transformation matrix we created, 
        //can alernatively push(1) and then set the mat you want t3d_matrix_set(,)
            rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
        t3d_model_draw(decorationSpawner->spawnerActor.model);// Draw Static Mesh
        //t3d_model_draw_skinned(playerStruct->PlayerActor.model, &playerStruct->skel);// Draw skinned mesh with main skeleton.  
    t3d_matrix_pop(1);// must also pop it when done
    decorationSpawner->spawnerActor.dpl = rspq_block_end();*/


}

void SpawnerLoop(struct DecorationSpawnerStruct* decorationSpawner, Camera* camera, float deltaTime, T3DViewport* viewport)
{
    //float scale = 0.5f;
    int tempInactive = 0;
    for (int i = 0; i < 2; i++)
    {
        PickupLoop(&decorationSpawner->decorations[i], camera, deltaTime, viewport);
        if (decorationSpawner->decorations[i].pickupState == EPUS_Inactive) tempInactive++;
    }
    if (tempInactive != decorationSpawner->numInactive)
    {
        decorationSpawner->numInactive = tempInactive;
        decorationSpawner->spawnerActor.BillboardTimer = 1.5f;
        decorationSpawner->spawnerActor.BillboardPosition = decorationSpawner->spawnerActor.Position;
    }


      //=========== Update Model =============//
  /*t3d_mat4_from_srt_euler(&decorationSpawner->spawnerActor.Transform,
    (float[3]){scale, scale, scale},
    (float[3]){0.0f, 0.0f, 0.0f},
    (float[3]){decorationSpawner->spawnerActor.Position.v[0], decorationSpawner->spawnerActor.Position.v[1], decorationSpawner->spawnerActor.Position.v[2]}
  );
  t3d_mat4_to_fixed(decorationSpawner->spawnerActor.TransformFP, &decorationSpawner->spawnerActor.Transform);*/

  decorationSpawner->spawnerTrigger.TriggerActor.Position = decorationSpawner->spawnerActor.Position;
}

void SpawnerUpdateModel(struct DecorationSpawnerStruct* decorationSpawner)
{
    float scale = 0.5f;
    //=========== Update Model =============//
  t3d_mat4_from_srt_euler(&decorationSpawner->spawnerActor.Transform,
    (float[3]){scale, scale, scale},
    (float[3]){0.0f, 0.0f, 0.0f},
    (float[3]){decorationSpawner->spawnerActor.Position.v[0], decorationSpawner->spawnerActor.Position.v[1], decorationSpawner->spawnerActor.Position.v[2]}
  );
  t3d_mat4_to_fixed(decorationSpawner->spawnerActor.TransformFP, &decorationSpawner->spawnerActor.Transform);
}


void SpawnDecoration(struct DecorationSpawnerStruct* decorationSpawner, struct PlayerStruct* player)
{
    //check if any inactive decorations for this spawner available
    for (int i = 0; i < 2; i++)
    {
        debugf("Decoration #%d has state: %d\n", i, decorationSpawner->decorations[i].pickupState);
        if (decorationSpawner->decorations[i].pickupState == EPUS_Inactive)
        {
            //Also check if player isn't already holding something
            debugf("Again, Decoration #%d is player NOT holding this: %d\n", i, player->heldPickup == NULL);
            if (player->heldPickup == NULL)
            {
                //if so, activate decoration and attach to this player
                DecorationActivate(&decorationSpawner->decorations[i]);
                //decorationSpawner->decorations[i].holdingPlayerStruct = player;
                //player->heldPickup = &decorationSpawner->decorations[i];
                PlayerPicksUp(&decorationSpawner->decorations[i], player);

                return;
            }
        }
    }


}


void DecorationDeactivate(struct PickupStruct* pickupStruct)
{
    if (pickupStruct->holdingPlayerStruct != NULL)
    {
        pickupStruct->holdingPlayerStruct->heldPickup = NULL;
        pickupStruct->holdingPlayerStruct = NULL;
    }
    pickupStruct->pickupActor.Position = (T3DVec3) {{0.f, -1000.f, 0.f}};
    pickupStruct->pickupState = EPUS_Inactive;
}

void DecorationActivate(struct PickupStruct* pickupStruct)
{
    pickupStruct->pickupState = EPUS_Idle;

}

void SpawnerFree(struct DecorationSpawnerStruct* decorationSpawner)
{
    PickupFree(&decorationSpawner->decorations[0]);
  PickupFree(&decorationSpawner->decorations[1]);

  ActorFree(&decorationSpawner->spawnerActor);

  t3d_model_free(decorationSpawner->modelBase);
  rspq_block_free(decorationSpawner->dplBase);

  TriggerFree(&decorationSpawner->spawnerTrigger);


}
