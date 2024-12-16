/*
===============================================================================
AF_ACTOR_H defninitions
The actor struct and helper functions
A pre-defined sruct that is game specific
===============================================================================
*/
#ifndef AF_ACTOR_H
#define AF_ACTOR_H
#include "AF_Lib_Define.h"
#include "AF_Vec3.h"
#include "AF_Vec2.h"
#include "AF_Collision.h"
#include "ECS/Entities/AF_ECS.h"
#include "ECS/Components/AF_CSprite.h"

#ifndef PLATFORM_GB
#include "AF_Mesh.h"
#include "AF_Material.h"

/*
====================
AF_3DACTOR struct
Contains a copy of each component type
All components sizes are known at compile time 
except for AF_Mesh which loads verts and indices from a file
If 3D game then loaded verts are known at compile time as its just a quad hard coded
====================
*/
typedef struct {
	flag_t tag;
	BOOL startEnabled;
	Vec3 pos;
	Vec3 scale;
	Vec2 spritePos;
	Vec2 spriteSize;
	Vec2 spriteSheetSize;
	AF_FLOAT animationSpeed;
	uint32_t animationFrames;
	BOOL animationLoop;
	AF_Mesh mesh;
	AF_Material material;
	BOOL isKinematic;
        void (*collisionCallback)(struct AF_Collision);

} AF_3DActor;
#endif

/*
====================
AF_2DACTOR struct
Contains a copy of each component type
All components sizes are known at compile time 
except for AF_Mesh which loads verts and indices from a file
====================
*/
typedef struct {
	flag_t tag;
	BOOL startEnabled;
	Vec2 pos;
	Vec2 scale;
	Vec2 spritePos;
	Vec2 spriteSize;
	Vec2 spriteSheetSize;
	AF_FLOAT animationSpeed;
	uint8_t animationFrames;
	BOOL animationLoop;
#ifndef PLATFORM_GB
	AF_Mesh mesh;
	AF_Material material;
#endif
	BOOL isKinematic;
        void (*collisionCallback)(struct AF_Collision);

} AF_2DActor;

/*
====================+
AF_Actor_Create2DActor
Create game actors with some default settings
Take in an actor struct and pas back the entity with all the components setup
*====================
*/
static inline AF_Entity* AF_Actor_Create2DActor(AF_ECS* _ecs, const AF_2DActor _actor){

    AF_Entity* _entity = AF_ECS_CreateEntity(_ecs);
    // Set the tag
    _entity->id_tag = AF_ECS_AssignTag(_entity->id_tag, _actor.tag);

    // Set the pos
    _entity->transform->pos = _actor.pos;

    // starting enabled
    _entity->flags = AF_Component_SetEnabled(_entity->flags, _actor.startEnabled);

    // Set the scale
    _entity->transform->scale = _actor.scale;
    // set the sprite data. Can't use compound literals as GB uses older version for c < 99 :(
        PACKED_CHAR component = TRUE;
	component = AF_Component_SetHas(component, TRUE);
	component = AF_Component_SetEnabled(component, TRUE);
    	AF_CSprite* sprite = _entity->sprite; 
	sprite->enabled = component;
	sprite->loop = _actor.animationLoop; 
	sprite->currentFrame = 0;
	sprite->animationFrames = _actor.animationFrames;
	sprite->nextFrameTime = 0;
	sprite->animationSpeed = _actor.animationSpeed;
	sprite->pos.x = _actor.spritePos.x;
	sprite->pos.y = _actor.spritePos.y;
	sprite->size.x = _actor.spritePos.x;
	sprite->size.y = _actor.spritePos.y;
	sprite->spriteSheetSize.x = _actor.spriteSheetSize.x;
	sprite->spriteSheetSize.y = _actor.spriteSheetSize.y;
/*
    // Add the animation component
    *_entity->animation = AF_CAnimation_ADD();
    
    
#ifndef PLATFORM_GB	
    // set the mesh
    *_entity->mesh = _actor.mesh;

    // set the material
    _entity->mesh->material = _actor.material;
#endif
    // set the regidbody
    *_entity->rigidbody = AF_CRigidbody_ADD();

    // set the collider
    *_entity->collider = AF_CCollider_ADD();

    // set the collision callback
    _entity->collider->collision.callback = _actor.collisionCallback;
*/
    //PACKED_CHAR entityComponent = _entity->enabled;
    return _entity;
}


/*
====================+
AF_Actor_Create3dActor
Create game actors with some default settings
Take in an actor struct and pas back the entity with all the components setup
*====================
*/
#ifndef PLATFORM_GB
static inline AF_Entity* AF_Actor_Create3DActor(AF_ECS* _ecs, const AF_3DActor _actor){

    AF_Entity* _entity = AF_ECS_CreateEntity(_ecs);
    // Set the tag
    _entity->id_tag = AF_ECS_AssignTag(_entity->id_tag, _actor.tag);

    // Set the pos
    _entity->transform->pos = _actor.pos;

    // starting enabled
    _entity->flags = AF_Component_SetEnabled(_entity->flags, _actor.startEnabled);

    // Set the scale
    _entity->transform->scale = _actor.scale;

    // set the sprite pos
    *_entity->sprite = AF_CSprite_ADD();
    
    // set the sprite size
    _entity->sprite->size = _actor.spriteSize;

    // set the spritesheet starting pos
    _entity->sprite->pos = _actor.spritePos;

    // set the sprite sheet size 
    _entity->sprite->spriteSheetSize = _actor.spriteSheetSize;

    // Add the animation component
    *_entity->animation = AF_CAnimation_ADD();
    
    // Set the animation speed and time
    _entity->animation->animationSpeed = _actor.animationSpeed;

    // set the animation frames
    _entity->animation->animationFrames = _actor.animationFrames;

    // set the animation loop
    _entity->animation->loop = _actor.animationLoop;
    // set the mesh
    *_entity->mesh = _actor.mesh;

    // set the material
    _entity->mesh->material = _actor.material;
    // set the regidbody
    *_entity->rigidbody = AF_CRigidbody_ADD();

    // set the collider
    *_entity->collider = AF_CCollider_ADD();

    // set the collision callback
    _entity->collider->collision.callback = _actor.collisionCallback;

    //PACKED_CHAR entityComponent = _entity->enabled;
    
    //AF_Log(" 
    
    return _entity;
}
#endif
#endif // ACTOR_H
