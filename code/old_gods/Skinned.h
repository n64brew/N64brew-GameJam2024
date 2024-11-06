#ifndef SKINNED_H
#define SKINNED_H

#include <libdragon.h>
#include <GL/gl.h>
#include <stdint.h>
#include "ECS/Components/AF_CTransform3D.h"
#include "AF_Vec3.h"
#include "AF_Vec4.h"
#include "AF_Mat4.h"

#include "AF_Lib_Define.h"
#include "camera.h"

typedef struct {
    float position[3];
    float texcoord[2];
    float normal[3];
    uint8_t mtx_index;
} skinned_vertex_t;

static const skinned_vertex_t skinned_vertices[] = {
    { .position = {-2,  0, -1}, .texcoord = {0.f, 0.f}, .normal = { 0.f,  1.f,  0.f}, .mtx_index = 0 },
    { .position = {-2,  0,  1}, .texcoord = {1.f, 0.f}, .normal = { 0.f,  1.f,  0.f}, .mtx_index = 0 },
    { .position = {-1,  0, -1}, .texcoord = {0.f, 1.f}, .normal = { 0.f,  1.f,  0.f}, .mtx_index = 0 },
    { .position = {-1,  0,  1}, .texcoord = {1.f, 1.f}, .normal = { 0.f,  1.f,  0.f}, .mtx_index = 0 },
    { .position = { 1,  0, -1}, .texcoord = {0.f, 2.f}, .normal = { 0.f,  1.f,  0.f}, .mtx_index = 1 },
    { .position = { 1,  0,  1}, .texcoord = {1.f, 2.f}, .normal = { 0.f,  1.f,  0.f}, .mtx_index = 1 },
    { .position = { 2,  0, -1}, .texcoord = {0.f, 3.f}, .normal = { 0.f,  1.f,  0.f}, .mtx_index = 1 },
    { .position = { 2,  0,  1}, .texcoord = {1.f, 3.f}, .normal = { 0.f,  1.f,  0.f}, .mtx_index = 1 },
};



// Bone
typedef struct Bone {
    Vec3 position;
    Vec4 rotation;  // Qaternion rotation
    Vec3 scale;
    Mat4 inverseBindPose;
    Mat4 currentTransform;  //
    struct Bone* parent;
    struct Bone** children; // array of children
    uint8_t childCount;
} Bone;

// Skeleton
typedef struct Skeleton {
    Bone* bones;
    int boneCount;
} Skeleton;

// Keyframe
typedef struct Keyframe {
    AF_FLOAT time;
    Vec3 position;
    Vec4 rotation;
    Vec3 scale;
} Keyframe;

// Animation
#define MAX_BONES 2 // Define the maximum number of bones

typedef struct Animation {
    uint8_t boneCount;
    Keyframe* keyframes[MAX_BONES]; // array of keyframes for each bone
    int keyframeCount[MAX_BONES]; // Number of keyframes per bone
    AF_FLOAT duration;
} Animation;


// Custom Data

// Define the skeleton structure
static Skeleton skeleton;
static Animation animation;

// Define the bones
//static Bone rootBone;
static Bone leftEdgeBone;
static Bone rightEdgeBone;
/*
static Keyframe rootBoneKeyframes[] = {
    { 0.0f, { 0, 0, 0 }, { 0, 0, 0, 1 }, { 1, 1, 1 } }, // Start at origin
    { 1.0f, { 0, 1, 0 }, { 0, 0, 0, 1 }, { 1, 1, 1 } }, // Move up
    { 2.0f, { 0, 0, 0 }, { 0, 0, 0, 1 }, { 1, 1, 1 } }, // Return to origin
};*/

static Keyframe leftEdgeKeyframes[] = {
    { 0.0f, { -2, 0, 0 }, { 0, 0, 0, 1 }, { 1, 1, 1 } },
    { 1.0f, { -2, 1, 0 }, { 0, 0, 0, 1 }, { 1, 1, 1 } }, // Move up
    { 2.0f, { -2, 0, 0 }, { 0, 0, 0, 1 }, { 1, 1, 1 } }, // Return to original
};

static Keyframe rightEdgeKeyframes[] = {
    { 0.0f, { 2, 0, 0 }, { 0, 0, 0, 1 }, { 1, 1, 1 } },
    { 1.0f, { 2, 1, 0 }, { 0, 0, 0, 1 }, { 1, 1, 1 } }, // Move up
    { 2.0f, { 2, 0, 0 }, { 0, 0, 0, 1 }, { 1, 1, 1 } }, // Return to original
};

// Setup the skeleton
void setup_skeleton() {

/*
    // Initialize the root bone
    rootBone.position = (Vec3){0, 0, 0};
    rootBone.rotation = (Vec4){0, 0, 0, 1}; // No rotation
    rootBone.scale = (Vec3){1, 1, 1};
    rootBone.parent = NULL;
    rootBone.children = NULL; // No children
    rootBone.childCount = 0; // No children
*/
    // Initialize the left edge bone
    leftEdgeBone.position = (Vec3){-2, 0, 0}; // Position corresponds to the left edge vertex
    leftEdgeBone.rotation = (Vec4){0, 0, 0, 1}; // No rotation
    leftEdgeBone.scale = (Vec3){1, 1, 1};
    leftEdgeBone.parent = NULL;//&rootBone; // Parent is the root bone
    leftEdgeBone.children = NULL; // No children
    leftEdgeBone.childCount = 0; // No children

    // Initialize the right edge bone
    rightEdgeBone.position = (Vec3){2, 0, 0}; // Position corresponds to the right edge vertex
    rightEdgeBone.rotation = (Vec4){0, 0, 0, 1}; // No rotation
    rightEdgeBone.scale = (Vec3){1, 1, 1};
    rightEdgeBone.parent = NULL;//&rootBone; // Parent is the root bone
    rightEdgeBone.children = NULL; // No children
    rightEdgeBone.childCount = 0; // No children

    // Set up the skeleton
    skeleton.boneCount = MAX_BONES; // One root bone and two edge bones
    skeleton.bones = (Bone*)malloc(sizeof(Bone) * skeleton.boneCount);
    if(skeleton.bones == NULL){
        debugf("Skinned: Setup_skeleton: skeleton.bones is null\n");
        return; 
    }
    skeleton.bones[0] = leftEdgeBone;
    skeleton.bones[1] = rightEdgeBone;
    //skeleton.bones[2] = rightEdgeBone;

    // Correctly link parent pointers after allocation
    //leftEdgeBone.parent = &skeleton.bones[0]; // Link to root bone
    //rightEdgeBone.parent = &skeleton.bones[0]; // Link to root bone
}


// Setup animation data
void setup_animation(){//(Animation* animation) {
    animation.boneCount = MAX_BONES;
    // Initialize keyframes for each bone
    //animation.keyframes[0] = rootBoneKeyframes; // Root bone does not have keyframes
    animation.keyframes[0] = leftEdgeKeyframes; // Keyframes for left edge bone
    animation.keyframes[1] = rightEdgeKeyframes; // Keyframes for right edge bone

    // Store keyframe counts directly without dynamic allocation
    //animation.keyframeCount[0] = sizeof(rootBoneKeyframes) / sizeof(Keyframe); // Count for left edge
    animation.keyframeCount[0] = sizeof(leftEdgeKeyframes) / sizeof(Keyframe); // Count for left edge
    animation.keyframeCount[1] = sizeof(rightEdgeKeyframes) / sizeof(Keyframe); // Count for right edge
}




Vec3 lerp(Vec3 start, Vec3 end, float t) {
    Vec3 result;
    result.x = start.x + t * (end.x - start.x);
    result.y = start.y + t * (end.y - start.y);
    result.z = start.z + t * (end.z - start.z);
    return result;
}

// Lerp rotation function for quaternions
Vec4 lerp_rotation(Vec4 start, Vec4 end, float t) {
    Vec4 result;
    result.x = start.x + t * (end.x - start.x);
    result.y = start.y + t * (end.y - start.y);
    result.z = start.z + t * (end.z - start.z);
    result.w = start.w + t * (end.w - start.w);

    // Normalize the result to keep it as a valid quaternion
    return Vec4_NORMALIZE(result);
}

// Function to create an identity matrix
Mat4 identity_matrix() {
    Mat4 mat = {
        .rows = {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}
        }
    };
    return mat;
}

// Function to build a rotation matrix from a quaternion
Mat4 quat_to_rotation_matrix(Vec4 q) {
    Mat4 rot = identity_matrix();

    AF_FLOAT xx = q.x * q.x;
    AF_FLOAT yy = q.y * q.y;
    AF_FLOAT zz = q.z * q.z;
    AF_FLOAT xy = q.x * q.y;
    AF_FLOAT xz = q.x * q.z;
    AF_FLOAT yz = q.y * q.z;
    AF_FLOAT wx = q.w * q.x;
    AF_FLOAT wy = q.w * q.y;
    AF_FLOAT wz = q.w * q.z;

    rot.rows[0] = (Vec4){1.0f - 2.0f * (yy + zz), 2.0f * (xy - wz), 2.0f * (xz + wy), 0};
    rot.rows[1] = (Vec4){2.0f * (xy + wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz - wx), 0};
    rot.rows[2] = (Vec4){2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (xx + yy), 0};
    rot.rows[3] = (Vec4){0, 0, 0, 1};

    return rot;
}

// Function to build a scaling matrix
Mat4 scale_matrix(Vec3 scale) {
    Mat4 scaleMat = identity_matrix();
    scaleMat.rows[0].x = scale.x;
    scaleMat.rows[1].y = scale.y;
    scaleMat.rows[2].z = scale.z;
    return scaleMat;
}

// Function to build a translation matrix
Mat4 translation_matrix(Vec3 position) {
    Mat4 transMat = identity_matrix(); // Start with an identity matrix
    transMat.rows[3].x = position.x;   // Set the translation in the fourth column
    transMat.rows[3].y = position.y;   // Set the translation in the fourth column
    transMat.rows[3].z = position.z;   // Set the translation in the fourth column
    return transMat;
}



void Mat4_To_Float_Array(Mat4 mat, float* out) {
    // Assuming 'out' is already allocated with at least 16 floats
    for (int i = 0; i < 4; ++i) {
        out[0 + i * 4] = mat.rows[i].x;   // Column 1
        out[1 + i * 4] = mat.rows[i].y;   // Column 2
        out[2 + i * 4] = mat.rows[i].z;   // Column 3
        out[3 + i * 4] = mat.rows[i].w;   // Column 4
    }
}

Keyframe* get_start_keyframe(Animation* _animation, int boneIndex, AF_FLOAT time) {
    if (_animation == NULL) {
        debugf("get_start_keyframe: _animation is null\n");
        return NULL;
    }

    if (boneIndex < 0 || boneIndex >= _animation->boneCount) {
        debugf("get_start_keyframe: boneIndex out of bounds %i bone count %i\n", boneIndex, _animation->boneCount);
        return NULL;
    }

    Keyframe* keyframes = _animation->keyframes[boneIndex];
    if (keyframes == NULL) {
        debugf("get_start_keyframe: keyframes is null, index %i\n", boneIndex);
        return NULL;
    }

    int keyframeCount = _animation->keyframeCount[boneIndex];

    // If time is before or at the first keyframe, return the first keyframe
    if (time < keyframes[0].time) {
        return &keyframes[0];
    }

    // Search for the keyframe just before the given time
    for (int i = 1; i < keyframeCount; ++i) {
        if (keyframes[i].time > time) {
            return &keyframes[i - 1];
        }
    }

    // If time is after the last keyframe, return the last keyframe
    return &keyframes[keyframeCount - 1];
}

Keyframe* get_end_keyframe(Animation* _animation, int boneIndex, AF_FLOAT time) {
    if (_animation == NULL) {
        debugf("get_end_keyframe: _animation is null\n");
        return NULL;
    }

    if (boneIndex < 0 || boneIndex >= _animation->boneCount) {
        debugf("get_end_keyframe: boneIndex out of bounds %i bone count %i\n", boneIndex, _animation->boneCount);
        return NULL;
    }

    Keyframe* keyframes = _animation->keyframes[boneIndex];
    if (keyframes == NULL) {
        debugf("get_end_keyframe: keyframes is null, index %i\n", boneIndex);
        return NULL;
    }

    int keyframeCount = _animation->keyframeCount[boneIndex];

    // If time is before or at the first keyframe, return the first keyframe
    if (time < keyframes[0].time) {
        return &keyframes[0];
    }

    // Search for the keyframe just after the given time
    for (int i = 0; i < keyframeCount; ++i) {
        if (keyframes[i].time > time) {
            return &keyframes[i];
        }
    }

    // If time is after the last keyframe, return the last keyframe
    return &keyframes[keyframeCount - 1];
}


// Function to build the transformation matrix
Mat4 build_transform_matrix(Vec3 position, Vec4 rotation, Vec3 scale) {
    Mat4 transMat = translation_matrix(position);
    //Mat4 rotMat = quat_to_rotation_matrix(rotation);
    //Mat4 scaleMat = scale_matrix(scale);

    // Order of transformations: Scale -> Rotate -> Translate
    Mat4 transform = transMat;//Mat4_MULT_M4(transMat, Mat4_MULT_M4(rotMat, scaleMat));
    return transform;
}


// Function to print a Mat4 matrix
void printMat4(const Mat4* matrix) {
    for (int i = 0; i < 4; i++) {
        debugf("| %f %f %f %f |\n", matrix->rows[i].x, matrix->rows[i].y, matrix->rows[i].z, matrix->rows[i].w);
    }
    debugf("\n");
}

void Update_Bone_Transform(Skeleton* _skeleton, Animation* _animation, AF_FLOAT _time) {
    if (_skeleton == NULL || _animation == NULL) {
        debugf("Skinned: Update_Bone_Transform: Error: Skeleton or Animation data is NULL.\n");
        return;
    }

    if (_skeleton->boneCount == 0 || _animation->keyframeCount[0] == 0) {
        debugf("Skinned: Update_Bone_Transform: Error: Skeleton or Animation data is uninitialized.\n");
        return;
    }

    for (int i = 0; i < _skeleton->boneCount; ++i) {
        // Find the keyframes around the current time and interpolate.
        Keyframe* start = get_start_keyframe(_animation, i, _time);
        Keyframe* end = get_end_keyframe(_animation, i, _time);

        // Log keyframe information
        if (start == NULL || end == NULL) {
            debugf("Update_Bone_Transform: Keyframe data for bone %i is missing.\n", i);
            continue; // Skip this bone if keyframes are missing
        }

        float time1 = (_time - start->time);
        float time2 = (end->time - start->time);

        //debugf("Bone %d: _time: %f, start->time: %f, end->time: %f, time1: %f, time2: %f\n", i, _time, start->time, end->time, time1, time2);
        
        if (time2 == 0) {
            //debugf("Bone %d: time2 is zero, skipping interpolation.\n", i);
            continue; // Skip if there's no time difference
        }
        
        AF_FLOAT t = time1 / time2;
        // Clamp t to the range [0, 1]
        t = fmaxf(0.0f, fminf(t, 1.0f)); // Ensure t is clamped between 0 and 1
        
        //debugf("Bone %d: Clamped t: %f\n", i, t);

        Vec3 interpolatedPosition = lerp(start->position, end->position, t);
        // Print the interpolated position values
        //debugf("Bone %d: Interpolated Position: (%f, %f, %f)\n", i, interpolatedPosition.x, interpolatedPosition.y, interpolatedPosition.z);

        Vec4 interpolatedRotation = lerp_rotation(start->rotation, end->rotation, t);
        Vec3 interpolatedScale = lerp(start->scale, end->scale, t);
    
        // Build transformation matrix
        Mat4 transform = build_transform_matrix(interpolatedPosition, interpolatedRotation, interpolatedScale);
        
        // Apply parent transformation if applicable
        if (_skeleton->bones[i].parent != NULL) {
            Mat4 parentTransform = _skeleton->bones[i].parent->currentTransform;
            transform = Mat4_MULT_M4(parentTransform, transform);
        }

        // Update the bone's current transform
        _skeleton->bones[i].currentTransform = transform;

        //debugf("=====SkinneD: RenderSkeletal: %i ======\n", i);
        //printMat4(&transform);
    }
}







void draw_skinned()
{
    glEnable(GL_MATRIX_PALETTE_ARB);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_MATRIX_INDEX_ARRAY_ARB);

    glVertexPointer(        3,  GL_FLOAT,           sizeof(skinned_vertex_t),   skinned_vertices[0].position);
    glTexCoordPointer(      2,  GL_FLOAT,           sizeof(skinned_vertex_t),   skinned_vertices[0].texcoord);
    glNormalPointer(            GL_FLOAT,           sizeof(skinned_vertex_t),   skinned_vertices[0].normal);
    glMatrixIndexPointerARB(1,  GL_UNSIGNED_BYTE,   sizeof(skinned_vertex_t),   &skinned_vertices[0].mtx_index);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(skinned_vertices)/sizeof(skinned_vertex_t));

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_MATRIX_INDEX_ARRAY_ARB);

    glDisable(GL_MATRIX_PALETTE_ARB);
}

// Helper to get current ModelView matrix
void getCurrentModelViewMatrix(float* outMatrix) {
    if (outMatrix == NULL) {
        debugf("Skinned: getCurrentModelViewMatrix: Error - NULL output matrix\n");
        return;
    }
    
    // Method 1: Get directly from OpenGL (if supported on your N64 implementation)
    glGetFloatv(GL_MODELVIEW, outMatrix);
    
    // Method 2: Track matrix stack ourselves
    // memcpy(outMatrix, modelViewStack.matrices[modelViewStack.top], sizeof(float) * 16);
}



void applyBoneTransformation(Mat4 boneMatrix,float _time) {
    
    
    // Extract translation (assuming row-major order)
    //float tx = boneMatrix.rows[3].x; // element (3, 0) in the 4x4 matrix
    //float ty = boneMatrix.rows[3].y; // element (3, 1)
    //float tz = boneMatrix.rows[3].z; // element (3, 2)

    //float flatMatrix[16]; // assuming boneMatrix is properly initialized elsewhere
    //Mat4_To_Float_Array(boneMatrix, flatMatrix);
    //glMultMatrixf(flatMatrix);
    //glTranslatef(tx, ty, tz);
    glRotatef(-sinf(_time*0.1f)*45, 0, 0, 1);

}


void Render_Skeletal(AF_CTransform3D* _transform, AF_FLOAT _time){

    Skeleton* _skeleton = &skeleton;
    Animation* _animation = &animation;
    if (_skeleton == NULL || _animation == NULL) {
        debugf("Render_Skeletal: skeleton or animation is null\n");
        return;
    }
    
    Update_Bone_Transform(_skeleton, &animation, _time);

    // Save current matrix mode
    //GLint previousMode;
    //glGetIntegerv(GL_MATRIX_MODE, &previousMode);
    

    // Set up model transform first
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glScalef(_transform->scale.x, _transform->scale.y, _transform->scale.z);
    glTranslatef(_transform->pos.x, _transform->pos.y, _transform->pos.z);
    
    //float modelViewMatrix[16];
    //getCurrentModelViewMatrix(modelViewMatrix);

    // Set the matrix mode for the palette
    glMatrixMode(GL_MATRIX_PALETTE_ARB);
    
    for (int i = 0; i < _skeleton->boneCount; i++) {
        glCurrentPaletteMatrixARB(i);
        
        glCopyMatrixN64(GL_MODELVIEW);
        // original rotation

        if(i == 0){
            applyBoneTransformation(_skeleton->bones[i].currentTransform, _time );
        }else{
            applyBoneTransformation(_skeleton->bones[i].currentTransform, -_time );
        }
        
    }


    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_CULL_FACE);
    draw_skinned();
    glEnable(GL_CULL_FACE);

   

    glPopMatrix();
    
}

void render_skinned(AF_CTransform3D* _transform, float _animationTime)//const camera_t *camera, float animation)
{
    rdpq_debug_log_msg("Skinned");
    
    float index = fmod(_animationTime * 0.1f, 3.0f); // Loops through 0 to 3

    Render_Skeletal(_transform, index);
    
}

#endif