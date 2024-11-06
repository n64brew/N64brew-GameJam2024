
/*================
Simple transform struct with pos, rot, scale values
=================*/
#ifndef CTRANSFORM_H
#define CTRANSFORM_H
typedef struct {
    float position[3];
    float rotation[3];
    float scale[3];
} CTransform;
#endif
