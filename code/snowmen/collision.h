#ifndef MYHEADER_H
#define MYHEADER_H

#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>


enum CollisionTypes {
    ECT_Sphere,
    ECT_Capsule,
    ECT_Mesh,
    ECT_Box,
    ECT_Environment
};

typedef struct {

} TrianglePoints;

typedef struct {
    float radius;
    T3DVec3 center;
} SphereCollider;

typedef struct {
    float radius;
    T3DVec3 tip;
    T3DVec3 base;
    T3DVec3 Capsule_AABB_Min;
    T3DVec3 Capsule_AABB_Max;
} CapsuleCollider;




//static uint32_t pizza;
extern uint32_t sphere_tri_counter;
extern uint32_t capsule_tri_counter;
extern uint32_t capsule_mesh_counter;
extern uint32_t chunk_counter;
extern uint32_t part_counter;
extern uint32_t indicies_counter;


bool CollideSphereSphere(const SphereCollider*, const SphereCollider*, T3DVec3*, float*);//requires sphere center location of both spheres and each radius
//returns whether it's a hit, penetration depth, and penetration normal

bool CollideSphereTriangle(const T3DVec3*, const SphereCollider*, T3DVec3*, float*);//Requires tri struct, sphere center/radius
//const T3DObject* mesh replace verticies

void CollideCapsuleSphere();//Very similar to sphere-sphere collision, just find closes point on line to sphere for the capsule

bool CollideCapsuleCapsule(const CapsuleCollider*, const CapsuleCollider*, T3DVec3*, float*);//will eventually go to sphere-sphere. Requires Tip and Base points, as well as sphere stuff, for both
//Returns

bool CollideCapsuleTriangle(const T3DVec3*, const CapsuleCollider*, T3DVec3*, float*);//Will eventually go to sphere-triangle. Requires tri struct, and requires Tip and Base points, 
    //as well as sphere stuff, for the capsule
//const T3DObject* mesh replace verticies

bool CollideCapsuleMesh(const T3DModel*, const T3DMat4*, const CapsuleCollider*, T3DVec3*, float*);//Will eventually go to sphere-triangle. Requires tri struct, and requires Tip and Base points, 


void GetVerticles(int16_t vertex[3][3], const T3DObjectPart *part, int j);
//returns struct of 3 vectors for each point of the tri

void ClosestPointOnLineSegment(T3DVec3*, const T3DVec3*, const T3DVec3*, const T3DVec3*);//requires two distances (_____) and point

void scaleVector(T3DVec3* vector, float scale);

float clamp_Float(float value, float min, float max);

bool GetObjectFromModel(T3DObject* obj, const T3DModel* model, uint32_t c);

void ConvertVerticies(T3DVec3* verticies, const int16_t vertexFP[3][3], const T3DMat4* mat);

T3DObject* t3d_model_get_nameless_object(const T3DModel *model, uint32_t i);

void fast_vec3_norm(T3DVec3 *);

bool TestAABBCapsuleTriangle(const CapsuleCollider* capsule, const T3DVec3* verticies);

bool TestAABBvsAABB(const T3DVec3* aMin, const T3DVec3* aMax, const T3DVec3* bMin, const T3DVec3* bMax);


#endif