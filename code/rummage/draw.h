#ifndef __DRAW_H
#define __DRAW_H

#include <GL/gl.h>

void draw_aabb(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z, float r, float g, float b) {
    glBegin(GL_LINES);
        glColor3f(r, g, b);

        // Bottom face
        glVertex3f(min_x, min_y, min_z);
        glVertex3f(max_x, min_y, min_z);
        
        glVertex3f(max_x, min_y, min_z);
        glVertex3f(max_x, min_y, max_z);
        
        glVertex3f(max_x, min_y, max_z);
        glVertex3f(min_x, min_y, max_z);
        
        glVertex3f(min_x, min_y, max_z);
        glVertex3f(min_x, min_y, min_z);

        // Top face
        glVertex3f(min_x, max_y, min_z);
        glVertex3f(max_x, max_y, min_z);
        
        glVertex3f(max_x, max_y, min_z);
        glVertex3f(max_x, max_y, max_z);
        
        glVertex3f(max_x, max_y, max_z);
        glVertex3f(min_x, max_y, max_z);
        
        glVertex3f(min_x, max_y, max_z);
        glVertex3f(min_x, max_y, min_z);

        // Front face
        glVertex3f(min_x, min_y, min_z);
        glVertex3f(min_x, max_y, min_z);
        
        glVertex3f(min_x, max_y, min_z);
        glVertex3f(max_x, max_y, min_z);
        
        glVertex3f(max_x, max_y, min_z);
        glVertex3f(max_x, min_y, min_z);
        
        glVertex3f(max_x, min_y, min_z);
        glVertex3f(min_x, min_y, min_z);

        // Back face
        glVertex3f(min_x, min_y, max_z);
        glVertex3f(min_x, max_y, max_z);
        
        glVertex3f(min_x, max_y, max_z);
        glVertex3f(max_x, max_y, max_z);
        
        glVertex3f(max_x, max_y, max_z);
        glVertex3f(max_x, min_y, max_z);
        
        glVertex3f(max_x, min_y, max_z);
        glVertex3f(min_x, min_y, max_z);

        // Right face
        glVertex3f(max_x, min_y, min_z);
        glVertex3f(max_x, max_y, min_z);
        
        glVertex3f(max_x, max_y, min_z);
        glVertex3f(max_x, max_y, max_z);
        
        glVertex3f(max_x, max_y, max_z);
        glVertex3f(max_x, min_y, max_z);
        
        glVertex3f(max_x, min_y, max_z);
        glVertex3f(max_x, min_y, min_z);

        // Left face
        glVertex3f(min_x, min_y, min_z);
        glVertex3f(min_x, max_y, min_z);
        
        glVertex3f(min_x, max_y, min_z);
        glVertex3f(min_x, max_y, max_z);
        
        glVertex3f(min_x, max_y, max_z);
        glVertex3f(min_x, min_y, max_z);
        
        glVertex3f(min_x, min_y, max_z);
        glVertex3f(min_x, min_y, min_z);

    glEnd();
}

void draw_line(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z, float r, float g, float b) {
    glBegin(GL_LINES);
        glColor3f(r, g, b);
        glVertex3f(min_x, min_y, min_z);
        glVertex3f(max_x, max_y, max_z);
    glEnd();
}


#endif
