#include "crate.h"

void CreateNewCrate(Actor* CrateActor, float width, float height, const T3DVec3* center, T3DMat4* Transform)
{
    Actor TempActor = {
    //.model = ,
    .actorType = EAT_Crate,
    .hasCollision = true,
    .isDynamic = false,
    //.initFunc = CrateInit,
    //.loopFunc = CrateLoop,
    .collisionType = ECT_Mesh,
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
}

void CrateInit(struct Actor* actor, T3DMat4* Transform)
{
  actor->Transform = *Transform;
  actor->TransformFP = malloc_uncached(sizeof(T3DMat4FP));
  actor->model = t3d_model_load("rom:/box.t3dm");

  actor->collisionType = ECT_Sphere;

    rspq_block_begin();
    t3d_matrix_push(actor->TransformFP);//push the transformation matrix we created, 
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
    t3d_model_draw(actor->model);// Draw skinned mesh with main skeleton.  

    t3d_matrix_pop(1);// must also pop it when done
  actor->dpl = rspq_block_end();
   /*t3d_mat4_from_srt_euler(&actor->Transform,
    (float[3]){1.f, 1.f, 1.f},
    (float[3]){0.f, 0.f, 0.f},
    (float[3]){0.f, -300.f, 0.f}
    );*/
    t3d_mat4_to_fixed(actor->TransformFP, &actor->Transform);

    //GenerateStaticCollision(actor);
}

void CrateInitGiveModel(struct Actor* actor, T3DMat4* Transform, T3DModel *givenModel)
{
  actor->Transform = *Transform;
  actor->TransformFP = malloc_uncached(sizeof(T3DMat4FP));
  actor->model = givenModel;

  actor->collisionType = ECT_Mesh;

    rspq_block_begin();
    t3d_matrix_push(actor->TransformFP);//push the transformation matrix we created, 
    rdpq_set_prim_color(RGBA32(255, 255, 255, 255));
    t3d_model_draw(actor->model);// Draw skinned mesh with main skeleton.  

    t3d_matrix_pop(1);// must also pop it when done
  actor->dpl = rspq_block_end();
   /*t3d_mat4_from_srt_euler(&actor->Transform,
    (float[3]){1.f, 1.f, 1.f},
    (float[3]){0.f, 0.f, 0.f},
    (float[3]){0.f, -300.f, 0.f}
    );*/
    t3d_mat4_to_fixed(actor->TransformFP, &actor->Transform);

    //GenerateStaticCollision(actor);
}

void CrateLoop(struct Actor* actor, float deltaTime)
{
   
}