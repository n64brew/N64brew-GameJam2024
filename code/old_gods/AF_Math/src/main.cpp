/* Commented out because it breaks the build otherwise

#include <iostream>
#include <cassert>
#include <stdio.h>
#include <stdexcept>

#include "AF_Math.h"

void RunUnitTests();

int main(){
    
    printf("Starting AF_Math Unit Tests \n");
    RunUnitTests();
    return 0;
}

void RunUnitTests(){

    // Test dot product
    AF_Vec3 v7{1.0f,2.0f,3.0f};
    AF_Vec3 v8{2.0f,3.0f,4.0f};
    float dotTest = AFV3_DOT(v7,v8);
    assert(dotTest == 20.0f);

    // Test cross product
    AF_Vec3 v9{1.0f,2.0f,3.0f};
    AF_Vec3 v10{2.0f,3.0f,4.0f};
    AF_Vec3 v11 = AFV3_CROSS(v9, v10);
    assert(v11.x == -1.0f);
    assert(v11.y == 2.0f);
    assert(v11.z == -1.0f);
    
    // Test normalize
    AF_Vec3 v12{1.0f,2.0f,3.0f};
    AF_Vec3 v13 = AFV3_NORMALIZE(v12);
    assert(v13.x == 0.26726124f);
    assert(v13.y == 0.53452248f);
    assert(v13.z == 0.80178368f);

    
    // Test distance
    AF_Vec3 v14{1.0f,2.0f,3.0f};
    AF_Vec3 v15{2.0f,3.0f,4.0f};
    float dist = AFV3_DISTANCE(v14,v15);
    assert(dist == 1.73205081f);

    // Test length
    AF_Vec3 v16{1.0f,2.0f,3.0f};
    float len = AFV3_MAGNITUDE(v16);
    assert(len == 3.74165739f);

    // Test Projection
    AF_Vec3 v17{1.0f,2.0f,3.0f};
    AF_Vec3 v18{2.0f,3.0f,4.0f};
    AF_Vec3 v19 = AFV3_PROJECTION(v17,v18);
    assert(v19.x == 1.37931037f);
    assert(v19.y == 2.06896544f);
    assert(v19.z == 2.75862074f);

    // Test Orthogonal
    AF_Vec3 v20{0.707106781f,0.707106781f, 0.0f};//
    AF_Vec3 v21{-1.0f, 1.0f, -1.0f};
    AF_Vec3 v22{0.0f,-2.0f,-2.0};
    std::vector<AF_Vec3> vv1;
    vv1.push_back(v20);
    vv1.push_back(v21);
    vv1.push_back(v22);

    AFV3_ORTHOGANLIZE(&vv1[0], &vv1[1], &vv1[2]);
    // x
    assert(vv1[0].x == 0.707106829f);
    assert(vv1[0].y == 0.707106829f);
    assert(vv1[0].z == 0.0f);

    // y
    assert(vv1[1].x == -0.577350259f);
    assert(vv1[1].y == 0.577350259f);
    assert(vv1[1].z == -0.577350259f);

    // z
    assert(vv1[2].x == 0.408248335f);
    assert(vv1[2].y == -0.408248216f);
    assert(vv1[2].z == -0.816496551f);


    // Test Area of a triangle method with three points
    AF_Vec3 v30{1.0f, 2.0f, 3.0f};
    AF_Vec3 v31{-2.0f, 2.0f, 4.0f};
    AF_Vec3 v32{7.0f, -8.0f, 6.0f};
    // Three Points Method
    float area = area_of_triangle(v30, v31, v32);
    assert(area == 15.0f);
    
    printf("ALL TESTS PASSED :) \n");



}
*/