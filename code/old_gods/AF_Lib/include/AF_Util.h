/*
===============================================================================
AF_UTIL_H

Implimentation of helper functions for utility functions like,
 - ReadFile
 - String comparisons
===============================================================================
*/
#ifndef AF_UTIL_H
#define AF_UTIL_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "AF_Log.h"
#include "AF_Vec2.h"
#include "AF_Vec3.h"
#include "AF_Lib_Define.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
====================
AF_Util_ReadFile
Read file from path and return char array ptr
====================
*/

static inline char* AF_Util_ReadFile(const char* thisFilePath) {
    #ifdef PLATFORM_GB
        printf("TODO: Failed to read file from %s \n Need implement file read for platform GB\n", thisFilePath);
	return NULL;
    #else
    FILE* _fileOpen =NULL;
    fopen(thisFilePath, "r");
    if (_fileOpen == NULL) {
        fprintf(stderr, "AF_Util: Read File: Failed to read file \n%s \nCheck file exists\n\n", thisFilePath);
        return NULL;
    }

    size_t textBufferSize = 1024;
    size_t totalLength = 0;
    char* textBuffer = (char*)malloc(textBufferSize);
    if (!textBuffer) {
        fprintf(stderr, "AF_Util: Read File: Memory allocation failed\n");
        fclose(_fileOpen);
        return NULL;
    }

    char line[1024];
    while (fgets(line, sizeof(line), _fileOpen) != NULL) {
        size_t lineLength = strlen(line);
        if (totalLength + lineLength + 1 > textBufferSize) {
            textBufferSize *= 2;
            char* tempBuffer = (char*)realloc(textBuffer, textBufferSize);
            if (!tempBuffer) {
                fprintf(stderr, "AF_Util: Read File: Memory reallocation failed\n");
                free(textBuffer);
                fclose(_fileOpen);
                return NULL;
            }
            textBuffer = tempBuffer;
        }
        strcpy(textBuffer + totalLength, line);
        totalLength += lineLength;
    }

    if (ferror(_fileOpen)) {
        fprintf(stderr, "AF_Util: Read File: Error reading file \n%s\n", thisFilePath);
        free(textBuffer);
        fclose(_fileOpen);
        return NULL;
    }

    fclose(_fileOpen);

    if (totalLength == 0) {
        free(textBuffer);
        return NULL;
    } else {
        return textBuffer;
    }

return NULL;
#endif
}




/*
====================
AF_STRING_EMPTY
Check if a const char* is empty
Return 1 if string is empty
REturn 0 if string is not empty
====================
*/
static inline BOOL AF_STRING_IS_EMPTY(const char* _string){
    // null check the const char*, then see if the first element is a end of line
    if ((_string != NULL) && (_string[0] == '\0')) {
        return TRUE;
    }else {
        return FALSE;
    }
}

/*
====================
AF_GetMaxElement
Little helper function to take in a float array and return the index of the highest value
====================
*/
static inline int AF_GetMaxElement(AF_FLOAT _elements[], int _size){
	int maxElement = -1;
	AF_FLOAT highestValue = -1;
	for(int i = 0; i < _size; ++i){
		if(_elements[i] > highestValue){
			highestValue = _elements[i];
			maxElement = i;
		}
	}
	return maxElement;
}

/*
static void multiplyMatrixVec(const AF_FLOAT matrix[16], const AF_FLOAT in[4], AF_FLOAT out[4]) {
    for (int i = 0; i < 4; i++) {
        out[i] = matrix[0 * 4 + i] * in[0] +
                 matrix[1 * 4 + i] * in[1] +
                 matrix[2 * 4 + i] * in[2] +
                 matrix[3 * 4 + i] * in[3];
    }
}*/

/*
====================
AF_WorldToScreen
Little helper function to take a vec 3 position and convert it to a screen position
====================
*/
static inline Vec2 AF_WorldToScreen(const Vec3* _worldPos, const AF_FLOAT _modelViewMatrix[16], const AF_FLOAT _projectionMatrix[16], const int _viewport[4], Vec2 _screenSize) {
    Vec2 screenPosition = {0, 0};
    
    // Model-view transformation
    AF_FLOAT viewPos[4] = {
        _worldPos->x * _modelViewMatrix[0] + _worldPos->y * _modelViewMatrix[4] + _worldPos->z * _modelViewMatrix[8]  + _modelViewMatrix[12],
        _worldPos->x * _modelViewMatrix[1] + _worldPos->y * _modelViewMatrix[5] + _worldPos->z * _modelViewMatrix[9]  + _modelViewMatrix[13],
        _worldPos->x * _modelViewMatrix[2] + _worldPos->y * _modelViewMatrix[6] + _worldPos->z * _modelViewMatrix[10] + _modelViewMatrix[14],
        _worldPos->x * _modelViewMatrix[3] + _worldPos->y * _modelViewMatrix[7] + _worldPos->z * _modelViewMatrix[11] + _modelViewMatrix[15]
    };

    // Apply projection transformation
    AF_FLOAT clipPos[4] = {
        viewPos[0] * _projectionMatrix[0] + viewPos[1] * _projectionMatrix[4] + viewPos[2] * _projectionMatrix[8]  + viewPos[3] * _projectionMatrix[12],
        viewPos[0] * _projectionMatrix[1] + viewPos[1] * _projectionMatrix[5] + viewPos[2] * _projectionMatrix[9]  + viewPos[3] * _projectionMatrix[13],
        viewPos[0] * _projectionMatrix[2] + viewPos[1] * _projectionMatrix[6] + viewPos[2] * _projectionMatrix[10] + viewPos[3] * _projectionMatrix[14],
        viewPos[0] * _projectionMatrix[3] + viewPos[1] * _projectionMatrix[7] + viewPos[2] * _projectionMatrix[11] + viewPos[3] * _projectionMatrix[15]
    };

    // Perspective division (convert to normalized device coordinates)
    AF_FLOAT w = clipPos[3];
    if (w != 0.0f) {
        AF_FLOAT ndcX = clipPos[0] / w;
        AF_FLOAT ndcY = clipPos[1] / w;

        // Viewport transformation (convert from NDC [-1,1] to screen space [0, screenSize])
        screenPosition.x = (ndcX * 0.5f + 0.5f) * _viewport[2] + _viewport[0];  // X-axis
        screenPosition.y = (ndcY * -0.5f + 0.5f) * _viewport[3] + _viewport[1];  // Y-axis
    }

    return screenPosition;
}


#ifdef __cplusplus
}
#endif

#endif  // AF_UTIL_H
