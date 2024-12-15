/*
===============================================================================
AF_MeshData_H

===============================================================================
*/
#ifndef AF_MESH_DATA_H
#define AF_MESH_DATA_H
#include "AF_Mesh.h"
#include "AF_Material.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    // array of meshes
    AF_CMesh* meshes;
    AF_Material* materials;
    uint32_t numMeshes;
    uint32_t materialsSize;
    uint32_t vao;
    uint32_t vbo;
    uint32_t ibo;
    uint32_t shaderID;
} AF_MeshData;

inline int AF_MeshData_Destroy(AF_MeshData* _meshData){
    if(_meshData == NULL){
        return 0;
    }
    free(_meshData->meshes);
    free(_meshData->materials);
    return 1;
}

#ifdef __cplusplus
}
#endif

#endif //AF_MESH_DATA_H
