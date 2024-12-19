#include "actor.h"

/*void CreateNewNPC(Actor* CrateActor, float width, float height, const T3DVec3* center, T3DMat4* Transform)
{
    Actor TempActor = {
    //.model = ,
    .actorType = EAT_NPC,
    .hasCollision = true,
    .isDynamic = false,
    //.initFunc = CrateInit,
    //.loopFunc = CrateLoop,
    .collisionType = ECT_Capsule,
    .collisionRadius = width/2.f,
    .collisionCenter = *center,
    .CollisionHeight = height,
    .CurrentVelocity = (T3DVec3){{0.f, 0.f, 0.f}},
    .DesiredMovement = (T3DVec3){{0.f, 0.f, 0.f}},
    .Position = (T3DVec3){{0.f, 0.f, 0.f}},
    .PrevPosition = (T3DVec3){{0.f, 0.f, 0.f}},
    .Transform = *Transform
    //T3DMat4FP TransformFP
    //rspq_block_t *dpl
  };
  CrateActor = &TempActor;
}*/

/*void NPCInit(struct Actor* actor, T3DMat4* Transform)
{
  actor->Transform = *Transform;
  actor->TransformFP = malloc_uncached(sizeof(T3DMat4FP));

  actor->collisionType = ECT_Capsule;

    rspq_block_begin();
    t3d_matrix_push(actor->TransformFP);//push the transformation matrix we created, 
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));  
    t3d_model_draw(actor->model);// Draw skinned mesh with main skeleton.  

    t3d_matrix_pop(1);// must also pop it when done
  actor->dpl = rspq_block_end();*/
   /*t3d_mat4_from_srt_euler(&actor->Transform,
    (float[3]){1.f, 1.f, 1.f},
    (float[3]){0.f, 0.f, 0.f},
    (float[3]){0.f, -300.f, 0.f}
    );*/
    /*t3d_mat4_to_fixed(actor->TransformFP, &actor->Transform);
}

void NPCLoop(struct Actor* actor, float deltaTime)
{
     
}*/

void ActorInit(struct Actor* actor)
{
  actor->BillboardPosition = (T3DVec3){{0.f, 0.f, 0.f}};
  //actor->BillboardTimer = 0.f;
  if (actor->collisionType == ECT_Mesh)
  {
    GenerateStaticCollisionNew(actor);
  }
}


void ActorLoop(struct Actor* actor, float deltaTime)
{

}


float ApplyFriction(float SpeedValue, float friction, float deltaTime)
{
  return SpeedValue - (SpeedValue*friction*deltaTime);
}

float MoveTowards(float CurrentVelocity1d, float DesiredVelocity1d, float MaxSpeedChange, float friction, float deltaTime)
{
  if (fabs(DesiredVelocity1d) <= 0.5f) return ApplyFriction(CurrentVelocity1d, friction, deltaTime);
    if (CurrentVelocity1d < DesiredVelocity1d)
    {
      if ((CurrentVelocity1d + MaxSpeedChange) < DesiredVelocity1d)
      {
        CurrentVelocity1d += MaxSpeedChange;
      }
      else
      {
        CurrentVelocity1d = DesiredVelocity1d;
      }
    }
    else if (CurrentVelocity1d > DesiredVelocity1d)
    {
      if ((CurrentVelocity1d - MaxSpeedChange) > DesiredVelocity1d)
      {
        CurrentVelocity1d -= MaxSpeedChange;
      }
      else
      {
        CurrentVelocity1d = DesiredVelocity1d;
      }
    }
    return CurrentVelocity1d;
}

T3DVec3 MoveTowardsVector(T3DVec3 CurrentVelocity, T3DVec3 DesiredVelocity, float MaxSpeedChange)//NOT EVEN USED LMAO
{
  /*if (t3d_vec3_len2(&DesiredVelocity) <= 0.25f) <---- THIS is patchwork fix, should find better solution
  {
    return (T3DVec3) {{
      ApplyFriction(CurrentVelocity.v[0]),
      0.f,
      ApplyFriction(CurrentVelocity.v[2])
    }};
  }*/
    float CurrVelocityLen2 = t3d_vec3_len2(&CurrentVelocity);
    float DesVelocityLen2 = t3d_vec3_len2(&DesiredVelocity);
    if (CurrVelocityLen2 < DesVelocityLen2)//increase velocity by maxspeed change to get closer to desired velocity
    {
        float CurrVelocityMag = sqrt(CurrVelocityLen2);
        float DesVelocityMag = sqrt(DesVelocityLen2);
        if (CurrVelocityMag + MaxSpeedChange < DesVelocityMag)
        {
            float NewCurrentSpeed = CurrVelocityMag + MaxSpeedChange;
            T3DVec3 CurrentRatio;
            if (CurrVelocityMag == 0.f)
            {
                float InverseDesVelocityMag = 1 / DesVelocityMag;
                CurrentRatio = (T3DVec3) {{
                DesiredVelocity.v[0] * InverseDesVelocityMag,
                0.f,
                DesiredVelocity.v[2] * InverseDesVelocityMag
                }};
            }
            else
            {
                float InverseCurrVelocityMag = 1 / CurrVelocityMag;
                CurrentRatio = (T3DVec3) {{
                CurrentVelocity.v[0] * InverseCurrVelocityMag,
                0.f,
                CurrentVelocity.v[2] * InverseCurrVelocityMag
                }};
            }

                CurrentVelocity = (T3DVec3) {{
                CurrentRatio.v[0] * NewCurrentSpeed,
                0.f,
                CurrentRatio.v[2] * NewCurrentSpeed
                }};
        }
        else
        {
            CurrentVelocity = DesiredVelocity;//adding full maxspeedchange would go above desired speed, cap at desired
        }
    }
    else if (CurrVelocityLen2 > DesVelocityLen2)//overshot, must reduce speed by maxspeedchange
    {
        if (sqrtf((t3d_vec3_len2(&CurrentVelocity))) - MaxSpeedChange > sqrtf(t3d_vec3_len2(&DesiredVelocity)))
        {
            float CurrVelocityMag = sqrt(CurrVelocityLen2);
            float DesVelocityMag = sqrt(DesVelocityLen2);
            float NewCurrentSpeed = CurrVelocityMag - MaxSpeedChange;
            T3DVec3 CurrentRatio;
            if (CurrVelocityMag == 0.f)
            {
                float InverseDesVelocityMag = 1 / DesVelocityMag;
                CurrentRatio = (T3DVec3) {{
                DesiredVelocity.v[0] * InverseDesVelocityMag,
                0.f,
                DesiredVelocity.v[2] * InverseDesVelocityMag
                }};
            }
            else
            {
                float InverseCurrVelocityMag = 1 / CurrVelocityMag;
                CurrentRatio = (T3DVec3) {{
                CurrentVelocity.v[0] * InverseCurrVelocityMag,
                0.f,
                CurrentVelocity.v[2] * InverseCurrVelocityMag
                }};
            }

                CurrentVelocity = (T3DVec3) {{
                CurrentRatio.v[0] * NewCurrentSpeed,
                0.f,
                CurrentRatio.v[2] * NewCurrentSpeed
                }};
        }
        else
        {
            CurrentVelocity = DesiredVelocity;
        }
    }
    return CurrentVelocity;
}

bool TestAllCollision(Actor* InstigatorActor, Actor** AllActors, T3DVec3* penetration_normal, float* penetration_depth, float deltaTime)//for now, for loop through all actors/terrain
{
    bool anyCollision=false;
    int numCollisions = 0;
    int length = 14;//13//7;//sizeof(AllActors) / sizeof(AllActors[0]);
    for (int i = 0; i < length; i++)//MUST CHANGE TO ACTUAL ARRAY LENGTH
    {
        //debugf("int i = %d\n", i);
        if (AllActors[i] != InstigatorActor)
        {
            //debugf("Pizza 2");
            if(TestCollision(InstigatorActor, AllActors[i], penetration_normal, penetration_depth, deltaTime)) //&& anyCollision == false)
            {
              //debugf("Penetration Depth = %f\n", *penetration_depth);
              //debugf("Pizza 3");
              anyCollision = true;
            //  numCollisions++;
              //debugf("num collisions: %d\n", numCollisions);
             // if (numCollisions >= 3)
             // {
                //debugf("This is it!\n");
                return anyCollision;
             // }
             // else
             // {
             //   i = -1;
                //debugf("OH my\n");
             // }
              //return anyCollision;
            }
        }
    }
    return anyCollision;
}

void CapsuleRespondCollideNSlide(Actor* CapsuleActor, T3DVec3* penetration_normal, float penetration_depth, float deltaTime)
{
  //CapsuleActor->Position = CapsuleActor->PrevPosition;
  //debugf("Time to slide... Penetration Depth = %f\n", penetration_depth);
  //debugf("Time to slide... Penetration Norm = %f, %f, %f\n", penetration_normal->v[0], penetration_normal->v[1], penetration_normal->v[2]);

  float velocity_length = t3d_vec3_len(&CapsuleActor->CurrentVelocity);
  T3DVec3 velocity_normalized = CapsuleActor->CurrentVelocity;
  if (velocity_length == 0.f)
  {
    //debugf("VINNY PLS\n");
    //  velocity_length = 0.0001f;
      velocity_normalized.v[0] = 0;
      velocity_normalized.v[1] = 1;
      velocity_normalized.v[2] = 0;
  }
  else
  {
    float invVelocityLength = 1 / velocity_length;//inversed velocity length
    velocity_normalized.v[0] *= invVelocityLength;
    velocity_normalized.v[1] *= invVelocityLength;
    velocity_normalized.v[2] *= invVelocityLength;
  }


  T3DVec3 undesired_motion = *penetration_normal;
  float scale_val = t3d_vec3_dot(&velocity_normalized, penetration_normal);
  scaleVector(&undesired_motion, scale_val);

  T3DVec3 desired_motion;
  t3d_vec3_diff(&desired_motion, &velocity_normalized, &undesired_motion);
  
  CapsuleActor->CurrentVelocity = desired_motion;
  scaleVector(&CapsuleActor->CurrentVelocity, velocity_length);

  T3DVec3 RemovedPenetration = *penetration_normal;
  scaleVector(&RemovedPenetration, (penetration_depth + 0.0001f));
  t3d_vec3_add(&CapsuleActor->Position, &CapsuleActor->Position, &RemovedPenetration);

  //finally apply velocity
  CapsuleActor->DesiredMovement = (T3DVec3){{
  CapsuleActor->CurrentVelocity.v[0] * deltaTime,
  CapsuleActor->CurrentVelocity.v[1] * deltaTime,
  CapsuleActor->CurrentVelocity.v[2] * deltaTime
  }};

  t3d_vec3_add(&CapsuleActor->Position, &CapsuleActor->Position, &CapsuleActor->DesiredMovement);
  // if (PlayerPosition.v[1] < -200.f)
  //{
  //PlayerPosition.v[1] = -200.f;
  //if (PlayerState == EPS_Airborne)
  //{  
  //PlayerState = EPS_Idle;
  //}
  //}

  //PlayerSphere.center = CapsuleActor->Position;

  //PlayerCapsule.base = CapsuleActor->Position;
  //PlayerCapsule.tip = CapsuleActor->Position;
  //PlayerCapsule.tip.v[1] += (CapsuleActor.CollisionHeight*2);//half height for capsule

  CapsuleActor->collisionCenter = (T3DVec3){{
  CapsuleActor->Position.v[0],
  CapsuleActor->Position.v[1] + CapsuleActor->CollisionHeight,
  CapsuleActor->Position.v[2]
  }};
}

bool TestCollision(Actor* InstigatorActor, Actor* OtherActor, T3DVec3* penetration_normal, float* penetration_depth, float deltaTime)//Dynamic against Static, eventually replace ifs with nested switch/case
{
  //debugf("TestCollision\n");
    //if ((InstigatorActor->collisionType == ECT_Capsule) && InstigatorActor->hasCollision
    //    && (OtherActor->collisionType == ECT_Mesh) && OtherActor->hasCollision)
    //    {
          //debugf("_ %d _\n", OtherActor->collisionType);
          if (OtherActor->collisionType == ECT_Mesh)
          {
            //debugf("It's a Mesh!\n");
            return TestCapsuleMeshCollision(InstigatorActor, OtherActor, penetration_normal, penetration_depth, deltaTime);
          }
          /*else if (OtherActor->collisionType == ECT_Capsule)
          {
            //return TestCapsuleCapsuleCollision(InstigatorActor, OtherActor, deltaTime);//for now
          }
          else if (OtherActor->collisionType == ECT_Environment)
          {
            return TestCapsuleEnvCollision(InstigatorActor, OtherActor, deltaTime);
          }
          else*/
          
          //debugf("%d but is actually %d", ECT_Sphere, OtherActor->collisionType);
          
          if (OtherActor->collisionType == ECT_Sphere)
          {
            //debugf("It's a Sphere!\n");
            SphereCollider s1;
            SphereCollider s2;

            s1 = (SphereCollider){
              .radius = InstigatorActor->collisionRadius,
              .center = InstigatorActor->Position
            };

            s2 = (SphereCollider){
              .radius = OtherActor->collisionRadius,
              .center = OtherActor->Position
            };
            //debugf("Ok, let's try and collide\n");
            return CollideSphereSphere(&s1, &s2, penetration_normal, penetration_depth);
          }
    //    }
    //    else return false;
    return false;
}

bool TraverseOctreeCapsule(struct Octree* node, const CapsuleCollider* capsule, T3DVec3* penetration_normal, float* penetration_depth, bool* ColHappened)
{
  //debugf("Happened? %d\n", *ColHappened);
  if (*ColHappened) return true;//does this function need to return a bool anyway?
  //for loop over all children
  //check if capsule's AABB intersects this child's AABB
    //if so, recursively call function on this node if this node has children
    //if node does NOT have children, return vert list
    //will have to combine multiple vert lists potentially if capsule is in multiple nodes
  if (node->children == NULL) return false;

  for (int i = 0; i < 8; i++)  
  {
    indicies_counter++;
    /*debugf("i =  %d\n", i);
    debugf("Capsule Min\n");
        debugf("%f, %f, %f\n", capsule->Capsule_AABB_Min.v[0], capsule->Capsule_AABB_Min.v[1], capsule->Capsule_AABB_Min.v[2]);
    debugf("Capsule Max\n");
        debugf("%f, %f, %f\n", capsule->Capsule_AABB_Max.v[0], capsule->Capsule_AABB_Max.v[1], capsule->Capsule_AABB_Max.v[2]);
    debugf("AABB Min\n");
        debugf("%f, %f, %f\n", node->children[i].AABB_Min.v[0], node->children[i].AABB_Min.v[1], node->children[i].AABB_Min.v[2]);
    debugf("AABB Max\n");
        debugf("%f, %f, %f\n", node->children[i].AABB_Max.v[0], node->children[i].AABB_Max.v[1], node->children[i].AABB_Max.v[2]); */

    if (TestAABBvsAABB(&capsule->Capsule_AABB_Min, &capsule->Capsule_AABB_Max, &node->children[i].AABB_Min, &node->children[i].AABB_Max))
    /*if (capsule->Capsule_AABB_Min.v[0] <= node->children[i].AABB_Max.v[0] && capsule->Capsule_AABB_Max.v[0] >= node->children[i].AABB_Min.v[0] &&
        capsule->Capsule_AABB_Min.v[1] <= node->children[i].AABB_Max.v[1] && capsule->Capsule_AABB_Max.v[1] >= node->children[i].AABB_Min.v[1] &&
        capsule->Capsule_AABB_Min.v[2] <= node->children[i].AABB_Max.v[2] && capsule->Capsule_AABB_Max.v[2] >= node->children[i].AABB_Min.v[2])*/
    //if (true)//check every single octant. If this works, that means collision works fine but we're putting the tris in wrong octants, or something like that
    {
      //debugf("we in the octant now boys %d\n", i);
      /*debugf("if 1\n");
      if (node->children == NULL) debugf("well shoot\n");
      else debugf("oh ok\n");*/
      if (node->children[i].children != NULL) 
      {
        //debugf("if 2\n");
        if (TraverseOctreeCapsule(&node->children[i], capsule, penetration_normal, penetration_depth, ColHappened))
        {
          return true;
        }
      }
      else
      {
        //debugf("else 2\n");
        //numVertsToCheck += node->children[i]->numVertices
        //add verts to vertsToCheck
        //debugf("This node has no children, let's test it's triangles (%d)\n", node->children[i].numVertices/3);
        for (int j = 0; j < node->children[i].numVertices; j += 3)
        {
          /*debugf("test trianlge %d\n", j / 3);
          debugf("Verts to test:\n");
          debugf("%f, %f, %f\n", node->children[i].vertices[j].v[0], node->children[i].vertices[j].v[1], node->children[i].vertices[j].v[2]);
          debugf("%f, %f, %f\n", node->children[i].vertices[j+1].v[0], node->children[i].vertices[j+1].v[1], node->children[i].vertices[j+1].v[2]);
          debugf("%f, %f, %f\n", node->children[i].vertices[j+2].v[0], node->children[i].vertices[j+2].v[1], node->children[i].vertices[j+2].v[2]);
          debugf("Max and Min:\n");
          debugf("%f, %f, %f\n", capsule->Capsule_AABB_Max.v[0], capsule->Capsule_AABB_Max.v[1], capsule->Capsule_AABB_Max.v[2]);
          debugf("%f, %f, %f\n", capsule->Capsule_AABB_Min.v[0], capsule->Capsule_AABB_Min.v[1], capsule->Capsule_AABB_Min.v[2]);*/

          
          if (CollideCapsuleTriangle(&node->children[i].vertices[j], capsule, penetration_normal, penetration_depth))
          {
            //debugf("if 3\n");
           // debugf("REAL collision at %d\n", i);
              *ColHappened = true;
              return true;
          }
        }
      }
    }
  }
  return false;
}

bool CollideCapsuleMeshOctree(struct Actor* actor, const CapsuleCollider* capsule, T3DVec3* penetration_normal, float* penetration_depth)
{
  bool* ColHappened = malloc(sizeof(bool));//this is kind of a crappy fix, should return array of all potential tris and only do CollideCapsuleTriangle in this func
  *ColHappened = false;
    /*for (int i = 0; i < actor->numCollisionTris; i++)
    {
        if (CollideCapsuleTriangle(&actor->CollisionVertices[i*3], capsule, penetration_normal, penetration_depth))
        {
            return true;
        }
    }
    return false;*/
    //malloc for vertstocheck
    //T3DVec3* vertsToCheck = malloc(sizeof(T3DVec3))
    //int numVertsToCheck;
    TraverseOctreeCapsule(&actor->CollisionOctree, capsule, penetration_normal, penetration_depth, ColHappened);
    if (*ColHappened)
    {
      //debugf("ALRIGHT! WE GOT A COLLISION!\n");
      return true;
    }
    else{
      return false;
    }

}

bool CollideCapsuleMeshCached(struct Actor* actor, const CapsuleCollider* capsule, T3DVec3* penetration_normal, float* penetration_depth)
{
  //debugf("Are we even getting here %d???????????????????????????????\n", actor->numCollisionTris);
  //T3DVec3 verticies[3];
  //int cringecounter = 0;
    for (int i = 0; i < actor->numCollisionTris; i++)
    {
      //debugf("I guess we're here: %d???????????????????????????????\n", actor->numCollisionTris);
        //verticies[0] = CollisionVertices[i]


        if (CollideCapsuleTriangle(&actor->CollisionVertices[i*3], capsule, penetration_normal, penetration_depth))
        {
          //debugf("WOW! %d\n", cringecounter);
            return true;
        }
        //cringecounter++;
    }
    //debugf("Aw... %d\n", cringecounter);
    return false;
}

bool TestCapsuleMeshCollision(Actor* CapsuleActor, Actor* StaticMeshActor, T3DVec3* penetration_normal, float* penetration_depth, float deltaTime)
{
  capsule_mesh_counter++;
    //!!!!!!!!!!!!!!! Must set PrevPosition in Grounded movement!!!!!!!!!!!!!!!!!
    //T3DVec3 penetration_normal;
    //float penetration_depth;

    T3DVec3 playerTip = (T3DVec3){{
    CapsuleActor->collisionCenter.v[0],// + CapsuleActor->CollisionHeight,
    CapsuleActor->collisionCenter.v[1] - CapsuleActor->CollisionHeight,
    CapsuleActor->collisionCenter.v[2]// + CapsuleActor->CollisionHeight
    }};
    //debugf("collision tip: %f, %f, %f\n", playerTip.v[0], playerTip.v[1], playerTip.v[2]);
    T3DVec3 playerBase = (T3DVec3){{
    CapsuleActor->collisionCenter.v[0],// - CapsuleActor->CollisionHeight,
    CapsuleActor->collisionCenter.v[1] + CapsuleActor->CollisionHeight,
    CapsuleActor->collisionCenter.v[2]// - CapsuleActor->CollisionHeight
    }};
    //debugf("collision base: %f, %f, %f\n", playerBase.v[0], playerBase.v[1], playerBase.v[2]);

    CapsuleCollider PlayerCapsule = (CapsuleCollider){
    CapsuleActor->collisionRadius,
    playerBase,
    playerTip,
    CapsuleActor->AABB_Min,
    CapsuleActor->AABB_Max
    };



    //if (CollideCapsuleMesh(StaticMeshActor->model, &StaticMeshActor->Transform, &PlayerCapsule, &penetration_normal, &penetration_depth))
    if (CollideCapsuleMeshCached(StaticMeshActor, &PlayerCapsule, penetration_normal, penetration_depth)){
      //debugf("                      WORLDS COLLIDE\n");
    //debugf("   $       $       $     $    alright, let's test this mesh\n");
    //if (CollideCapsuleMeshOctree(StaticMeshActor, &PlayerCapsule, penetration_normal, penetration_depth))
    //{
      //debugf("Pen Depth: %f, Pen norm: %f, %f, %f\n", penetration_depth, penetration_normal.v[0], penetration_normal.v[1], penetration_normal.v[2]);
        
        return true;
    }
      //debugf(" ***************AW DANG****************\n");
    
    return false;
        //  if (PlayerState == EPS_Airborne)
        //{
        //CurrentVerticalSpeed -= GravityAcceleration*unit * deltaTime;
        //VerticalMovement = CurrentVerticalSpeed * deltaTime;
        //}
        //else
        //{
        //  VerticalMovement = 0.f;
        //}

        /*DesiredMovement = (T3DVec3){{
        0.f,
        VerticalMovement,
        0.f
        }};

        t3d_vec3_add(&CapsuleActor->Position, &CapsuleActor->Position, &DesiredMovement);

        CapsuleActor.collisionCenter = (T3DVec3){
        CapsuleActor->Position.v[0];
        CapsuleActor->Position.v[1] + CapsuleActor.CollisionHeight;
        CapsuleActor->Position.v[2];
        };*/
}

bool TestCapsuleEnvCollision(Actor* CapsuleActor, Actor* CHANGEEnvstruct, float deltaTime)
{
  return false;
}

bool TestCapsuleCapsuleCollision(Actor* CapsuleActor1, Actor* CapsuleActor2, float deltaTime)
{
    //!!!!!!!!!!!!!!! Must set PrevPosition in Grounded movement!!!!!!!!!!!!!!!!!
    T3DVec3 penetration_normal;
    float penetration_depth;

    T3DVec3 playerTip = (T3DVec3){{
    CapsuleActor1->collisionCenter.v[0],// + CapsuleActor->CollisionHeight,
    CapsuleActor1->collisionCenter.v[1] + CapsuleActor1->CollisionHeight,
    CapsuleActor1->collisionCenter.v[2]// + CapsuleActor->CollisionHeight
    }};
    T3DVec3 playerBase = (T3DVec3){{
    CapsuleActor1->collisionCenter.v[0],// - CapsuleActor->CollisionHeight,
    CapsuleActor1->collisionCenter.v[1] - CapsuleActor1->CollisionHeight,
    CapsuleActor1->collisionCenter.v[2]// - CapsuleActor1->CollisionHeight
    }};

    CapsuleCollider Capsule1 = (CapsuleCollider){
    CapsuleActor1->collisionRadius,
    playerBase,
    playerTip
    };


    T3DVec3 playerTip2 = (T3DVec3){{
    CapsuleActor2->collisionCenter.v[0],// + CapsuleActor->CollisionHeight,
    CapsuleActor2->collisionCenter.v[1] + CapsuleActor2->CollisionHeight,
    CapsuleActor2->collisionCenter.v[2]// + CapsuleActor->CollisionHeight
    }};
    T3DVec3 playerBase2 = (T3DVec3){{
    CapsuleActor2->collisionCenter.v[0],// - CapsuleActor->CollisionHeight,
    CapsuleActor2->collisionCenter.v[1] - CapsuleActor2->CollisionHeight,
    CapsuleActor2->collisionCenter.v[2]// - CapsuleActor2->CollisionHeight
    }};

    CapsuleCollider Capsule2 = (CapsuleCollider){
    CapsuleActor2->collisionRadius,
    playerBase2,
    playerTip2
    };


    if (CollideCapsuleCapsule(&Capsule1, &Capsule2, &penetration_normal, &penetration_depth))
    {
        CapsuleRespondCollideNSlide(CapsuleActor1, &penetration_normal, penetration_depth, deltaTime);
        return true;
    }
    
    return false;
}

void CalcCapsuleAABB(struct Actor* playerActor)
{
  T3DVec3 base_sphere_min = {{
      playerActor->Position.v[0] - playerActor->collisionRadius,
      playerActor->Position.v[1] - playerActor->collisionRadius,
      playerActor->Position.v[2] - playerActor->collisionRadius,
  }};
  T3DVec3 base_sphere_max = {{
      playerActor->Position.v[0] + playerActor->collisionRadius,
      playerActor->Position.v[1] + playerActor->collisionRadius,
      playerActor->Position.v[2] + playerActor->collisionRadius,
  }};
  T3DVec3 tip_sphere_min = {{
      playerActor->Position.v[0] - playerActor->collisionRadius,
      playerActor->Position.v[1] + playerActor->CollisionHeight*2  - playerActor->collisionRadius,
      playerActor->Position.v[2] - playerActor->collisionRadius,
  }};
  T3DVec3 tip_sphere_max = {{
      playerActor->Position.v[0] + playerActor->collisionRadius,
      playerActor->Position.v[1] + playerActor->CollisionHeight*2  + playerActor->collisionRadius,
      playerActor->Position.v[2] + playerActor->collisionRadius,
  }};

  playerActor->AABB_Min = base_sphere_min;
  for (int i = 0; i < 3; i++)
  {
      if (tip_sphere_min.v[0] < base_sphere_min.v[0]) playerActor->AABB_Min.v[0] = tip_sphere_min.v[0];
  }
  playerActor->AABB_Max = base_sphere_max;
  for (int i = 0; i < 3; i++)
  {
      if (tip_sphere_max.v[0] > base_sphere_max.v[0]) playerActor->AABB_Max.v[0] = tip_sphere_max.v[0];
  }
}

void GenerateStaticCollisionNew(struct Actor* actor)
{
  debugf("Matterial position: %f, %f, %f\n", actor->Transform.m[3][0], actor->Transform.m[3][1], actor->Transform.m[3][2]);

    //const char *path = "rom:/mygame/box.col";
    int size = 0;
    CollisionStruct* model = asset_load(actor->collisionModelPath, &size);
    if(memcmp(model, "COL", 3) != 0) 
    {
        assertf(false, "Invalid collision file: %s", actor->collisionModelPath);
    }
    assertf(model->magic[3] == 42,
    "Invalid T3D model version: %d != %d\n"
    "Please make a clean build of t3d and your project",
    42, model->magic[3]);
    /*debugf("Is indeed a collision file! Size: %d\n", size);
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
    }*/

    actor->CollisionVertices = malloc(sizeof(T3DVec3) * model->totalTriCount * 3);

    T3DVec3 vertices[3];
    for (int j = 0; j < model->totalTriCount; j ++) {
        //GetVerticles(vertexFP, part, j);
        int16_t vertexFP[3][3];
        vertexFP[0][0] = model->tris[j].verts[0].v[0];
        vertexFP[0][1] = model->tris[j].verts[0].v[1];
        vertexFP[0][2] = model->tris[j].verts[0].v[2];
        vertexFP[1][0] = model->tris[j].verts[1].v[0];
        vertexFP[1][1] = model->tris[j].verts[1].v[1];
        vertexFP[1][2] = model->tris[j].verts[1].v[2];
        vertexFP[2][0] = model->tris[j].verts[2].v[0];
        vertexFP[2][1] = model->tris[j].verts[2].v[1];
        vertexFP[2][2] = model->tris[j].verts[2].v[2];

        ConvertVerticies(vertices, vertexFP, &actor->Transform);

        actor->CollisionVertices[j*3] = vertices[0];
        actor->CollisionVertices[j*3 + 1] = vertices[1];
        actor->CollisionVertices[j*3 + 2] = vertices[2];
    }

    actor->numCollisionTris = model->totalTriCount;
    /*debugf("%f %f %f %f\n", actor->Transform.m[0][0], actor->Transform.m[0][1], actor->Transform.m[0][2], actor->Transform.m[0][3]);
    debugf("%f %f %f %f\n", actor->Transform.m[1][0], actor->Transform.m[1][1], actor->Transform.m[1][2], actor->Transform.m[1][3]);
    debugf("%f %f %f %f\n", actor->Transform.m[2][0], actor->Transform.m[2][1], actor->Transform.m[2][2], actor->Transform.m[2][3]);
    debugf("%f %f %f %f\n", actor->Transform.m[3][0], actor->Transform.m[3][1], actor->Transform.m[3][2], actor->Transform.m[3][3]);*/


  free(model);
}

void ActorFree(Actor* actor)
{
  if(actor->collisionType == ECT_Mesh)
  {
    free(actor->CollisionVertices);
  }
if(actor->dpl != NULL)
{
  rspq_block_free(actor->dpl);
}
//rspq_block_free(actor->dpl);
if(actor->TransformFP != NULL)
{
  free_uncached(actor->TransformFP);
}
//free_uncached(actor->TransformFP);
if(actor->model != NULL)
{
  t3d_model_free(actor->model);
}




//free collision path?

}