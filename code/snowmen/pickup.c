#include "pickup.h"

void CreatePickup(struct PickupStruct* pickupStruct)
{
    T3DMat4 modelMat;
    t3d_mat4_from_srt_euler(&modelMat,
        (float[3]){1.f, 1.f, 1.f},
        (float[3]){0.f, 0.f, 0.f},
        (float[3]){0.f, 0.f, 0.f}
    );

    *pickupStruct = (PickupStruct){
        .pickupState = EPUS_Idle
        //.pickupType,
        //.pickupActor,
        //.holdingPlayerStruct
    };

    pickupStruct->pickupActor = (Actor){
    //.model = ,
    .actorType = EAT_Pickup,
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

pickupStruct->Rotation = (T3DVec3) {{0.f, 0.f, 0.f}};
  pickupStruct->snowballSize = 5.f;
    pickupStruct->maxSnowballSize = 10.f;
}

void PickupInit(struct PickupStruct* pickupStruct, enum EDecorationType decoType)
{
    if (decoType == EDT_Empty)
    {
        pickupStruct->pickupType = EPUT_Snowball;
    }
    else
    {
        pickupStruct->pickupType = EPUT_Decoration;
    }
    pickupStruct->snowballSize = 5.f;
    pickupStruct->maxSnowballSize = 10.f;
    pickupStruct->holdingPlayerStruct = NULL;
    pickupStruct->Rotation = (T3DVec3) {{0.f, 0.f, 0.f}};

    pickupStruct->pickupActor.TransformFP = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_identity(pickupStruct->pickupActor.TransformFP);

    pickupStruct->pickupActor.collisionType = ECT_Sphere;

    CalcCapsuleAABB(&pickupStruct->pickupActor);
        
        pickupStruct->triggerStruct =(TriggerStruct){

        };
        CreateTriggerActor(&pickupStruct->triggerStruct);
        TriggerActorInit(&pickupStruct->triggerStruct);
        


    /*if (pickupStruct->pickupType == EPUT_Decoration)
    {
        pickupStruct->decorationType = EDT_Decoration1;
        //pickupStruct->pickupActor.model = t3d_model_load("rom:/snowmen/box.t3dm");
    }
    else
    {
        pickupStruct->pickupActor.model = t3d_model_load("rom:/snowmen/pickup_snowball.t3dm");
    }*/
   PickupSetType(pickupStruct, decoType);

    //Create Display List, only done once for each model.
    /*rspq_block_begin();
    t3d_matrix_push(pickupStruct->pickupActor.TransformFP);//push the transformation matrix we created, 
        //can alernatively push(1) and then set the mat you want t3d_matrix_set(,)
        if (pickupStruct->pickupType == EPUT_Decoration)
        {
            rdpq_set_prim_color(RGBA32(0, 255, 0, 255));
        }
        else
        {
            rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
        }

        t3d_model_draw(pickupStruct->pickupActor.model);// Draw Static Mesh
        //t3d_model_draw_skinned(playerStruct->PlayerActor.model, &playerStruct->skel);// Draw skinned mesh with main skeleton.  
    t3d_matrix_pop(1);// must also pop it when done
    pickupStruct->pickupActor.dpl = rspq_block_end();*/

    PickupDeactivate(pickupStruct);
}

void SnowballSwapColours(struct PickupStruct* pickupStruct, color_t ballColor)
{
    /*rspq_block_begin();
    t3d_matrix_push(pickupStruct->pickupActor.TransformFP);
        rdpq_set_prim_color(ballColor);
    t3d_model_draw(pickupStruct->pickupActor.model);// Draw Static Mesh
    t3d_matrix_pop(1);// must also pop it when done
    pickupStruct->pickupActor.dpl = rspq_block_end();*/
}


/*void awful_look(T3DMat4 *out, const T3DVec3 *eye, const T3DVec3 *dir, const T3DVec3 *up)
{
    T3DVec3 s, u;

    t3d_vec3_cross(&s, dir, up);
    t3d_vec3_norm(&s);

    t3d_vec3_cross(&u, &s, dir);

    *out = (T3DMat4){{
        {s.v[0], u.v[0], -dir->v[0], 0},
        {s.v[1], u.v[1], -dir->v[1], 0},
        {s.v[2], u.v[2], -dir->v[2], 0},
        {-t3d_vec3_dot(&s, eye), -t3d_vec3_dot(&u, eye), t3d_vec3_dot(dir, eye), 1},
    }};
}
void awful_lookat(T3DMat4 *out, const T3DVec3 *eye, const T3DVec3 *target, const T3DVec3 *up)
{
    T3DVec3 dir;
    t3d_vec3_diff(&dir, target, eye);
    t3d_vec3_norm(&dir);
    awful_look(out, eye, &dir, up);
}*/


void PickupLoop(struct PickupStruct* pickupStruct, Camera* camera, float deltaTime, T3DViewport* viewport)
{
    pickupStruct->scale = 0.3f;
    if (pickupStruct->pickupState == EPUS_PickedUp)
    {
        if (pickupStruct->pickupType == EPUT_Decoration)
        {
            if (t3d_vec3_len2(&pickupStruct->holdingPlayerStruct->PlayerActor.CurrentVelocity) >= 0.1f)
            {
                pickupStruct->pickupActor.Position = pickupStruct->holdingPlayerStruct->PlayerActor.Position;
                T3DVec3 NewDirection = {{pickupStruct->holdingPlayerStruct->PlayerActor.CurrentVelocity.v[0], 0.f, pickupStruct->holdingPlayerStruct->PlayerActor.CurrentVelocity.v[2]}};
                fast_vec3_norm(&NewDirection);
                pickupStruct->pickupActor.Position.v[0] += NewDirection.v[0] * 15.f;
                pickupStruct->pickupActor.Position.v[2] += NewDirection.v[2] * 15.f;
                //pickupStruct->pickupActor.Position.v[1]+=20.f;
                //debugf("")
            }
        }
        else
        {
            float distTravelled = t3d_vec3_distance(&pickupStruct->prevPosition, &pickupStruct->holdingPlayerStruct->PlayerActor.Position);
            //debugf("Dist Travelled: %f\n", distTravelled);

            pickupStruct->snowballSize += (distTravelled / 50);
            if (pickupStruct->snowballSize >= pickupStruct->maxSnowballSize)
            {
                pickupStruct->snowballSize = pickupStruct->maxSnowballSize;
            }
            pickupStruct->scale *= pickupStruct->snowballSize / pickupStruct->maxSnowballSize;

            if (t3d_vec3_len2(&pickupStruct->holdingPlayerStruct->PlayerActor.CurrentVelocity) >= 0.1f)
            {
                pickupStruct->pickupActor.Position = pickupStruct->holdingPlayerStruct->PlayerActor.Position;

            //pickupStruct->pickupActor.Position.v[0]+=40.f;//should be in player direction
            //Maybe should directly use angle for this part:


            //debugf("Prev Position: %f, %f, %f\n", pickupStruct->prevPosition.v[0], pickupStruct->prevPosition.v[1], pickupStruct->prevPosition.v[2]);
            //debugf("New Position: %f, %f, %f\n", pickupStruct->pickupActor.Position.v[0], pickupStruct->pickupActor.Position.v[1], pickupStruct->pickupActor.Position.v[2]);


            
                T3DVec3 NewDirection = {{pickupStruct->holdingPlayerStruct->PlayerActor.CurrentVelocity.v[0], 0.f, pickupStruct->holdingPlayerStruct->PlayerActor.CurrentVelocity.v[2]}};
                fast_vec3_norm(&NewDirection);
                pickupStruct->pickupActor.Position.v[0] += NewDirection.v[0] * 30.f * pickupStruct->snowballSize / pickupStruct->maxSnowballSize;
                pickupStruct->pickupActor.Position.v[2] += NewDirection.v[2] * 30.f * pickupStruct->snowballSize / pickupStruct->maxSnowballSize;
                pickupStruct->pickupActor.collisionRadius = 10.f * pickupStruct->snowballSize / pickupStruct->maxSnowballSize;
                pickupStruct->pickupActor.Position.v[1]-= 10.f * pickupStruct->snowballSize / pickupStruct->maxSnowballSize;
                pickupStruct->Rotation.v[2] += NewDirection.v[0] * (distTravelled / 50);
                pickupStruct->Rotation.v[0] -= NewDirection.v[2] * (distTravelled / 50);
            }


            pickupStruct->prevPosition = pickupStruct->holdingPlayerStruct->PlayerActor.Position;//pickupStruct->pickupActor.Position;
        }
    }
    else
    {
        if (pickupStruct->pickupType == EPUT_Snowball)
        {
            pickupStruct->scale *= pickupStruct->snowballSize / pickupStruct->maxSnowballSize;
        }
    }
      //=========== Here lies my hopes and dreams =============//



        //awful_lookat(&tempMat4, &pickupStruct->pickupActor.Position, &camera->camPos, &camera->cameraUp);
        //t3d_mat4_look_at(&tempMat4, &pickupStruct->pickupActor.Position, &camera->camPos, &camera->cameraUp);

        //pickupStruct->pickupActor.Transform = tempMat4;
        
        /*t3d_mat4_from_srt_euler(&pickupStruct->pickupActor.Transform,
            (float[3]){scale, scale, scale}, 
            (float[3]){pickupStruct->Rotation.v[0], pickupStruct->Rotation.v[1], pickupStruct->Rotation.v[2]},
            (float[3]){pickupStruct->pickupActor.Position.v[0], pickupStruct->pickupActor.Position.v[1] + 30.f, pickupStruct->pickupActor.Position.v[2]}
        );*/

        /*T3DMat4 ident = pickupStruct->pickupActor.Transform;

        //t3d_mat4_identity(&ident);
        T3DVec3 dumbFront = camera->cameraFront;
        dumbFront.v[0] = -dumbFront.v[0];
        dumbFront.v[1] = -dumbFront.v[1];
        dumbFront.v[2] = -dumbFront.v[2];

        t3d_mat4_rot_from_dir(&ident, &dumbFront, &camera->cameraUp);

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                pickupStruct->pickupActor.Transform.m[i][j] = ident.m[i][j];
            }
        }*/


        


        //transpose matCamera

        //T3DMat4 matCamT = &viewport->matCamera;



        //multiply with object matrix

        //re add








  //t3d_viewport_look_at(viewport, &camera->camPos, &camera->camTarget, &camera->cameraUp);
        //T3DMat4 temp;
        //t3d_mat4_look_at(&temp, &camera->camTarget,  &camera->camPos, &camera->cameraUp);



        //T3DMat4 ident;

        //t3d_mat4_identity(&ident);

        //t3d_mat4_rot_from_dir(&temp, camera->cameraFront, camera->cameraUp);

            /*fm_cosf(ThisCamera->Yaw3rdPerson) * fm_cosf(ThisCamera->Pitch3rdPerson),
            fm_sinf(ThisCamera->Pitch3rdPerson),
            fm_sinf(ThisCamera->Yaw3rdPerson) * fm_cosf(ThisCamera->Pitch3rdPerson)
           
            t3d_vec3_norm(&ThisCamera->cameraFront);*/


        //t3d_mat4_mul(&temp, &pickupStruct->pickupActor.Transform, &viewport->matCamera);

            /*for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    pickupStruct->pickupActor.Transform.m[i][j] = viewport->matCamera.m[i][j];
                }
            }*/

        //pickupStruct->pickupActor.Transform = viewport->matCamera;



    /*T3DVec3 diff;
    t3d_vec3_diff(&diff, &camera->camPos, &pickupStruct->pickupActor.Position);
    for (int i = 0; i < 3; i++)
    {
        pickupStruct->pickupActor.Transform.m[i][2] = diff.v[i];
    }*/
    


    /*T3DMat4 tempMat4;
    t3d_mat4_look_at(&tempMat4, &camera->camPos, &pickupStruct->pickupActor.Position, &camera->cameraUp);
    T3DMat4 mulMat;
    t3d_mat4_mul(&mulMat, &tempMat4, &pickupStruct->pickupActor.Transform);
    pickupStruct->pickupActor.Transform = mulMat;*/
    
    //t3d_mat4_identity(&tempMat4);


    /*T3DVec3 RightVector;
        t3d_vec3_cross(&RightVector, &camera->cameraFront, &camera->cameraUp);//Get right vector from up vector and front pointing vector
        fast_vec3_norm(&RightVector);
        T3DVec3 RightVectorNorm = RightVector;
        RightVectorNorm.v[1] = 0.f;
        fast_vec3_norm(&RightVectorNorm);
        T3DVec3 CameraFrontNorm = camera->cameraFront;
        CameraFrontNorm.v[1] = 0.f;
        fast_vec3_norm(&CameraFrontNorm);

    for (int i = 0; i < 3; i++)
    {

            pickupStruct->pickupActor.Transform.m[i][0] = RightVectorNorm.v[i];
            pickupStruct->pickupActor.Transform.m[i][0] = camera->cameraUp.v[i];

    }*/
        
    //T3DVec3 diff;
    //t3d_vec3_diff(&diff, &camera->camPos, &pickupStruct->pickupActor.Position);
    //t3d_vec3_norm(&diff);
    //t3d_mat4_rot_from_dir(&pickupStruct->pickupActor.Transform, &diff, &camera->cameraUp);
    //t3d_mat4_look_at(&pickupStruct->pickupActor.Transform, &pickupStruct->pickupActor.Position, &player->PlayerActor.Position, &camera->cameraUp);
    //t3d_mat4_translate(&pickupStruct->pickupActor.Transform, pickupStruct->pickupActor.Position.v[0], pickupStruct->pickupActor.Position.v[1], pickupStruct->pickupActor.Position.v[2]);
    //t3d_mat4_scale(&pickupStruct->pickupActor.Transform, scale, scale, scale);
    //t3d_mat4_rotate(&pickupStruct->pickupActor.Transform, &camera->cameraUp, 3.14159/2);

  //t3d_mat4_to_fixed(pickupStruct->pickupActor.TransformFP, &pickupStruct->pickupActor.Transform);

  pickupStruct->triggerStruct.TriggerActor.Position = pickupStruct->pickupActor.Position;
}

void PickupUpdateModel(struct PickupStruct* pickupStruct)
{
      //=========== Update Model =============//
        //awful_lookat(&tempMat4, &pickupStruct->pickupActor.Position, &camera->camPos, &camera->cameraUp);
        //t3d_mat4_look_at(&tempMat4, &pickupStruct->pickupActor.Position, &camera->camPos, &camera->cameraUp);

        //pickupStruct->pickupActor.Transform = tempMat4;
        
        t3d_mat4_from_srt_euler(&pickupStruct->pickupActor.Transform,
            (float[3]){pickupStruct->scale, pickupStruct->scale, pickupStruct->scale}, 
            (float[3]){pickupStruct->Rotation.v[0], pickupStruct->Rotation.v[1], pickupStruct->Rotation.v[2]},
            (float[3]){pickupStruct->pickupActor.Position.v[0], pickupStruct->pickupActor.Position.v[1] + 30.f, pickupStruct->pickupActor.Position.v[2]}
        );
        t3d_mat4_to_fixed(pickupStruct->pickupActor.TransformFP, &pickupStruct->pickupActor.Transform);
}

void PickupSetLocation(struct PickupStruct* pickupStruct, const T3DVec3* newLocation)
{
    pickupStruct->pickupActor.Position = *newLocation;
}

void PlayerPicksUp(struct PickupStruct* pickupStruct, struct PlayerStruct* playerStruct)
{
    if (pickupStruct->holdingPlayerStruct != NULL)
    {
        debugf("Can't take pickup out of someone else's hand\n");
        return;// false;
    }
    pickupStruct->pickupState = EPUS_PickedUp;
    pickupStruct->pickupActor.isDynamic = true;
    pickupStruct->pickupActor.hasCollision = true;
    if (pickupStruct->spawnLocationPtr != NULL)
    {
        pickupStruct->spawnLocationPtr->isOccupied = false;
        pickupStruct->spawnLocationPtr = NULL;
    }
    else{
        //debugf("BRO WHAT WHAT\n\n\n\n");
    }

    pickupStruct->holdingPlayerStruct = playerStruct;
    pickupStruct->holdingPlayerStruct->heldPickup = pickupStruct;
    if (pickupStruct->pickupType == EPUT_Snowball)
    {
        Player_RunBegin(pickupStruct->holdingPlayerStruct);
        pickupStruct->pickupActor.Position = pickupStruct->holdingPlayerStruct->PlayerActor.Position;
        pickupStruct->pickupActor.Position.v[0]+=40.f;//should be in player direction
        pickupStruct->prevPosition = pickupStruct->pickupActor.Position;

        playerStruct->CurrentVelocityLocal = (T3DVec3){{
            0.f,
            0.f,
            0.f
        }};
        color_t newColor = RGBA32(255, 255, 255, 255);
        SnowballSwapColours(pickupStruct, newColor);
    }
    else
    {
        debugf("Decoration, Let's see\n");
        pickupStruct->pickupActor.Position = pickupStruct->holdingPlayerStruct->PlayerActor.Position;
        //pickupStruct->pickupActor.Position.v[0]+=40.f;//should be in player direction
        //pickupStruct->prevPosition = pickupStruct->pickupActor.Position;

        playerStruct->CurrentVelocityLocal = (T3DVec3){{
            0.f,
            0.f,
            0.f
        }};

    }

}

void PlayerDrops(struct PickupStruct* pickupStruct, struct PlayerStruct* playerStruct)
{
    pickupStruct->pickupState = EPUS_Idle;
    pickupStruct->pickupActor.isDynamic = false;
    pickupStruct->pickupActor.hasCollision = true;//need way to prevent picking up on accident/while hit or hitting etc.

    if (pickupStruct->pickupType == EPUT_Snowball)
    {
        Player_RunEnd(pickupStruct->holdingPlayerStruct);
        if (pickupStruct->holdingPlayerStruct != NULL)
        {
            debugf("yo what's up 1\n");
            pickupStruct->holdingPlayerStruct->heldPickup = NULL;
            pickupStruct->holdingPlayerStruct = NULL;
        }
    }
    else
    {
        //pickupStruct->pickupActor.Position.v[1] = 0.f;
       // DecorationDeactivate(pickupStruct); !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    if (pickupStruct->holdingPlayerStruct != NULL)
    {
        debugf("yo what's up 2\n");
        pickupStruct->holdingPlayerStruct->heldPickup = NULL;
        pickupStruct->holdingPlayerStruct = NULL;
    }
    //pickupStruct->pickupActor.Position = (T3DVec3) {{0.f, -1000.f, 0.f}};
    pickupStruct->pickupState = EPUS_Idle;
    }

    //pickupStruct->holdingPlayerStruct->heldPickup = NULL;
    //pickupStruct->holdingPlayerStruct = NULL;
    
}

void PickupDelete(struct PickupStruct* pickupStruct)
{
    PickupDeactivate(pickupStruct);
    //debugf("delete\n");
    //pickupStruct->pickupActor.Position = (T3DVec3){{100.f, 0.f, 100.f}};
    
    //pickupStruct->pickupState = EPUS_Idle;
    //pickupStruct->pickupActor.isDynamic = false;
    //pickupStruct->pickupActor.hasCollision = true;//need way to prevent picking up on accident/while hit or hitting etc.

    //pickupStruct->holdingPlayerStruct = NULL;
}

void PickupSpawn(struct PickupStruct* pickupStruct)
{

}



void PickupDeactivate(struct PickupStruct* pickupStruct)
{
    pickupStruct->pickupActor.Position = (T3DVec3) {{-10000.f, -10000.f, -10000.f}};
    pickupStruct->pickupState = EPUS_Inactive;
    pickupStruct->snowballSize = 5.f;
    if (pickupStruct->holdingPlayerStruct != NULL)
    {
        pickupStruct->holdingPlayerStruct->heldPickup = NULL;
        pickupStruct->holdingPlayerStruct = NULL;
    }
}

void PickupActivate(struct PickupStruct* pickupStruct, struct SpawnLocation* location)
{
    pickupStruct->pickupState = EPUS_Idle;
    pickupStruct->spawnLocationPtr = location;
}

void PickupSetType(struct PickupStruct* pickupStruct, enum EDecorationType decorationType)
{
    pickupStruct->decorationType = decorationType;

    color_t color = color_from_packed32(0x000000<<8);

    /*if (pickupStruct->pickupActor.model != NULL)
    {
        t3d_model_free(pickupStruct->pickupActor.model);
    }*/

    if (pickupStruct->pickupType == EPUT_Decoration)
    {
        switch (pickupStruct->decorationType)
            {
            case EDT_Decoration1:
                pickupStruct->pickupActor.model = t3d_model_load("rom:/snowmen/deco_rock_1.t3dm");
                color = color_from_packed32(0x4a4a4a<<8);
                break;
            case EDT_Decoration2:
                pickupStruct->pickupActor.model = t3d_model_load("rom:/snowmen/deco_carrot.t3dm");
                color = color_from_packed32(0xdb741a<<8);
                break;
            case EDT_Decoration3:
                pickupStruct->pickupActor.model = t3d_model_load("rom:/snowmen/deco_mitt.t3dm");
                color = color_from_packed32(0xc75292<<8);
                break;
            case EDT_Decoration4:
                pickupStruct->pickupActor.model = t3d_model_load("rom:/snowmen/deco_hat.t3dm");
                color = color_from_packed32(0xffffff<<8);
                break;
            case EDT_Decoration5:
                pickupStruct->pickupActor.model = t3d_model_load("rom:/snowmen/deco_scarf.t3dm");
                color = color_from_packed32(0xb02e20<<8);
                break;
            case EDT_Decoration6:
                pickupStruct->pickupActor.model = t3d_model_load("rom:/snowmen/deco_stick.t3dm");
                color = color_from_packed32(0x704022<<8);
                break;
            
            default:
                break;
            }
    }
    else
    {
        pickupStruct->pickupActor.model = t3d_model_load("rom:/snowmen/pickup_snowball.t3dm");
        color = RGBA32(255, 255, 255, 255);

        rspq_block_begin();
            t3d_matrix_push(pickupStruct->pickupActor.TransformFP);
            rdpq_set_prim_color(color_from_packed32(0xfca9dd<<8));
            t3d_model_draw(pickupStruct->pickupActor.model);// Draw Static Mesh
        t3d_matrix_pop(1);// must also pop it when done
        pickupStruct->dplAltSnowball = rspq_block_end();
    }
   /*if(pickupStruct->decorationType == EDT_Decoration1)
   {
        pickupStruct->pickupActor.model = t3d_model_load("rom:/snowmen/deco_rock_1.t3dm");
        color = color_from_packed32(0x4a4a4a<<8);
   }
   else if (pickupStruct->decorationType == EDT_Decoration2)
   {
        pickupStruct->pickupActor.model = t3d_model_load("rom:/snowmen/deco_carrot.t3dm");
        color = color_from_packed32(0xdb741a<<8);
   }
    else if (pickupStruct->decorationType == EDT_Decoration3)
   {
        pickupStruct->pickupActor.model = t3d_model_load("rom:/snowmen/deco_mitt.t3dm");
        color = color_from_packed32(0xc75292<<8);
   }
   else if (pickupStruct->decorationType == EDT_Decoration4)
   {
        pickupStruct->pickupActor.model = t3d_model_load("rom:/snowmen/deco_hat.t3dm");
        color = color_from_packed32(0xffffff<<8);
   }
    else if (pickupStruct->decorationType == EDT_Decoration5)
   {
        pickupStruct->pickupActor.model = t3d_model_load("rom:/snowmen/deco_scarf.t3dm");
        color = color_from_packed32(0xb02e20<<8);
   }
    else if (pickupStruct->decorationType == EDT_Decoration6)
   {
        pickupStruct->pickupActor.model = t3d_model_load("rom:/snowmen/deco_stick.t3dm");
        color = color_from_packed32(0x704022<<8);
   }*/

    rspq_block_begin();
        t3d_matrix_push(pickupStruct->pickupActor.TransformFP);
        rdpq_set_prim_color(color);
        t3d_model_draw(pickupStruct->pickupActor.model);// Draw Static Mesh
    t3d_matrix_pop(1);// must also pop it when done
    pickupStruct->pickupActor.dpl = rspq_block_end();
    
}

void PickupFree(struct PickupStruct* pickupStruct)
{
    ActorFree(&pickupStruct->pickupActor);
    TriggerFree(&pickupStruct->triggerStruct);

    if (pickupStruct->dplAltSnowball != NULL)
    {
        rspq_block_free(pickupStruct->dplAltSnowball);
    }

}