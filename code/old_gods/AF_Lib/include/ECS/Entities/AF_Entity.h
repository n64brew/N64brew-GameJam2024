/*
===============================================================================
AF_Entity_H defninitions
The entity struct and helper functions
===============================================================================
*/

#ifndef AF_Entity_H
#define AF_Entity_H
//#include "AF_ECS.h"
#include "AF_Lib_Define.h"
#include "ECS/Components/AF_CSprite.h"
#include "ECS/Components/AF_CCollider.h"
#include "ECS/Components/AF_CText.h"
#include "ECS/Components/AF_CAudioSource.h"
#include "ECS/Components/AF_CPlayerData.h"
#include "ECS/Components/AF_CSkeletalAnimation.h"
#include "ECS/Components/AF_CAI_Behaviour.h"

// Components
#ifdef PLATFORM_GB
#include "ECS/Components/AF_CTransform2D.h"
#include "ECS/Components/AF_C2DRigidbody.h"

#else
#include "ECS/Components/AF_C3DRigidbody.h"
#include "ECS/Components/AF_CMesh.h"
#include "ECS/Components/AF_CTransform3D.h"
#include "ECS/Components/AF_CAnimation.h"
#include "ECS/Components/AF_CCamera.h"
#endif



/*
====================
AF_Entity
Entity struct
Contains a copy of each component type
All components sizes are known at compile time 
except for AF_Mesh which loads verts and indices from a file
If 2D game then loaded verts are known at compile time as its just a quad hard coded
====================
*/
// Size of struct is exactly 64 bytes
typedef struct {
    flag_t flags;	// Entity has ben enabled
    PACKED_UINT32 id_tag;		// Packed datatype holding both a tag and ID. id of the entity. ID can be 0 to 536, 870, 911, tag holds up to 8 variants

    #ifdef PLATFORM_GB

    char packed1;		// pack out an extra byte
    uint16_t packed2;		// pack out and extra 2 bytes
    AF_CTransform2D* transform;
    AF_CSprite* sprite;		// sprite cmponent
    AF_C2DRigidbody* rigidbody;	// rigidbody component
    AF_CCollider* collider;	// Collider component

    #else
    AF_CTransform3D* parentTransform;
    AF_CTransform3D* transform;	// 3d transform component
    AF_CSprite* sprite;		// sprite cmponent
    AF_C3DRigidbody* rigidbody; // 3d rigidbody
    AF_CCollider* collider;	// Collider component
    AF_CAnimation* animation;	// animation Component
    AF_CCamera* camera;		// camera component
    AF_CMesh* mesh;		// mesh component 	// TODO: turn this into a component type
    AF_CText* text;
    AF_CAudioSource* audioSource;
    AF_CPlayerData* playerData;
    AF_CSkeletalAnimation* skeletalAnimation;
    AF_CAI_Behaviour* aiBehaviour;
        #endif
} AF_Entity;

// Little helper struct that can be use
typedef struct AF_EntityPair {
    AF_Entity* entity1;
    AF_Entity* entity2;
} EntityPair;

static inline PACKED_UINT32 AF_ECS_GetTag(PACKED_UINT32 _id_tag){
	// Unpacking the packed uint32_t
	return _id_tag & 0x7;
}

static inline PACKED_UINT32 AF_ECS_GetID(PACKED_UINT32 _id_tag){
	// unpacking the packed uint32_t
	return (_id_tag >> 3) & 0x1FFFFFFF;
}

static inline PACKED_UINT32 AF_ECS_AssignTag(PACKED_UINT32 _id_tag, PACKED_UINT32 _tagValue){
	// update the tag value in the packed uint32, keeping the id intact
	PACKED_UINT32 returnTag = (_tagValue & 0x7) | ((AF_ECS_GetID(_id_tag) & 0x1FFFFFFF) << 3);
	return returnTag;
}

static inline PACKED_UINT32 AF_ECS_AssignID(PACKED_UINT32 _id_tag, PACKED_UINT32 _idValue){
	// update the id value in the packed uint32, keeping the tag intact
	PACKED_UINT32 returnID = (AF_ECS_GetTag(_idValue) & 0x7) | ((_id_tag & 0x1FFFFFFF) << 3);
	return returnID;
}


#endif //AF_Entity_H
