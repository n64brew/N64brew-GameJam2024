/*
===============================================================================
AF_ECS_H defninitions
Entity Component System used to process entities and manage them
Effectively the scene graph 
New AF_ECS struct objects can be created to hold the entities for each scene.
===============================================================================
*/

#ifndef AF_ECS_H
#define AF_ECS_H
#include <stdio.h>
#include <assert.h>
#include "AF_Entity.h"
#include "ECS/Components/AF_Component.h"

#define AF_ECS_TOTAL_ENTITIES 65

/*
====================
AF_ECS
Entity Component System struct
Used to hold all the entities
====================
*/
typedef struct {
    uint32_t entitiesCount;
    uint32_t currentEntity;
    // TODO: don't ifdef, be smarter
    AF_Entity entities[AF_ECS_TOTAL_ENTITIES];
    AF_CSprite sprites[AF_ECS_TOTAL_ENTITIES];		// sprite cmponent
    
    #ifdef PLATFORM_GB
    AF_C2DRigidbody rigidbodies[AF_ECS_TOTAL_ENTITIES];	// rigidbody component
    AF_CTransform2D transforms[AF_ECS_TOTAL_ENTITIES];
    AF_CCollider colliders[AF_ECS_TOTAL_ENTITIES];	// Collider component

    #else
    AF_CTransform3D transforms[AF_ECS_TOTAL_ENTITIES];	// 3d transform component
    AF_C3DRigidbody rigidbodies[AF_ECS_TOTAL_ENTITIES];	// rigidbody component
	AF_CCollider colliders[AF_ECS_TOTAL_ENTITIES];	// Collider component


    
    AF_CAnimation animations[AF_ECS_TOTAL_ENTITIES];	// animation Component
    AF_CMesh meshes[AF_ECS_TOTAL_ENTITIES];		// mesh component 	// TODO: turn this into a component type
	AF_CText texts[AF_ECS_TOTAL_ENTITIES];
	AF_CAudioSource audioSources[AF_ECS_TOTAL_ENTITIES];
	AF_CPlayerData playerDatas[AF_ECS_TOTAL_ENTITIES];
	AF_CSkeletalAnimation skeletalAnimations[AF_ECS_TOTAL_ENTITIES];
	AF_CAI_Behaviour aiBehaviours[AF_ECS_TOTAL_ENTITIES];
        #endif
} AF_ECS;

/*
====================
AF_ECS_Init
Init helper function to initialise all the entities.
Entities are all loaded into memory at the start using the compile time define AF_ECS_TOTAL_ENTITIES
====================
*/
static inline void AF_ECS_Init(AF_ECS* _ecs){
	assert(_ecs != NULL && "AF_ECS_Init: argument is null");
	// Initialise all entities in the entity pool with default values
	for(uint32_t i = 0; i < AF_ECS_TOTAL_ENTITIES; i++){
		AF_Entity* entity = &_ecs->entities[i];
		flag_t* componentState = &entity->flags;
 		//entity->enabled = TRUE;
		entity->flags = AF_Component_SetEnabled(*componentState, TRUE);
		entity->id_tag = AF_ECS_AssignID(entity->id_tag, i);
		entity->id_tag = AF_ECS_AssignTag(entity->id_tag, 0);
			
		// include gb components
		
		// Sprite Components
		_ecs->sprites[i] = AF_CSprite_ZERO();
		entity->sprite = &_ecs->sprites[i];
		
		#ifdef PLATFORM_GB
		// Tranform
	        _ecs->transforms[i] = AF_CTransform2D_ZERO();
		entity->transform = &_ecs->transforms[i];


		// Rigidbody Component
		_ecs->rigidbodies[i] = AF_C2DRigidbody_ZERO();
		entity->rigidbody = &_ecs->rigidbodies[i];


		#else
		// Transform Component
		entity->parentTransform = NULL;
		
		// Transform Component
		_ecs->transforms[i] = AF_CTransform3D_ZERO();
		entity->transform = &_ecs->transforms[i];
		

		// Rigidbody3D
		_ecs->rigidbodies[i] = AF_C3DRigidbody_ZERO();
		entity->rigidbody = &_ecs->rigidbodies[i];

		// Colliders
		_ecs->colliders[i] = AF_CCollider_ZERO();
		entity->collider = &_ecs->colliders[i];
		
		// Animation component
		_ecs->animations[i] = AF_CAnimation_ZERO();
		entity->animation = &_ecs->animations[i];

		// Add Meshes
		_ecs->meshes[i] = AF_CMesh_ZERO();
		entity->mesh = &_ecs->meshes[i];

		// Add text
		_ecs->texts[i] = AF_CText_ZERO();
		entity->text = & _ecs->texts[i];

		// Add audio
		_ecs->audioSources[i] = AF_CAudioSource_ZERO();
		entity->audioSource = &_ecs->audioSources[i];

		// player data
		_ecs->playerDatas[i] = AF_CPlayerData_ZERO();
		entity->playerData = &_ecs->playerDatas[i];

		// skeletal animations
		_ecs->skeletalAnimations[i] = AF_CSkeletalAnimation_ZERO();
		entity->skeletalAnimation = &_ecs->skeletalAnimations[i];

		// ai Behaviours
		_ecs->aiBehaviours[i] = AF_CAI_Behaviour_ZERO();
		entity->aiBehaviour = &_ecs->aiBehaviours[i];
		
		// Camera Component
		entity->camera = NULL;//AF_CCamera_ZERO();
				
		#endif
	}
	_ecs->entitiesCount = AF_ECS_TOTAL_ENTITIES;
}

/*
====================
AF_ECS_CreateEntity
Helper function to enable the entity and pass on a pointer reference to it
All entities already exist in memory so this just enables it.
====================
*/
static inline AF_Entity* AF_ECS_CreateEntity(AF_ECS* _ecs){
	assert(_ecs != NULL && "AF_ECS_CreateEntity: argument is null");
	assert(_ecs->currentEntity <= _ecs->entitiesCount && "AF_ECS_CreateEntity: ECS: Ran out of entities !!!\n");

    // increment the entity count and return the reference to the next available entity
    _ecs->currentEntity++;

    AF_Entity* entity = &_ecs->entities[_ecs->currentEntity];
    entity->id_tag = AF_ECS_AssignID(entity->id_tag, _ecs->currentEntity);
    PACKED_CHAR* componentState = &entity->flags;
    //entity->enabled = TRUE;
    entity->flags = AF_Component_SetEnabled(*componentState, TRUE);

    //entity->enabled = true;
    // Give this entity a default transform component that is enabled
    // TODO: don't if def
    #ifdef PLATFORM_GB
	// include gb components
    *entity->transform = AF_CTransform2D_ADD();
    #else
    *entity->transform = AF_CTransform3D_ADD();
    #endif

    if(entity == NULL){
		printf("AF_ECS: AF_ECS_CreateEntity failed, and is returining a null entity\n");
	}
    return entity;
}



//void AF_RemoveEntity(Entity _entity);
void AF_ECS_Update(AF_Entity* _entities);



#endif //AF_ECS_H
