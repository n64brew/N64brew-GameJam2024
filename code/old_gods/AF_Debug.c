/*
===============================================================================
Implementation of AF_Debug.h to be used with 
n64 libdragon
===============================================================================
*/

#include "AF_Debug.h"
#include <libdragon.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/gl_integration.h>
#include <rspq_profile.h>
/*
====================
AF_Debug_DrawPoint
Drwa a pixel point at the given screen space location
Provide an x, and y location as well as a size
====================
*/
void AF_Debug_DrawPoint(AF_FLOAT _xPos1, AF_FLOAT _yPos1, AF_FLOAT _size);


/*
====================
AF_Debug_DrawLineWorld
Drwa a line between two points in world space
====================
*/
void AF_Debug_DrawLineWorld(Vec3* _point1, Vec3* _point2, AF_FLOAT _color[], BOOL _ignoreDepth){
    // Get the display and z buffer 
    // Set line color
    glColor3f(_color[0], _color[1], _color[2]); // Red

    if(_ignoreDepth == FALSE){
        // Enable depth testing
        glEnable(GL_DEPTH_TEST);
        // Set the depth function to always pass
        glDepthFunc(GL_ALWAYS);
    }

    
    
    glBegin(GL_LINES);
        glVertex3f(_point1->x, _point1->y, _point1->z);
        glVertex3f(_point2->x, _point2->y, _point2->z);
    glEnd();

    if(_ignoreDepth == FALSE){
        // Enable depth testing
        // Restore the default depth function
        glDepthFunc(GL_LESS);   
    }
}

/*
====================
AF_Debug_DrawLineArrayWorld
Drwa a line between two points in world space
====================
*/
void AF_Debug_DrawLineArrayWorld(Vec3* _lineArray, int _arraySize, AF_FLOAT _color[], BOOL _ignoreDepth){
    if(_arraySize < 2){
        debugf("AF_Debug_DrawLineArrayWorld: array size is too small \n");
        return;
    }
    // Get the display and z buffer 
    // Set line color
    glColor3f(_color[0], _color[1], _color[2]); // Red

    if(_ignoreDepth == FALSE){
        // Enable depth testing
        glEnable(GL_DEPTH_TEST);
        // Set the depth function to always pass
        glDepthFunc(GL_ALWAYS);
    }

    
    
    glBegin(GL_LINES);
        for(int i  = 0; i < _arraySize; ++i){
            glVertex3f(_lineArray[i].x, _lineArray[i].y, _lineArray[i].z);
        }
    glEnd();

    if(_ignoreDepth == FALSE){
        // Restore the default depth function
        glDepthFunc(GL_LESS);   
    }
}