/*
===============================================================================
AF_CMesh_H

Header only Mesh loading
functions to load meshes, creating memory on the heap based on the size of the mesh

===============================================================================
*/
#ifndef AF_CMESH_H
#define AF_CMESH_H
#include "AF_Vertex.h"
#include "AF_Lib_Define.h"
#include "AF_Material.h"
#include "ECS/Components/AF_Component.h"

#ifdef __cplusplus
extern "C" {
#endif  

// Inspired by Tiny3D fixed point matrix
// 3D s16.16 fixed-point vector, used as-is by the RSP (n64)
typedef struct {
  int16_t i[4];
  uint16_t f[4];
} Vec4FP;

// 4x4 Matrix of 16.16 fixed-point numbers, used as-is by the RSP (n64)
typedef struct {
  Vec4FP m[4];
} __attribute__((aligned(16))) Mat4FP;

enum AF_MESH_TYPE{
	AF_MESH_TYPE_PLANE, 
	AF_MESH_TYPE_CUBE,
	AF_MESH_TYPE_SPHERE,
	AF_MESH_TYPE_MESH
};





// Mesh Struct
typedef struct {
    //BOOL has;
    PACKED_CHAR enabled;
    AF_Vertex* vertices;
    uint16_t vertexCount;
    uint32_t* indices;
    int indexCount;
    // TODO: pack this
    uint32_t vao;
    uint32_t vbo;
    uint32_t ibo;
    AF_Material material;
    BOOL showDebug;
	enum AF_MESH_TYPE meshType;
	uint8_t meshID;	// only fit 255 mesh types
	BOOL isAnimating;
	void* modelMatrix;
	void* displayListBuffer;
} AF_CMesh;

/*
====================
AF_CMesh_ZERO
Function used to create an empty mesh component
====================
*/
static inline AF_CMesh AF_CMesh_ZERO(void){
    AF_CMesh returnMesh = {
	//.has = FALSE,
	.enabled = FALSE,
	.vertices = NULL,
	.vertexCount = 0,
	.indices = NULL,
	.indexCount = 0,
	.vao = 0,
	.vbo = 0,
	.ibo = 0,
	.material = {0,0},
	.showDebug = FALSE,
	.meshType = AF_MESH_TYPE_PLANE,
	.meshID = 0,
	.isAnimating = FALSE,
	.modelMatrix = NULL,
	.displayListBuffer = NULL
	};
    return returnMesh;
}

/*
====================
AF_CMesh_ADD
Function used to Add the component
====================
*/
static inline AF_CMesh AF_CMesh_ADD(void){
    PACKED_CHAR component = TRUE;
    component = AF_Component_SetHas(component, TRUE);
    component = AF_Component_SetEnabled(component, TRUE);

    AF_CMesh returnMesh = {
	//.has = TRUE,
	.enabled = component,
	.vertices = NULL,
	.vertexCount = 0,
	.indices = NULL,
	.indexCount = 0,
	.vao = 0,
	.vbo = 0,
	.ibo = 0,
	.material = {0,0},
	.showDebug = FALSE,
	.meshType = AF_MESH_TYPE_PLANE,
	.meshID = 0,
	.isAnimating = FALSE,
	.modelMatrix = NULL,
	.displayListBuffer = NULL
	};
    return returnMesh;
}



#ifdef __cplusplus
}
#endif  

#endif  // AF_CMESH_H
