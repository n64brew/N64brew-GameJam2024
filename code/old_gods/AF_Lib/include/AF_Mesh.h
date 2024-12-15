/*
===============================================================================
AF_Mesh_H

Header only Mesh loading
functions to load meshes, creating memory on the heap based on the size of the mesh

===============================================================================
*/
#ifndef AF_MESH_H
#define AF_MESH_H
#include "AF_Vertex.h"
#include "AF_Lib_Define.h"
#include "AF_Util.h"
//#include "AF_Log.h"
#include "AF_Material.h"
#include "ECS/Components/AF_Component.h"
#include "ECS/Components/AF_CMesh.h"
#ifdef __cplusplus
extern "C" {
#endif  

#define AF_MESH_FILE_TITLE "AF_Mesh: "


/*
====================
AF_Mesh_GetVec2FromString
Take in a char array and return the float values 
Extract and return the first two float values found
Used for extracting things like tex coords out of a .obj
====================
*/
static inline Vec2 AF_Mesh_GetVec2FromString(char* _buffer){ //, uint16_t _size){
    #ifdef PLATFORM_GB
	if(_buffer){}
	printf("TODO: implement custom strtok, as sdcc doesn't support it \n");
	Vec2 returnEmpty = {0,0};
        return returnEmpty;
    #else
    Vec2 returnVec2 = {0.0f, 0.0f};
    char* token = strtok(_buffer, " ");
    token = strtok(NULL, " ");
    if(token != NULL){
        AF_FLOAT num1 = strtof(token, NULL);
        token = strtok(NULL, " ");
        AF_FLOAT num2 = strtof(token, NULL);
        //AF_Log("Extracted Vec2 float %f %f \n", num1, num2);
        
        returnVec2.x = num1;
        returnVec2.y = num2;
    }
    // search the char buffer for the first two number characters and convet it to a vec2
   
    return returnVec2;
#endif
}

/*
====================
AF_Mesh_GetVec3FromString
Take in a char array and return the float values 
Extract and return the first 3 float values
Used for things like position values out of a .obj
====================
*/
static inline Vec3 AF_Mesh_GetVec3FromString(char* _buffer){ //, uint16_t _size){
    	
    Vec3 returnVec3 = {0.0f, 0.0f, 0.0f};
#ifdef PLATFORM_GB
	if(_buffer){}
	printf("TODO: implment a AF_Mesh_GetVec3FromString: that doesn't use strok for use on Platform GB\n");
	return returnVec3;
#else
    char* token = strtok(_buffer, " ");
    token = strtok(NULL, " ");
    if(token != NULL){
        AF_FLOAT num1 = strtof(token, NULL);
        token = strtok(NULL, " ");
        AF_FLOAT num2 = strtof(token, NULL);
        token = strtok(NULL, " ");
        AF_FLOAT num3 = strtof(token, NULL);
        //AF_Log("Extracted Vec3 float %f %f %f \n", num1, num2, num3);
        
        returnVec3.x = num1;
        returnVec3.y = num2;
        returnVec3.z = num3;
    }
    // search the char buffer for the first two number characters and convet it to a vec2
   
    return returnVec3;
#endif
}

/*
====================
AF_Mesh_GetOBJAttribSize
Read a file, ideally in .obj format
extract each line, and determine how many verticies and indicies are in the 3d model
return a copy of a mesh struct with the verticies and indicies count values assigned. 
DOES NOT load the actual verticies or indices or create new memory on the heap.
====================
*/
static inline AF_CMesh AF_Mesh_GetOBJAttribSize(void* _filePtr){
	AF_CMesh mesh = {
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
		.showDebug = FALSE
	    };

    #ifdef PLATFORM_GB
	if(_filePtr){}
	printf("TODO: re-implement AF_Mesh_GetOBJAttribSize: to support portable FILE type\n");
	return mesh;
    #else
	// re-cast the void ptr to FILE type as its safe to do so
        FILE* _file = (FILE*)_filePtr;
        if(_file == NULL){
        AF_Log_Error("%s Failed to open file to GetOBJAttribSize %s\n", AF_MESH_FILE_TITLE);
        return mesh;
    }

        // read the file and count
        // num of "v" verticies found 
        // num of "vt" texture coordsfound
        // num of "vn" normal vectors found
    char fileBuffer[1024] ; // Buffer for reading the file
    
    // read the file
    while(!feof(_file)){
        
         if(fgets(fileBuffer, 1024, _file) != NULL) {
        // print the return value (aka string read in) to terminal
            //AF_Vertex vertex ;
            if(fileBuffer[0] == 'v' && fileBuffer[1] == ' '){
                // vertex
                //AF_Vec3 vertPos = AF_Mesh_GetVec3FromString(fileBuffer);
                //vertex.position = vertPos;
                mesh.vertexCount++;
                continue;
            }
            if(fileBuffer[0] == 'f'){
                // face / indices
                // add 4 faces to the index count
                // search the first element of a new face token and store the value

                // cound the indicies
                char *p = strtok(fileBuffer, " ");
                while(p != NULL) {
                    if((strcmp(p,"f"))){
                        mesh.indexCount++;
                    }
                    //printf("%s\n", p);
                    // Move the tokens along
                    p = strtok(NULL, " ");
                }
                continue;
            }
            if(fileBuffer[0] == 'v' && fileBuffer[1] == 't'){
                // texture coordinate
                continue;
            }
            if(fileBuffer[0] == 'v' && fileBuffer[1] == 'n'){
                // normal
                continue;
            }
        }
    }
    // Move back to the beginning of the file using fseek and rewind
    fseek(_file, 0L, SEEK_SET);
    rewind(_file);
    
    return mesh;
#endif
}

/*
====================
AF_Mesh_Load_Data
Read a file, and perform needed operations to extract the data that hopefully is .obj format
This function relies on taking in a mesh object that already has the number of verticies and indicies assigned to a AF_Mesh struct
Function then reads the file data, creating memory on the heap to store the verticies and indicies.
Assigns the heap pointer value into the struct.
TODO: Make this utilise a memory manager like AF_Memory to ensure it allocates memory from pre-allocated game memory.
Don't trust the OS virtual memory allocation ;)
====================
*/
static inline AF_CMesh AF_Mesh_Load_Data(void* _filePtr, AF_CMesh _mesh){
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
	.showDebug = FALSE
    };

#ifdef PLATFORM_GB
	if(_filePtr == NULL | _mesh.enabled == 0){}
	printf("TODO: Failed to AF_Mesh_Load_Data: Need to implement a portable FILE type for GB \n");
	return returnMesh;
#else
    // Recast the void* back to FILE type as its safe to do so
    FILE* _file = (FILE*)_filePtr;
    uint32_t verticesCount = 0;//_mesh.vertexCount;
    uint32_t indicesCount = 0;//_mesh.indexCount;
    uint32_t texCoordsCount = 0; 
    char fileBuffer[1024] ; // Buffer for reading the file

    PACKED_CHAR component = TRUE;
    component = AF_Component_SetHas(component, TRUE);
    component = AF_Component_SetEnabled(component, TRUE);

    // Create a new AF_Struct, that holds the pointers to the heap allocated verticies and indices data
    AF_CMesh loadedMesh = {
	    		//0,
			component,
                        (AF_Vertex*)malloc(sizeof(AF_Vertex) * _mesh.vertexCount), 
                        _mesh.vertexCount,
                        (uint32_t*)malloc(sizeof(uint32_t) * _mesh.indexCount),
                        _mesh.indexCount,
			0,
			0,
			0,
			{0,0},
			FALSE
                        };
   returnMesh = loadedMesh; 

    while(!feof(_file)){
         if(fgets(fileBuffer, 1024, _file) != NULL) {
        // print the return value (aka string read in) to terminal
            AF_Vertex vertex = {{0,0,0},{0,0,0},{0,0}};
            if(fileBuffer[0] == 'v' && fileBuffer[1] == ' '){
                // vertex
                Vec3 vertPos = AF_Mesh_GetVec3FromString(fileBuffer);
                vertex.position = vertPos;
                //AF_Log("v %f %f %f\n", vertex.position.x, vertex.position.y, vertex.position.z);

                returnMesh.vertices[verticesCount] = vertex;
                // TODO: null check the returned mesh.
                
                verticesCount++;
                continue;
            }
            if(fileBuffer[0] == 'f'){
                // face / indices
                // vertex
                
                // get the first element of each space sperated token
                // Add the indicies 
                char *p = strtok(fileBuffer, " ");
                while(p != NULL) {
                    if((strcmp(p,"f"))){
                        // Add the indicies, the first element in the token is the indicies 
                        returnMesh.indices[indicesCount] = atoi(&p[0]);
                        indicesCount++;
                    }
                    //printf("%s %i %i\n", p, indicesCount, atoi(&p[0]));
                    p = strtok(NULL, " ");
                }
                
                continue;
            }
            if(fileBuffer[0] == 'v' && fileBuffer[1] == 't'){
                // texture coordinate
		Vec2 texPos = AF_Mesh_GetVec2FromString(fileBuffer);
		//vertex.texCoord = texPos;
		// don't overwrite the verts when add the tex coords
		// TODO: fix this
		AF_Vertex tempVertex = returnMesh.vertices[texCoordsCount];
		
		AF_Vertex newVertex = {
			{tempVertex.position.x,tempVertex.position.y,tempVertex.position.z},
			{tempVertex.normal.x,tempVertex.normal.y,tempVertex.normal.z},
			{texPos.x, texPos.y}};
                returnMesh.vertices[texCoordsCount] = newVertex;
		texCoordsCount ++;

		continue;
            }
            if(fileBuffer[0] == 'v' && fileBuffer[1] == 'n'){
                // normal
                continue;
            }
        }
    }

    return returnMesh;
#endif
}


static inline AF_CMesh AF_Mesh_Load_OBJ(const char* _filePath){
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
	.showDebug = FALSE
    };

#ifdef PLATFORM_GB
	if(_filePath){}
	printf("TODO: AF_Mesh_LoadOBJ: need to implment portable FILE type for GB \n");
	return returnMesh;
#else
    // open the file
    FILE *file = fopen(_filePath, "r"); // Open the file for reading
    
    if(file == NULL){
        AF_Log_Error("%s Failed to open file %s\n", AF_MESH_FILE_TITLE, _filePath);
        return returnMesh;
    }

    // just get the size of the mesh stored into a AF_Mesh struct
    AF_CMesh tempMesh = AF_Mesh_GetOBJAttribSize(file);

    // Actually load the data into the mesh
    returnMesh = AF_Mesh_Load_Data(file, tempMesh);

   /* 
    AF_Log("Loading Model: \n");
    for(int i = 0; i < returnMesh.vertexCount; i++){
        AF_Vertex vertex = returnMesh.vertices[i];
        AF_Log("%f %f %f %f %f %f %f %f\n",vertex.position.x, vertex.position.y, vertex.position.z, vertex.normal.x, vertex.normal.y, vertex.normal.z, vertex.texCoord.x, vertex.texCoord.y);
    
    }

    AF_Log("Indexes: ");
    for(int i = 0; i < returnMesh.indexCount; i++){
        unsigned int index = returnMesh.indices[i];
        AF_Log("%i, ",index);
    }
    AF_Log("\n");
    */
    /*
    for(int i = 0; i < _mesh->vertexCount; i++){
        //AF_Log("V %f %f %f\n", mesh.vertices[i].position.x, mesh.vertices[i].position.y, mesh.vertices[i].position.z);
    }

    for(int i = 0; i < _mesh->indexCount; i++){
        //AF_Log("I %i\n", mesh.indices[i]);
    }*/

    //AF_Log("Mesh Verticies %d\n", mesh.vertexCount);
    //AF_Log("Mesh Indices %d\n", mesh.indexCount);
    
    // Return a copy of the AF_Mesh struct. It will contain pointers to the heap allocated memory

    
    //returnMesh.has =  TRUE;
    returnMesh.enabled = AF_Component_SetEnabled(returnMesh.enabled, TRUE);
    return returnMesh;
#endif
}

inline void AF_Mesh_Destroy(AF_CMesh* _mesh){
    if(_mesh == NULL){
        return;
    }

    free(_mesh->vertices);
    free(_mesh->indices);
    
}



#ifdef __cplusplus
}
#endif  

#endif  // AF_MESH_H
