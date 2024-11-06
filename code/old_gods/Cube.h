// From https://github.com/DragonMinded/libdragon/blob/opengl/examples/gldemo/plane.h
#ifndef CUBE_H
#define CUBE_H

#include <libdragon.h>
#include <GL/gl.h>
#include "Vertex.h"
#include "ECS/Components/AF_CTransform3D.h"
static const float cube_size = 1.0f;

static const vertex_t cube_vertices[] = {
    // +X
    { .position = { cube_size, -cube_size, -cube_size}, .texcoord = {0.f, 0.f}, .normal = { 1.f,  0.f,  0.f}, .color = 0xFF0000FF },
    { .position = { cube_size,  cube_size, -cube_size}, .texcoord = {1.f, 0.f}, .normal = { 1.f,  0.f,  0.f}, .color = 0xFF0000FF },
    { .position = { cube_size,  cube_size,  cube_size}, .texcoord = {1.f, 1.f}, .normal = { 1.f,  0.f,  0.f}, .color = 0xFF0000FF },
    { .position = { cube_size, -cube_size,  cube_size}, .texcoord = {0.f, 1.f}, .normal = { 1.f,  0.f,  0.f}, .color = 0xFF0000FF },

    // -X
    { .position = {-cube_size, -cube_size, -cube_size}, .texcoord = {0.f, 0.f}, .normal = {-1.f,  0.f,  0.f}, .color = 0x00FFFFFF },
    { .position = {-cube_size, -cube_size,  cube_size}, .texcoord = {0.f, 1.f}, .normal = {-1.f,  0.f,  0.f}, .color = 0x00FFFFFF },
    { .position = {-cube_size,  cube_size,  cube_size}, .texcoord = {1.f, 1.f}, .normal = {-1.f,  0.f,  0.f}, .color = 0x00FFFFFF },
    { .position = {-cube_size,  cube_size, -cube_size}, .texcoord = {1.f, 0.f}, .normal = {-1.f,  0.f,  0.f}, .color = 0x00FFFFFF },

    // +Y
    { .position = {-cube_size,  cube_size, -cube_size}, .texcoord = {0.f, 0.f}, .normal = { 0.f,  1.f,  0.f}, .color = 0x00FF00FF },
    { .position = {-cube_size,  cube_size,  cube_size}, .texcoord = {0.f, 1.f}, .normal = { 0.f,  1.f,  0.f}, .color = 0x00FF00FF },
    { .position = { cube_size,  cube_size,  cube_size}, .texcoord = {1.f, 1.f}, .normal = { 0.f,  1.f,  0.f}, .color = 0x00FF00FF },
    { .position = { cube_size,  cube_size, -cube_size}, .texcoord = {1.f, 0.f}, .normal = { 0.f,  1.f,  0.f}, .color = 0x00FF00FF },

    // -Y
    { .position = {-cube_size, -cube_size, -cube_size}, .texcoord = {0.f, 0.f}, .normal = { 0.f, -1.f,  0.f}, .color = 0xFF00FFFF },
    { .position = { cube_size, -cube_size, -cube_size}, .texcoord = {1.f, 0.f}, .normal = { 0.f, -1.f,  0.f}, .color = 0xFF00FFFF },
    { .position = { cube_size, -cube_size,  cube_size}, .texcoord = {1.f, 1.f}, .normal = { 0.f, -1.f,  0.f}, .color = 0xFF00FFFF },
    { .position = {-cube_size, -cube_size,  cube_size}, .texcoord = {0.f, 1.f}, .normal = { 0.f, -1.f,  0.f}, .color = 0xFF00FFFF },

    // +Z
    { .position = {-cube_size, -cube_size,  cube_size}, .texcoord = {0.f, 0.f}, .normal = { 0.f,  0.f,  1.f}, .color = 0x0000FFFF },
    { .position = { cube_size, -cube_size,  cube_size}, .texcoord = {1.f, 0.f}, .normal = { 0.f,  0.f,  1.f}, .color = 0x0000FFFF },
    { .position = { cube_size,  cube_size,  cube_size}, .texcoord = {1.f, 1.f}, .normal = { 0.f,  0.f,  1.f}, .color = 0x0000FFFF },
    { .position = {-cube_size,  cube_size,  cube_size}, .texcoord = {0.f, 1.f}, .normal = { 0.f,  0.f,  1.f}, .color = 0x0000FFFF },

    // -Z
    { .position = {-cube_size, -cube_size, -cube_size}, .texcoord = {0.f, 0.f}, .normal = { 0.f,  0.f, -1.f}, .color = 0xFFFF00FF },
    { .position = {-cube_size,  cube_size, -cube_size}, .texcoord = {0.f, 1.f}, .normal = { 0.f,  0.f, -1.f}, .color = 0xFFFF00FF },
    { .position = { cube_size,  cube_size, -cube_size}, .texcoord = {1.f, 1.f}, .normal = { 0.f,  0.f, -1.f}, .color = 0xFFFF00FF },
    { .position = { cube_size, -cube_size, -cube_size}, .texcoord = {1.f, 0.f}, .normal = { 0.f,  0.f, -1.f}, .color = 0xFFFF00FF },
};

static const uint16_t cube_indices[] = {
     0,  1,  2,  0,  2,  3,
     4,  5,  6,  4,  6,  7,
     8,  9, 10,  8, 10, 11,
    12, 13, 14, 12, 14, 15,
    16, 17, 18, 16, 18, 19,
    20, 21, 22, 20, 22, 23,
};

void setup_cube()
{
}

void draw_cube()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(vertex_t), (void*)(0*sizeof(float) + (void*)cube_vertices));
    glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), (void*)(3*sizeof(float) + (void*)cube_vertices));
    glNormalPointer(GL_FLOAT, sizeof(vertex_t), (void*)(5*sizeof(float) + (void*)cube_vertices));
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex_t), (void*)(8*sizeof(float) + (void*)cube_vertices));

    glDrawElements(GL_TRIANGLES, sizeof(cube_indices) / sizeof(uint16_t), GL_UNSIGNED_SHORT, cube_indices);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void render_cube(const AF_CTransform3D* _transform, int _animate, float _rotation)
{
    rdpq_debug_log_msg("Cube");
    glPushMatrix();

    if(_transform == NULL){
	debugf("Renderer Update: Trying to render a cube with a null transform\n");
	return;
	}

    // Apply scaling
    glScalef(_transform->scale.x, _transform->scale.y, _transform->scale.z);

    // Apply rotation if needed (optional)
    // glRotatef(_transform->rotationAngle, _transform->rotationAxis.x, _transform->rotationAxis.y, _transform->rotationAxis.z);
    if(_animate == 1){
        //glRotatef(_rotation*0.23f, 1, 0, 0);
        glTranslatef(_transform->pos.x*.75f, _transform->pos.y*.75f, _transform->pos.z*.75f);
        glRotatef(_rotation*2.0f, 0, 0, 1.0f);
        //glRotatef(_rotation*1.2f, 0, .5, 0);
    }

    glTranslatef(_transform->pos.x, _transform->pos.y, _transform->pos.z);
    

    // Apply vertex color as material color.
    // Because the cube has colors set per vertex, we can color each face seperately
    
    //glEnable(GL_COLOR_MATERIAL);

    // Apply to ambient and diffuse material properties
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    draw_cube();
    
    //glDisable(GL_COLOR_MATERIAL);

    glPopMatrix();
}

#endif
