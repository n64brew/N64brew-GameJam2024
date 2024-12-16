#include "triggerActor.h"


void CreateTriggerActor(TriggerStruct* triggerStruct)
{
    T3DMat4 modelMat;
    t3d_mat4_from_srt_euler(&modelMat,
        (float[3]){1.f, 1.f, 1.f},
        (float[3]){0.f, 0.f, 0.f},
        (float[3]){0.f, 0.f, 0.f}
    );

    *triggerStruct = (TriggerStruct){
        //.snowmanLevel = ESS_Empty,
        //.snowmanState = ESL_level1
        //enum ESnowmanOwner snowmanOwner;
        //Actor snowmanActor;
    };

    triggerStruct->TriggerActor = (Actor){
    //.model = ,
    .actorType = EAT_TRIGGER,
    .hasCollision = true,
    .isDynamic = false,
    //.initFunc = PlayerInit,
    //.loopFunc = PlayerLoop,
    .collisionType = ECT_Mesh,
    .collisionRadius = 30.f,
    .collisionCenter = (T3DVec3){{0.f, 0.f, 0.f}},
    .CollisionHeight = 50.f,
    .CurrentVelocity = (T3DVec3){{0.f, 0.f, 0.f}},
    .DesiredMovement = (T3DVec3){{0.f, 0.f, 0.f}},
    .Position = (T3DVec3){{-100.f, 0.f, -100.f}},
    .PrevPosition = (T3DVec3){{0.f, 0.f, 0.f}},
    .Transform = modelMat
    //T3DMat4FP TransformFP
    //rspq_block_t *dpl
    };
}

void TriggerActorInit(struct TriggerStruct* triggerStruct)
{
    /*snowmanStruct->snowmanOwner = snowmanOwner;
    snowmanStruct->snowmanLevel = ESL_level1;
    snowmanStruct->snowmanState = ESS_Empty;*/

    triggerStruct->TriggerActor.TransformFP = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_identity(triggerStruct->TriggerActor.TransformFP);

    triggerStruct->TriggerActor.collisionType = ECT_Sphere;

    CalcCapsuleAABB(&triggerStruct->TriggerActor);

    /*snowmanStruct->snowmanActor.model = t3d_model_load("rom:/mygame/box.t3dm");
    snowmanStruct->snowmanActor.collisionModelPath = "rom:/mygame/box.col";


    //Create Display List, only done once for each model.
    rspq_block_begin();
    t3d_matrix_push(snowmanStruct->snowmanActor.TransformFP);//push the transformation matrix we created, 
        //can alernatively push(1) and then set the mat you want t3d_matrix_set(,)
        rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
        t3d_model_draw(snowmanStruct->snowmanActor.model);// Draw Static Mesh
        //t3d_model_draw_skinned(playerStruct->PlayerActor.model, &playerStruct->skel);// Draw skinned mesh with main skeleton.  
    t3d_matrix_pop(1);// must also pop it when done
    snowmanStruct->snowmanActor.dpl = rspq_block_end();

    //ONLY TEMPORARY REALLY SNOWMAN SHOULD BE STATIC 
    t3d_mat4_from_srt_euler(&snowmanStruct->snowmanActor.Transform,
    (float[3]){1.f, 1.f, 1.f},
    (float[3]){0.f, 0.f, 0.f},
    (float[3]){snowmanStruct->snowmanActor.Position.v[0], snowmanStruct->snowmanActor.Position.v[1], snowmanStruct->snowmanActor.Position.v[2]}
    );
    t3d_mat4_to_fixed(snowmanStruct->snowmanActor.TransformFP, &snowmanStruct->snowmanActor.Transform);*/

    ActorInit(&triggerStruct->TriggerActor);
}

void TriggerActorLoop(struct TriggerStruct* triggerStruct, float deltaTime)
{
    CalcCapsuleAABB(&triggerStruct->TriggerActor);
    //triggerStruct->TriggerActor.Position = 
    //=========== Update Model =============//
  /*t3d_mat4_from_srt_euler(&triggerStruct->TriggerActor.Transform,
    (float[3]){1.f, 1.f, 1.f},
    (float[3]){0.f, 0.f, 0.f},
    (float[3]){triggerStruct->TriggerActor.Position.v[0], triggerStruct->TriggerActor.Position.v[1], triggerStruct->TriggerActor.Position.v[2]}
  );*/
  
  /*t3d_mat4_to_fixed(triggerStruct->TriggerActor.TransformFP, &triggerStruct->TriggerActor.Transform);*/
}



void TriggerFree(TriggerStruct* triggerStruct)
{
    ActorFree(&triggerStruct->TriggerActor);
}