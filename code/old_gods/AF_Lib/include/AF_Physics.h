/*
===============================================================================
AF_PHYSICS_H

Implementation for Physics helper functions such as
General collision check on all entities with filtering
AABB collision detection
Some code inspired by https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/physicstutorials
===============================================================================
*/
#ifndef AF_PHYSICS_H
#define AF_PHYSICS_H
#include "AF_Lib_Define.h"
#include "ECS/Entities/AF_Entity.h"
#include "ECS/Components/AF_CCollider.h"
#include "ECS/Components/AF_CTransform3D.h"
#include "ECS/Components/AF_CTransform2D.h"
#include "ECS/Entities/AF_ECS.h"
#include "AF_CollisionVolume.h"
#include "AF_Debug.h"
#include "AF_Ray.h"
#include "AF_Vec3.h"
#include "AF_Vec4.h"
#include "AF_Util.h"
//#include "AF_QuadTree.h"

#include <libdragon.h>
#ifdef __cplusplus
extern "C" {
#endif

#define GRAVITY_SCALE -9.8
#define DAMPING_FACTOR 0.05f

static const Vec3 AF_PHYSICS_CUBE_COLLISION_FACES [6] =
{
	{ -1 , 0 , 0} , { 1 , 0 , 0} ,
	{ 0 , -1 , 0} , { 0 , 1 , 0} ,
	{ 0 , 0 , -1} , { 0 , 0 , 1} ,
};


/*
====================
AF_PHYSICS_INIT
Definition for Physics init
====================
*/
void AF_Physics_Init();

/*
====================
AF_PHYSICS_UPDATE
Definition for Physics update
====================
*/
void AF_Physics_Update(AF_ECS* _ecs, const float _dt);


/*
static void AF_Physics_EarlyUpdate(AF_ECS* _ecs){
	// clear the velocities
	for(int i =0 ; _ecs->entitiesCount; ++i){
		AF_C3DRigidbody* rigidbody =  &_ecs->rigidbodies[i];
		// clear the velocity
		Vec3 zeroVelocity = {0,0,0};
		rigidbody->velocity = zeroVelocity;
	}
}*/

/*
====================
AF_Physics_LateUpdate
functino to be called during late update. allows rendering debug to occur and to occur after movmeent
======
*/
void AF_Physics_LateUpdate(AF_ECS* _ecs);

/*
====================
AF_Physics_LateRenderUpdate
functino to be called during late update. allows rendering debug to occur and to occur inbetween render calls, allowing debug rendering to occur
======
*/
void AF_Physics_LateRenderUpdate(AF_ECS* _ecs);

/*
====================
AF_PHYSICS_SHUTDOWN
Definition for Physics shutdown 
====================
*/
void AF_Physics_Shutdown();

//=====HELPER FUNCTIONS=====


/*
====================
AF_Physics_ApplyAngularImpulse
Apply force to rigidbody object
====================
*/
static inline void AF_Physics_ApplyAngularImpulse( AF_C3DRigidbody *  _rigidbody, const Vec3 _force){
	Vec3 angularForce = Vec3_MULT(_rigidbody->inertiaTensor, _force);
	_rigidbody->anglularVelocity = Vec3_ADD(_rigidbody->anglularVelocity, angularForce);
}

/*
====================
AF_Physics_ApplyLinearImpulse
Apply force to rigidbody object
====================
*/
static inline void AF_Physics_ApplyLinearImpulse( AF_C3DRigidbody *  _rigidbody, const Vec3 _force){
	Vec3 linearForce;
	if(_rigidbody->inverseMass > 0){
		linearForce = Vec3_MULT_SCALAR(_force, _rigidbody->inverseMass);
	}else{
		linearForce = _force;
	}
	 
	_rigidbody->velocity = Vec3_ADD(_rigidbody->velocity, linearForce);
}

/*
====================
AF_PHYSICS_ADDFORCEATPOSITION
Add force at a position
====================
*/
/*
static void AF_Physics_AddForceAtPosition(AF_CTransform3D* _transform, AF_C3DRigidbody* _rigidbody, const Vec3* _addedForce, const Vec3* _position){
	Vec3 localPos = Vec3_MINUS(*_position, _transform->pos);

	_rigidbody->force = Vec3_ADD(_rigidbody->force, *_addedForce);
	_rigidbody->torque = Vec3_CROSS(localPos, *_addedForce);
}*/

// TODO move this to vec4
// Function to create a quaternion from angular velocity and time step

static inline Vec4 createQuaternionFromAngularVelocity(Vec3 angVel, float dt) {
    // Calculate the scalar component (w) of the quaternion
    float halfDt = dt * 0.5f; // Half of the time step
    float angleMagnitude = Vec3_MAGNITUDE(angVel); // Calculate the magnitude of angular velocity
    float w = cos(angleMagnitude * halfDt); // Scalar part

    // Calculate the vector part of the quaternion
    Vec3 vectorPart = Vec3_MULT_SCALAR(angVel, sin(angleMagnitude * halfDt) / angleMagnitude);

    // Create and return the quaternion
    Vec4 q = {
        vectorPart.x,
        vectorPart.y,
        vectorPart.z,
        w
    };
    return q;
}


/*
====================
AF_PHYSICS_INTEGRATEVELOCITY
Integrate the position and some dampening into the velocity
====================
*/

static inline void AF_Physics_IntegrateVelocity(AF_CTransform3D* _transform, AF_C3DRigidbody* _rigidbody, const float _dt){
	float frameDamping = powf ( DAMPING_FACTOR, _dt);

	Vec3 position = _transform->pos;
	Vec3 linearVelocity = _rigidbody->velocity;
	Vec3 linearDT = Vec3_MULT_SCALAR(linearVelocity, _dt);
	position = Vec3_ADD(position, linearDT);
	_transform->pos = position;

	// LinearDamping
	linearVelocity = Vec3_MULT_SCALAR(linearVelocity, frameDamping);
	_rigidbody->velocity = linearVelocity;

	// Angular velocity and orientation
	Vec4 orientation = _transform->orientation;
	Vec3 angVel = _rigidbody->anglularVelocity;

	Vec3 anglularVecDT = Vec3_MULT_SCALAR(angVel, _dt);
	Vec3 anglularVecDTHalf = Vec3_MULT_SCALAR(anglularVecDT, 0.5f);
	Vec4 quatAngVel = createQuaternionFromAngularVelocity(anglularVecDTHalf,0.0);
	Vec4 quatAngOrient = Vec4_MULT(quatAngVel, orientation);
	orientation = Vec4_ADD(orientation, quatAngOrient);
	orientation = Vec4_NORMALIZE(orientation);

	_transform->orientation = orientation;

	angVel = Vec3_MULT_SCALAR(angVel, frameDamping);
	_rigidbody->anglularVelocity = angVel;
}

/*
====================
AF_PHYSICS_INTEGRATEACCELL
Integrate gravity and acceleration into the velocity
====================
*/
static inline void AF_Physics_IntegrateAccell(AF_C3DRigidbody* _rigidbody, const float _dt){
	// iterate over all the game objects
	if(AF_Component_GetEnabled(_rigidbody->enabled != TRUE)){
		return;
	}
	float inverseMass = _rigidbody->inverseMass;

	if (inverseMass > 0.0f) {
		Vec3 force = _rigidbody->force;
		Vec3 accell = Vec3_MULT_SCALAR(force, inverseMass);
		if (_rigidbody->gravity == TRUE) {
			accell.y += GRAVITY_SCALE;
		}
		Vec3 accelDT = Vec3_MULT_SCALAR(accell, _dt);
		_rigidbody->velocity = Vec3_ADD(_rigidbody->velocity, accelDT);
	}
}


/*
====================
AF_PHYSICS_SPHERE_RAYINTERSECTION
Calculate ray intersection hit test
====================
*/
static inline BOOL AF_Physics_Sphere_RayIntersection(const Ray* _ray, const AF_CTransform3D* _transform, const AF_CCollider* _collider, AF_Collision* _collision){
	Vec3 spherePos = _transform->pos;
	//Box* sphereCollisionVolume = (Sphere_CollisionVolume*)_collider->boundingVolume;
	AF_FLOAT sphereRadius = _collider->boundingVolume.x;
	Vec3 direction = Vec3_MINUS(spherePos, _ray->position);
	//Then project the sphere’s origin onto our ray direction vector

	AF_FLOAT sphereProj = Vec3_DOT(direction, _ray->direction);
	
	// Is point behind the ray?
	if(sphereProj < 0.0f) {
		return FALSE; //point is behind the ray!
	}	
	
	//Get closest point on ray line to sphere
	Vec3 point = Vec3_MULT_SCALAR(Vec3_ADD(_ray->position, _ray->direction), sphereProj);
	
	AF_FLOAT sphereDist = Vec3_MAGNITUDE(Vec3_MINUS(point, spherePos));

	if (sphereDist > sphereRadius) { 
		return FALSE;
	}
	AF_FLOAT offset = sqrt((sphereRadius * sphereRadius) - (sphereDist * sphereDist));//AF_Math_Sqrt((sphereRadius * sphereRadius) - (sphereDist * sphereDist));

 
	_collision->rayDistance = sphereProj - (offset);
	_collision->collisionPoint = Vec3_ADD(_ray->position, Vec3_MULT_SCALAR(_ray->direction, _collision->rayDistance));
	return TRUE;
		
} 

/*
====================
AF_PHYSICS_Box_RAYINTERSECTION
Calculate ray intersection hit test against a box
====================
*/
static inline BOOL AF_Physics_Box_RayIntersection(const Ray* _ray, const Vec3 _boxPos, const Vec3 _boxSize, AF_Collision* _collision){
	Vec3 boxMin = Vec3_MINUS(_boxPos, _boxSize);
	Vec3 boxMax = Vec3_ADD(_boxPos, _boxSize);
	BOOL returnResult = TRUE;
	AF_FLOAT rayPos[3] = {_ray->position.x, _ray->position.y, _ray->position.z};
	AF_FLOAT rayDir[3] = {_ray->direction.x, _ray->direction.y, _ray->direction.z};
	
	AF_FLOAT tVals[3] = {-1, -1, -1};
	// convert to float array so we can use a forloop on the values
	AF_FLOAT boxMinArray[3] = {boxMin.x, boxMin.y, boxMin.z};
	AF_FLOAT boxMaxArray[3] = {boxMax.x, boxMax.y, boxMax.z};
	

	// Get the x, y, z values if the collision is infront or behind the box edge
	for(int i = 0; i < 3; ++i){
		if(rayDir[i] > 0){
			tVals[i] = (boxMinArray[i] - rayPos[i]) / rayDir[i];
		}else if(rayDir[i] < 0){
			tVals[i] = (boxMaxArray[i] - rayPos[i]) / rayDir[i];
		}
	}

	// Figure out if the x, y, or z is the largest value
	AF_FLOAT bestT = AF_GetMaxElement(tVals, 3);
	if(bestT < 0.0f){
		debugf("AF_Physics_Box_RayIntersection: no backwards ray\n");
		returnResult = FALSE; // no backwards rays 
		return returnResult;
	}

	Vec3 intersection = Vec3_ADD(_ray->position, Vec3_MULT_SCALAR(_ray->direction, bestT));
	float intersectionFloatArray[3] = {intersection.x, intersection.y, intersection.z};
	const AF_FLOAT epsilon = 0.0001f;
	for(int i = 0; i < 3; ++i){
		if(	intersectionFloatArray[i] + epsilon < boxMinArray[i] ||
			intersectionFloatArray[i] - epsilon > boxMaxArray[i]) {
				debugf("AF_Physics_Box_RayIntersection: best intersection doesn't touch box \n");
				returnResult = FALSE; // best intersection doesn't touch the box
				return returnResult;
			}
	}
	_collision->collisionPoint = intersection;
	_collision->rayDistance = bestT;
	returnResult = TRUE; 
	debugf("AF_Physics_Box_RayIntersection: result %i \n", returnResult);
	return returnResult;
} 

/*
====================
AF_PHYSICS_AABB_RAYINTERSECTION
Calculate ray intersection hit test against an Axis Aligned Bounding Box
====================
*/
static inline BOOL AF_Physics_AABB_RayIntersection(const Ray* _ray, AF_CCollider* _collider, AF_Collision* _collision){
	Vec3 boxPos = _collider->pos;
	Vec3* _size = &_collider->boundingVolume;
	//Vec3 boxHalfSize = Vec3_MULT_SCALAR(*_size, 0.5f);
	//Vec3 boxHalfSize = Vec3_DIV_SCALAR(*_size, 2);
	Vec3 boxHalfSize = Vec3_MULT_SCALAR(*_size, .5f);
	return AF_Physics_Box_RayIntersection(_ray, boxPos, boxHalfSize, _collision);
} 

/*
====================
AF_PHYSICS_OBB_RAYINTERSECTION
Calculate ray intersection hit test against a Object Orientated Box
====================
*/
static inline BOOL AF_Physics_OBB_RayIntersection(const Ray* _ray, const AF_CTransform3D* _worldTransform, const Vec3* _size, AF_Collision* _collision){

	//Vec4 orientation = {_worldTransform->rot.x, _worldTransform->rot.y, _worldTransform->rot.z, 1};
	//Vec3 postion = _worldTransform->pos;
	//TODO: implement correctly. p16 https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/physicstutorials/1raycasting/Physics%20-%20Raycasting.pdf#page=5.08
	/*
	Matrix3 transform = Matrix3 ( orientation );
	Matrix3 invTransform = Matrix3 ( orientation . Conjugate ());

	Vector3 localRayPos = r . GetPosition () - position ;

	Ray tempRay ( invTransform * localRayPos , invTransform * r . GetDirection ());

	bool collided = Ray Bo xI nt er se ct io n ( tempRay , Vector3 () ,
	volume . Ge tHalfD imensi ons () , collision );

	if ( collided ) {
		collision . collidedAt = transform * collision . collidedAt + position ;
	}
		return collided ;
	*/
	return false;
}



/*
====================
AF_PHYSICS_COLLISION_AABB
Simple aabb rect 
Return true if collission occured
====================
*/
static inline BOOL AF_Physics_AABB(AF_Rect* _rect1, AF_Rect* _rect2){
	//https://learnopengl.com/In-Practice/2D-Game/Collisions/Collision-detection#:~:text=AABB%20%2D%20AABB%20collisions,the%20x%20and%20y%20axis.
	BOOL returnValue = FALSE;
	// Collision x-axis?
	BOOL collisionX = _rect1->w >= _rect2->x &&
		_rect2->w >= _rect1->x;

	// Collision y-axis?
	BOOL collisionY = _rect1->h >= _rect2->y &&
		_rect2->h >= _rect1->y;

	if (collisionX && collisionY) {
		returnValue = TRUE;
	}
	// collsion only if on both axes
	return returnValue;
}

/*
====================
AF_PHYSICS_Point_Inside_Rect
Calculate ray intersection hit test against a Object Orientated Box
====================
*/
static inline BOOL AF_Physics_Point_Inside_Rect(Vec2 _point, AF_Rect _rect){
	BOOL returnValue = FALSE;
	if (_point.x >= _rect.x && _point.x <= (_rect.x + _rect.w) &&
        _point.y >= _rect.y && _point.y <= (_rect.y + _rect.h)) {
        returnValue = TRUE;
    }
	return returnValue;
}
/*
====================
AF_PHYSICS_Plane_RAYINTERSECTION
Calculate ray intersection hit test against a Object Orientated Box
====================
*/
static inline BOOL AF_Physics_Plane_RayIntersection(const Ray* _ray, AF_CCollider* _collider, AF_Collision* _collision){

	Vec3 planePos = _collider->pos;
	Vec3* _size = &_collider->boundingVolume;
	// assume a horizontal plane at planePos.y
	AF_FLOAT t = (planePos.y - _ray->position.y) / _ray->direction.y;

	BOOL returnValue = TRUE;

	if(t < 0){
		returnValue = FALSE;  // No intersection (plane is behind the ray or parallel)
		//debugf("AF_Physics_Box_RayIntersection: no intersection, plane is behind the ray \n");
		return returnValue;
	}

	Vec3 intersection = Vec3_ADD(_ray->position, Vec3_MULT_SCALAR(_ray->direction, t));
	

	// Check if the intersection is within the desired plane bounds (if the plane is finite)
	if(_size->y == 0 && _size->x == 0 && _size->z == 0){
		// bounds are all 0 so plane is counted as infinite.
		//debugf("AF_Physics_Plane_RayIntersection: intersection on infintie plane at x: %f y: %f z: %f \n", intersection.x, intersection.y, intersection.z);
		returnValue = TRUE;
		_collision->collisionPoint = intersection;
		_collision->rayDistance = t;
		return returnValue;
	}

	// Check if within size/bounds
	Vec2 intersectionPoint = {intersection.x, intersection.z};
	AF_Rect planeRect = {-_size->x/2, -_size->z/2, _size->x, _size->z};
	if(AF_Physics_Point_Inside_Rect(intersectionPoint, planeRect) == FALSE){
		//debugf("AF_Physics_Box_RayIntersection: no intersection, ray is out of bounds \nintX: %f intY: %f rectX:%f rectY:%f rectz:%f  \n", intersectionPoint.x, intersectionPoint.y, _size->x, _size->y, _size->z);
		returnValue = FALSE;
		return returnValue;
	}
	
	//debugf("AF_Physics_Plane_RayIntersection: intersection within bounds x: %f y: %f z: %f \n", intersection.x, intersection.y, intersection.z);
	returnValue = TRUE;
	
	// For example, if the plane is bounded within an XZ range, you can check here

	_collision->collisionPoint = intersection;
	_collision->rayDistance = t;

	
	return returnValue;
}



/*
====================
AF_PHYSICS_RAYINTERSECTION
Calculate ray intersection hit test
====================
*/
static inline BOOL AF_Physics_RayIntersection(const Ray* _ray, AF_Entity* _entity, AF_Collision* _collision){
	//const AF_CTransform3D* transform = _entity->transform;
	const AF_CCollider* collider = _entity->collider;
	enum CollisionVolumeType type = collider->type;

	switch(type){
		case Plane:
			return AF_Physics_Plane_RayIntersection(_ray, _entity->collider, _collision);
		break;
		case AABB:
			return AF_Physics_AABB_RayIntersection(_ray, _entity->collider, _collision);
		break;
		
		case OBB:
			debugf("AF_Physics_RayIntersection: OBB ray interaction not implemented\n");
			return false;
		break;

		case Sphere:
			return false;//AF_Physics_Sphere_RayIntersection(_ray, _entity->transform, _entity->collider, _collision);
		break;

		case Mesh:
			debugf("AF_Physics_RayIntersection: Mesh ray interaction not implemented\n");
			return false;
		break;

		case Compound:
			debugf("AF_Physics_RayIntersection: Compound ray interaction not implemented\n");
			return false;
		break;

		case Invalid:
			debugf("AF_Physics_RayIntersection: Invalid collider type\n");
			return false;
		break;
	}
	
	return false;
}


/*
====================
AF_Physics_ImpulseResolveCollision
Resolve collision between two rigidbodies
====================
*/
static inline void AF_Physics_ResolveCollision(AF_Entity* _entityA, AF_Entity* _entityB, AF_Collision* _collision){
	AF_C3DRigidbody* rigidbodyA = _entityA->rigidbody;
	AF_C3DRigidbody* rigidbodyB = _entityB->rigidbody;

	AF_CTransform3D* transformA = _entityA->transform;
	AF_CTransform3D* transformB = _entityB->transform;

	AF_CCollider* colliderA = _entityA->collider;
	AF_CCollider* colliderB = _entityB->collider;

	float totalMass;
	if (rigidbodyA->inverseMass == 0.0f && rigidbodyB->inverseMass == 0.0f) {
		totalMass = 0.0f;
		return;
	} else if (rigidbodyA->inverseMass == 0.0f) {
		totalMass = rigidbodyB->inverseMass;
	} else if (rigidbodyB->inverseMass == 0.0f) {
		totalMass = rigidbodyA->inverseMass;
	} else {
		totalMass = rigidbodyA->inverseMass + rigidbodyB->inverseMass;
	}

	// Seperate using projection
	//float entity1AdjMass = rigidbodyA->inverseMass / totalMass;
	//float entity2AdjMass = rigidbodyB->inverseMass / totalMass;

	float penetrationScale = 0.075f; // Adjust as needed

	//Vec3 adjustedPosition1 = Vec3_MINUS(transformA->pos, Vec3_MULT_SCALAR(colliderA->collision.normal, (colliderA->collision.penetration * entity1AdjMass * penetrationScale)));
	//Vec3 adjustedPosition2 = Vec3_MINUS(transformB->pos, Vec3_MULT_SCALAR(colliderB->collision.normal, (colliderB->collision.penetration * entity2AdjMass * penetrationScale)));
	//Vec3 adjustedPosition1 = Vec3_MINUS(transformA->pos, Vec3_MULT_SCALAR(colliderA->collision.normal, (colliderA->collision.penetration * penetrationScale)));
	//Vec3 adjustedPosition2 = Vec3_MINUS(transformB->pos, Vec3_MULT_SCALAR(colliderB->collision.normal, (colliderB->collision.penetration * penetrationScale)));

	//transformA->pos = adjustedPosition1;
	//transformB->pos = adjustedPosition2;
	if (rigidbodyA->inverseMass > 0.0f) {
    	transformA->pos = Vec3_MINUS(transformA->pos, Vec3_MULT_SCALAR(colliderA->collision.normal, (colliderA->collision.penetration * penetrationScale)));
	}
	if (rigidbodyB->inverseMass > 0.0f) {
		transformB->pos = Vec3_MINUS(transformB->pos, Vec3_MULT_SCALAR(colliderB->collision.normal, (colliderB->collision.penetration * penetrationScale)));
	}

	Vec3 relativeA = Vec3_MINUS(_collision->collisionPoint, transformA->pos);
	Vec3 relativeB = Vec3_MINUS(_collision->collisionPoint, transformB->pos);

	Vec3 angVelocityA = Vec3_CROSS(rigidbodyA->anglularVelocity, relativeA);
	Vec3 angVelocityB = Vec3_CROSS(rigidbodyB->anglularVelocity, relativeB);

	Vec3 fullVelocityA = Vec3_ADD(rigidbodyA->velocity, angVelocityA);
	Vec3 fullVelocityB = Vec3_ADD(rigidbodyB->velocity, angVelocityB);

	Vec3 contactVelocity = Vec3_MINUS(fullVelocityB, fullVelocityA);


	// Build up the impulse force
	float impulseForce = Vec3_DOT(contactVelocity, _collision->normal);

	// work out the effect of inertia
	Vec3 crossRelativeNormalA = Vec3_CROSS(relativeA, _collision->normal);
	Vec3 tensorCrossRelativeNormalA = Vec3_MULT(rigidbodyA->inertiaTensor, crossRelativeNormalA);
	Vec3 inertiaA = Vec3_CROSS(tensorCrossRelativeNormalA, relativeA);

	Vec3 crossRelativeNormalB = Vec3_CROSS(relativeB, _collision->normal);
	Vec3 tensorCrossRelativeNormalB = Vec3_MULT(rigidbodyB->inertiaTensor, crossRelativeNormalB);
	Vec3 inertiaB = Vec3_CROSS(tensorCrossRelativeNormalB, relativeB);

	float angularEffect = Vec3_DOT(Vec3_ADD(inertiaA, inertiaB), _collision->normal);

	float cRestitution = 0.66f; // disperse some kinectic energy
	
	float j = 0;

	float totalMassAngularEffect = totalMass + angularEffect;
	if(totalMassAngularEffect != 0.0f){
		j = (-(1.0f + cRestitution) * impulseForce) / (totalMass + angularEffect);
	}else{
		j = 0;
	}
	


	Vec3 fullImpulse = Vec3_MULT_SCALAR(_collision->normal, j);

	// apply linear and angualr impulses in opposite directions 
	Vec3 negativeFullImpulse = Vec3_MULT_SCALAR(fullImpulse, -1);

	if (rigidbodyA->inverseMass > 0.0f) {
		AF_Physics_ApplyLinearImpulse(rigidbodyA, negativeFullImpulse);
		AF_Physics_ApplyAngularImpulse(rigidbodyA,Vec3_CROSS(relativeA, negativeFullImpulse));
	}

	if (rigidbodyB->inverseMass > 0.0f) {
		AF_Physics_ApplyLinearImpulse(rigidbodyB, fullImpulse);
		AF_Physics_ApplyAngularImpulse(rigidbodyB,Vec3_CROSS(relativeB, fullImpulse));
	}
}



/*
====================
AF_PHYSICS_AABB_Test
Calculate ray intersection hit test
====================
*/
static inline BOOL AF_Physics_AABB_Test(AF_ECS* _ecs){
	// TODO:
	// implement cheaper nested for loop
	/*
	https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/physicstutorials/4collisiondetection/Physics%20-%20Collision%20Detection.pdf
	for int x = 0; x < lastObject ; ++ x {
		for int y = x +1; y < lastObject ; ++ y ) {
			if ( IsColliding (x , y )) {
				ResolveCollision (x , y )
			}
		}
	}
	*/
	BOOL returnValue = FALSE;
	for(int i = 0; i < _ecs->entitiesCount; ++i){

		if(AF_Component_GetEnabled(_ecs->colliders[i].enabled == FALSE)){
			continue;
		}
		AF_Entity* entity1 = &_ecs->entities[i];
		AF_CCollider* collider1 = entity1->collider;
		
		
		// rayIntersectionTest everything
		for(int x = 0; x < _ecs->entitiesCount; ++x){
			if(AF_Component_GetEnabled(_ecs->colliders[x].enabled == FALSE)){
				continue;
			}

			// check self
			if(i == x){
				continue;
			}

			AF_Entity* entity2 = &_ecs->entities[x];
			AF_CCollider* collider2 = entity2->collider;
		

			Vec3* posA = &_ecs->transforms[i].pos;
			Vec3* posB = &_ecs->transforms[x].pos;
			Vec3 halfSizeA = Vec3_MULT_SCALAR(collider1->boundingVolume, .5f);
			Vec3 halfSizeB = Vec3_MULT_SCALAR(collider2->boundingVolume, .5f);
			//Vec3 halfSizeA = Vec3_DIV_SCALAR(collider1->boundingVolume, 2);
			//Vec3 halfSizeB = Vec3_DIV_SCALAR(collider2->boundingVolume, 2);

			//Vec3 scaledHalfSizeA = Vec3_MULT(halfSizeA, transform1->scale);
			//Vec3 scaledHalfSizeB = Vec3_MULT(halfSizeB, transform2->scale);


			Vec3 delta = Vec3_MINUS(*posA, *posB);
			Vec3 totalSize = Vec3_ADD(halfSizeA, halfSizeB);
			//Vec3 totalSize = Vec3_ADD(scaledHalfSizeA, scaledHalfSizeB);

			if(
				abs(delta.x) < totalSize.x  &&
				abs(delta.y) < totalSize.y && 
				abs(delta.z) < totalSize.z){

					// TODO: determine from what direction the collision occurs
					//ResolveCollision(_ecs, i, x);
					returnValue = TRUE;
					// Resolve collision
					//AF_PHYSICS_CUBE_COLLISION_FACES
					// Get the min and max of each cube
					Vec3 maxA = Vec3_ADD(collider1->pos, collider1->boundingVolume);
					Vec3 minA = Vec3_MINUS(collider1->pos, collider1->boundingVolume);

					Vec3 maxB = Vec3_ADD(collider2->pos, collider2->boundingVolume);
					Vec3 minB = Vec3_MINUS(collider2->pos, collider2->boundingVolume);

					int facesCount = 6;
					float distances [facesCount];
					
						 distances[0] = maxB.x - minA.x; // distance of box ’b ’ to ’ left ’ of ’a ’.
						 distances[1] = maxA.x - minB.x; // distance of box ’b ’ to ’ right ’ of ’a ’.
						 distances[2] = maxB.y - minA.y; // distance of box ’b ’ to ’ bottom ’ of ’a ’.
						 distances[3] = maxA.y - minB.y; // distance of box ’b ’ to ’ top ’ of ’a ’.
						 distances[4] = maxB.z - minA.z; // distance of box ’b ’ to ’ far ’ of ’a ’.
						 distances[5] = maxA.z - minB.z;  // distance of box ’b ’ to ’ near ’ of ’a ’.
					
					//TODO: where is __FLT_MAX__ defined? may not be portable
					float penetration = __FLT_MAX__;
					Vec3 bestAxis = {0,0,0};	// default value
					for(int j = 0; j < facesCount; ++j){
						if(distances[j] < penetration){
							penetration = distances[j];
							bestAxis = AF_PHYSICS_CUBE_COLLISION_FACES[j]; 
						}
					}

					// create a new collision struct
					AF_Collision collision1 = {returnValue, entity1, entity2, collider1->collision.callback, {0,0,0}, 0.0f, bestAxis, penetration}; 
					// TODO: i think the bestAxis should be inverted for the second object
					AF_Collision collision2 = {returnValue, entity2, entity1, collider2->collision.callback, {0,0,0}, 0.0f, Vec3_MULT_SCALAR(bestAxis, -1), penetration}; 
					
					// copy the new struct values to each collider
					collider1->collision = collision1;
					collider2->collision = collision2;

					// TODO: move this outside the core rendering loop
					collider1->collision.callback(&collider1->collision);
					collider2->collision.callback(&collider2->collision);

					// Apply collision resolution
					//AF_CTransform3D* transform1 = &_ecs->transforms[i];
					//AF_CTransform3D* transform2 = &_ecs->transforms[x];

					//AF_C3DRigidbody* rigidbody1 = &_ecs->rigidbodies[i];
					//AF_C3DRigidbody* rigidbody2 = &_ecs->rigidbodies[x];
					// don't apply force for kinematic objects
					if(_ecs->entities[i].rigidbody->isKinematic){
						continue;
					}
					AF_Physics_ResolveCollision(entity1, entity2, &collision1);
			}
		}
	}
		
	return returnValue;
}

//=======BROAD / NARROW PHASE========

/*
Order of execution
AF_Physics_UpdateBroadphaseAABB
||
AF_Physics_BroadPhase
||
AF_Physics_NarrowPhase
*/

/**/
static inline void AF_Physics_UpdateBroadphaseAABB(AF_CCollider* _collider){
	if(_collider->type == AABB){
		Vec3 boundingVolumeHalfDimensions = {_collider->boundingVolume.x/2.0f, _collider->boundingVolume.y/2.0f, _collider->boundingVolume.z/2.0f};
		_collider->broadphaseAABB = boundingVolumeHalfDimensions;
	}
}

/*
static void AF_Physics_BroadPhase(AF_ECS* _ecs){
	// TODO: broadphaseCollisions.clear();
	Vec2 treeSize = {1024, 1024};
	//QuadTree tree;

	for(int i = 0; i < _ecs->entitiesCount; ++i){
		Vec3 halfSizes;

		Vec3 pos = _ecs->transforms[i].pos;
		//QuadTree_Node* node = {NULL, 1, {pos.x, pos.y}, {halfSizes.x, halfSizes.y}, NULL};
		
		// Insert into our quad tree the node, its position and its half size bounds
		//AF_QuadTree_Insert(node, &_ecs->entities[i], &pos, &halfSizes, 7, 6);
		// TODO: actually insert into the quad tree
		//AF_QuadTree_Insert()

		// determine what objects may be colliding.
		
		tree.OperateOnContents([&](std::list < QuadTreeEntry < GameObject * > >& data ) {
		 CollisionInfo info ;
		
		for ( auto i = data . begin (); i != data . end (); ++ i ) {
			for ( auto j = std :: next ( i ); j != data . end (); ++ j ) {
		// is this pair of items already in the collision set -
		// if the same pair is in another quadtree node together etc
		info.a = min ((* i ).object , (* j ).object );
		info.b = max ((* i ).object , (* j ).object );
		broadphaseCollisions.insert ( info );
		

	}
}

*/

// Function to compare two CollisionInfo objects based on their hashes
static inline BOOL AF_Physics_CollisionInfoLessThan(const AF_Collision* info1, const AF_Collision* info2) {
    // Calculate hash for the first CollisionInfo
    size_t hash1 = (size_t)info1->entity1 + ((size_t)info1->entity2 << 8);
    // Calculate hash for the second CollisionInfo
    size_t hash2 = (size_t)info2->entity1 + ((size_t)info2->entity2 << 8);

    // Return true if the hash of info1 is less than that of info2
    return (hash1 < hash2);
}



// Function to handle narrow phase collision detection
static inline void AF_Physics_NarrowPhase(AF_Collision* broadPhaseCollisions, size_t collisionCount, int numCollisionFrames) {
    //AF_Collision allCollisions[1024]; // Example array to store all collisions
    //size_t allCollisionsCount = 0;       // Counter for all collisions

    // Iterate through the broad phase collisions
    //for (size_t i = 0; i < collisionCount; ++i) {
        //AF_Collision info = broadPhaseCollisions[i];

        // Check if the objects intersect
		// TODO: implement this
		/*
        if (ObjectIntersection(info.entity2, info.entity1, &info)) {
			//TODO: implement framesLeft
            //info.framesLeft = numCollisionFrames; // Set frames left
            ImpulseResolveCollision(info.entity1, info.entity2, info.collisionPoint); // Resolve collision
            allCollisions[allCollisionsCount++] = info; // Store collision info
        }*/
    //}

    // Optionally, you can print or process allCollisions here
    //printf("Total collisions: %zu\n", allCollisionsCount);
}





//=================


static inline void AF_Physics_DrawBox(AF_CCollider* collider, float* color){
	// render debug collider
                //draw all edges
                //if(collider->type == Plane){
	Vec3 pos = collider->pos;//_ecs[i].transforms->pos;
	Vec3 bounds = collider->boundingVolume;
	// Top
	/*
	Vec3 top_bottomLeft = {pos.x - bounds.x/2, pos.x + bounds.x/2, pos.z - bounds.z/2};
	Vec3 top_topLeft =  {pos.x - bounds.x/2, pos.x + bounds.x/2, pos.z + bounds.z/2};
	Vec3 top_topRight =  {pos.x + bounds.x/2, pos.x + bounds.x/2, pos.z + bounds.z/2};
	Vec3 top_bottomRight =  {pos.x + bounds.x/2, pos.x + bounds.x/2, pos.z - bounds.z/2};
	*/
	
	const int verticesCount = 24;  // Total lines: 12 edges * 2 vertices per edge
	Vec3 vertices[verticesCount];

	// Top face vertices
	vertices[0] = (Vec3){pos.x - bounds.x/2, pos.y + bounds.y/2, pos.z - bounds.z/2};  // top-bottomLeft
	vertices[1] = (Vec3){pos.x - bounds.x/2, pos.y + bounds.y/2, pos.z + bounds.z/2};  // top-topLeft

	vertices[2] = (Vec3){pos.x - bounds.x/2, pos.y + bounds.y/2, pos.z + bounds.z/2};  // top-topLeft
	vertices[3] = (Vec3){pos.x + bounds.x/2, pos.y + bounds.y/2, pos.z + bounds.z/2};  // top-topRight

	vertices[4] = (Vec3){pos.x + bounds.x/2, pos.y + bounds.y/2, pos.z + bounds.z/2};  // top-topRight
	vertices[5] = (Vec3){pos.x + bounds.x/2, pos.y + bounds.y/2, pos.z - bounds.z/2};  // top-bottomRight

	vertices[6] = (Vec3){pos.x + bounds.x/2, pos.y + bounds.y/2, pos.z - bounds.z/2};  // top-bottomRight
	vertices[7] = (Vec3){pos.x - bounds.x/2, pos.y + bounds.y/2, pos.z - bounds.z/2};  // top-bottomLeft

	// Bottom face vertices
	vertices[8]  = (Vec3){pos.x - bounds.x/2, pos.y - bounds.y/2, pos.z - bounds.z/2};  // bottom-bottomLeft
	vertices[9]  = (Vec3){pos.x - bounds.x/2, pos.y - bounds.y/2, pos.z + bounds.z/2};  // bottom-topLeft

	vertices[10] = (Vec3){pos.x - bounds.x/2, pos.y - bounds.y/2, pos.z + bounds.z/2};  // bottom-topLeft
	vertices[11] = (Vec3){pos.x + bounds.x/2, pos.y - bounds.y/2, pos.z + bounds.z/2};  // bottom-topRight

	vertices[12] = (Vec3){pos.x + bounds.x/2, pos.y - bounds.y/2, pos.z + bounds.z/2};  // bottom-topRight
	vertices[13] = (Vec3){pos.x + bounds.x/2, pos.y - bounds.y/2, pos.z - bounds.z/2};  // bottom-bottomRight

	vertices[14] = (Vec3){pos.x + bounds.x/2, pos.y - bounds.y/2, pos.z - bounds.z/2};  // bottom-bottomRight
	vertices[15] = (Vec3){pos.x - bounds.x/2, pos.y - bounds.y/2, pos.z - bounds.z/2};  // bottom-bottomLeft

	// Vertical edges connecting top and bottom faces
	vertices[16] = (Vec3){pos.x - bounds.x/2, pos.y + bounds.y/2, pos.z - bounds.z/2};  // top-bottomLeft
	vertices[17] = (Vec3){pos.x - bounds.x/2, pos.y - bounds.y/2, pos.z - bounds.z/2};  // bottom-bottomLeft

	vertices[18] = (Vec3){pos.x - bounds.x/2, pos.y + bounds.y/2, pos.z + bounds.z/2};  // top-topLeft
	vertices[19] = (Vec3){pos.x - bounds.x/2, pos.y - bounds.y/2, pos.z + bounds.z/2};  // bottom-topLeft

	vertices[20] = (Vec3){pos.x + bounds.x/2, pos.y + bounds.y/2, pos.z + bounds.z/2};  // top-topRight
	vertices[21] = (Vec3){pos.x + bounds.x/2, pos.y - bounds.y/2, pos.z + bounds.z/2};  // bottom-topRight

	vertices[22] = (Vec3){pos.x + bounds.x/2, pos.y + bounds.y/2, pos.z - bounds.z/2};  // top-bottomRight
	vertices[23] = (Vec3){pos.x + bounds.x/2, pos.y - bounds.y/2, pos.z - bounds.z/2};  // bottom-bottomRight

	AF_Debug_DrawLineArrayWorld(vertices, verticesCount, color, FALSE);
}

#ifdef __cplusplus
}
#endif

#endif //AF_PHYSICS_H
