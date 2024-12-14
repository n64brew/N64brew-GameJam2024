//#ifndef COLLISION_PIZZA_H_INCLUDED
#include "collision.h"
//#endif

uint32_t sphere_tri_counter;
uint32_t capsule_tri_counter;
uint32_t capsule_mesh_counter;
uint32_t chunk_counter;
uint32_t part_counter;
uint32_t indicies_counter;

bool CollideSphereSphere(const SphereCollider* s1, const SphereCollider* s2, T3DVec3* penetration_normal, float* penetration_depth)//requires sphere center location of both spheres and each radius
{
    //get distance between spheres. If it's less than the sum of their radiuses, they are intersecting
    float distSquared = t3d_vec3_distance2(&s1->center, &s2->center);
    float radius = s1->radius + s2->radius;
    if (distSquared <= radius * radius)
    {
        //Collision occured
        float dist = sqrtf(distSquared);
        *penetration_depth = radius - dist;
        t3d_vec3_diff(penetration_normal, &s1->center, &s2->center);
        fast_vec3_norm(penetration_normal);

    //debugf("Penetration Depth = %f\n", *penetration_depth);
    //debugf("radius 1 = %f, radius 2 = %f\n", s1->radius, s2->radius);

        return true;
    }
    else
    {
        //No Collision
        return false;
    }
}//!!!Seems to be an issue where if radius is a certain radius (10, 30) it won't work???
//returns weather it's a hit, penetration depth, and penetration normal

bool CollideSphereTriangle(const T3DVec3* verticies, const SphereCollider* sphere, T3DVec3* penetration_normal, float* penetration_depth)//Requires tri struct, sphere center/radius
{
    //debugf("Cammy_0\n");
    //Get distance between the center of the sphere and the "plane" of the triangle by taking the dot product of the
        // center - p0 and the normal vector of the triangle (gotten via cross product)
    //sphere_tri_counter++;

    T3DVec3 v1v0;
    t3d_vec3_diff(&v1v0, &verticies[1], &verticies[0]);
    T3DVec3 v2v0;
    t3d_vec3_diff(&v2v0, &verticies[2], &verticies[0]);
    T3DVec3 N;
    t3d_vec3_cross(&N, &v1v0, &v2v0);
    fast_vec3_norm(&N);
    //debugf("Cammy_1\n");

    T3DVec3 centerv0;
    t3d_vec3_diff(&centerv0, &sphere->center, &verticies[0]);
    float dist = t3d_vec3_dot(&centerv0, &N);//signed dist between sphere and plane of the tri.
    //if the distance is greater than the radius (not touching) or less than negative radius (not touching opposite side)
        //then their is definitely no collision.
    if (dist < -sphere->radius || dist > sphere->radius)
    {
        //continue;
        return false;
    }

    //Next, see if there's any point in the triangle that is inside the sphere or not.
    //Case 1. the sphere center projected to the plane is inside the triangle perimeter (face)
    T3DVec3 scaledN = N;
    scaleVector(&scaledN, dist);
    T3DVec3 point0;//sphere->center - N * dist;
    t3d_vec3_diff(&point0, &sphere->center, &scaledN);

    //determine if this point is withing all tri edges
    T3DVec3 c0;
    T3DVec3 p0v0;
    //T3DVec3 v1v0;
    t3d_vec3_diff(&p0v0, &point0, &verticies[0]);
    //t3d_vec3_diff(&v1v0, &verticies[1], &verticies[0]);
    t3d_vec3_cross(&c0, &p0v0, &v1v0);

    T3DVec3 c1;
    T3DVec3 p0v1;
    T3DVec3 v2v1;
    t3d_vec3_diff(&p0v1, &point0, &verticies[1]);
    t3d_vec3_diff(&v2v1, &verticies[2], &verticies[1]);
    t3d_vec3_cross(&c1, &p0v1, &v2v1);

    T3DVec3 c2;
    T3DVec3 p0v2;
    T3DVec3 v0v2;
    t3d_vec3_diff(&p0v2, &point0, &verticies[2]);
    t3d_vec3_diff(&v0v2, &verticies[0], &verticies[2]);
    t3d_vec3_cross(&c2, &p0v2, &v0v2);
    //debugf("Cammy_2\n");
    bool inside = ( t3d_vec3_dot(&c0, &N) <= 0.f && t3d_vec3_dot(&c1, &N) <= 0.f && t3d_vec3_dot(&c2, &N) <= 0.f );
    //if true, use point0 as the closest point on the sphere

    //if false, we're still not sure if we're intersecting or not.
    //Case 2. It's not on the face, so maybe it's on one of the edges?
    // find the closest point to the sphere center on all edges, choose the closest of the three and determine 
    //whether it's within the sphere radius
    float radius2 = sphere->radius * sphere->radius;

    //Edge 1:
    T3DVec3 point1;
    ClosestPointOnLineSegment(&point1, &verticies[0], &verticies[1], &sphere->center);
    T3DVec3 E1;
    t3d_vec3_diff(&E1, &sphere->center, &point1);
    float distsq1 = t3d_vec3_len2(&E1);
    bool intersects = distsq1 < radius2;

    //Edge 2:
    T3DVec3 point2;
    ClosestPointOnLineSegment(&point2, &verticies[1], &verticies[2], &sphere->center);
    T3DVec3 E2;
    t3d_vec3_diff(&E2, &sphere->center, &point2);
    float distsq2 = t3d_vec3_len2(&E2);
    intersects = distsq2 < radius2;

    //Edge 3:
    T3DVec3 point3;
    ClosestPointOnLineSegment(&point3, &verticies[2], &verticies[0], &sphere->center);
    T3DVec3 E3;
    t3d_vec3_diff(&E3, &sphere->center, &point3);
    float distsq3 = t3d_vec3_len2(&E3);
    intersects = distsq3 < radius2;
     //debugf("Cammy_3\n");

    //If one of these cases is true, we must calculate penetration depth and normal.
    if (inside || intersects)
    {
        //find best point, start with point0
        T3DVec3 best_point = point0;
        T3DVec3 intersection_vec;

        if (inside)
        {
            t3d_vec3_diff(&intersection_vec, &sphere->center, &point0);
        }
        else
        {
            T3DVec3 d;
            t3d_vec3_diff(&d, &sphere->center, &point1);
            float best_distsq = t3d_vec3_len2(&d);
            best_point = point1;
            intersection_vec = d;

            t3d_vec3_diff(&d, &sphere->center, &point2);
            float distsq = t3d_vec3_len2(&d);
            if (distsq < best_distsq)
            {
                //distsq = best_distsq;//is this right????????????????
                best_distsq = distsq;
                best_point = point2;
                intersection_vec = d;
            }

            t3d_vec3_diff(&d, &sphere->center, &point3);
            distsq = t3d_vec3_len2(&d);
            if (distsq < best_distsq)
            {
                //distsq = best_distsq;//is this right????????????????
                best_distsq = distsq;
                best_point = point3;
                intersection_vec = d;
            }
        }
        //debugf("Cammy_4\n");
        float len = t3d_vec3_len(&intersection_vec);
        //debugf("Cammy_len= %f\n", len);
        *penetration_normal = intersection_vec;
        /*float inverse_len = 1.f / len;
        *penetration_normal = (T3DVec3) {{
        intersection_vec.v[0] * inverse_len,
        intersection_vec.v[1] * inverse_len,
        intersection_vec.v[2] * inverse_len
        }};*/
        //debugf("Pen_Normal Before = %f %f %f\n", penetration_normal->v[0], penetration_normal->v[1], penetration_normal->v[2]);
        fast_vec3_norm(penetration_normal);
        //debugf("Pen_Normal = %f %f %f\n", penetration_normal->v[0], penetration_normal->v[1], penetration_normal->v[2]);
        *penetration_depth = sphere->radius - len;
        //debugf("Pen_Depth = %f\n", *penetration_depth);
        //return true for intersection success
        //debugf("Cammy_5\n");
        return true;
    }
    //debugf("Cammy_what?\n");
    return false;
}
//Returns

void CollideCapsuleSphere()//Very similar to sphere-sphere collision, just find closes point on line to sphere for the capsule
{

}

bool CollideCapsuleCapsule(const CapsuleCollider* c1, const CapsuleCollider* c2, T3DVec3* penetration_normal, float* penetration_depth)//Requires Tip and Base points, as well as sphere stuff, for both
{
    //Must find the most relevant spheres on both capsules
    //and find best points on the line of both capsules (best A and best B)
    //First, generate values:
    T3DVec3 c1_normal;
    t3d_vec3_diff(&c1_normal, &c1->tip, &c1->base);
    fast_vec3_norm(&c1_normal);
    scaleVector(&c1_normal, c1->radius);
    T3DVec3 c1_lineEndOffset = c1_normal;
    T3DVec3 c1_A;
    t3d_vec3_add(&c1_A, &c1->base, &c1_lineEndOffset);
    T3DVec3 c1_B;
    t3d_vec3_diff(&c1_B, &c1->tip, &c1_lineEndOffset);//could potentially keep this stuff in collider, not have to calc eachtime?
    
    T3DVec3 c2_normal;
    t3d_vec3_diff(&c2_normal, &c2->tip, &c2->base);
    fast_vec3_norm(&c2_normal);
    scaleVector(&c2_normal, c2->radius);
    T3DVec3 c2_lineEndOffset = c2_normal;
    T3DVec3 c2_A;
    t3d_vec3_add(&c2_A, &c2->base, &c2_lineEndOffset);
    T3DVec3 c2_B;
    t3d_vec3_diff(&c2_B, &c2->tip, &c2_lineEndOffset);

    //take vectors and distances between all 4 endpoints
    T3DVec3 v0;
    T3DVec3 v1;
    T3DVec3 v2;
    T3DVec3 v3;
    t3d_vec3_diff(&v0, &c2_A, &c1_A);
    t3d_vec3_diff(&v1, &c2_B, &c1_A);
    t3d_vec3_diff(&v2, &c2_A, &c1_B);
    t3d_vec3_diff(&v3, &c2_B, &c1_B);

        //squared distances (what's the diff between this and t3d_vec3_distance2????)
    float d0 = t3d_vec3_len2(&v0);
    float d1 = t3d_vec3_len2(&v1);
    float d2 = t3d_vec3_len2(&v2);
    float d3 = t3d_vec3_len2(&v3);

    //Find best potential endpoints on both capsules
    T3DVec3 bestA;
    if (d2 < d0 || d2 < d1 || d3 < d0 || d3 < d1)
    {
        bestA = c1_B;
    }
    else
    {
        bestA = c1_A;
    }
    //best point on C2 line segment nearest to best potential endpoint on C1 capsule
    T3DVec3 bestB;
    ClosestPointOnLineSegment(&bestB, &c2_A, &c2_B, &bestA);
    //and then the same for capsule A segment
    ClosestPointOnLineSegment(&bestA, &c1_A, &c1_B, &bestB);

    // then place spheres on those points and do sphere collision like normal
    //T3DVec3 penetration_normal;
    t3d_vec3_diff(penetration_normal, &bestA, &bestB);
    float len = t3d_vec3_len2(penetration_normal);
    fast_vec3_norm(penetration_normal);
    *penetration_depth = c1->radius + c2->radius - len;
    bool intersects = *penetration_depth > 0;

    return intersects;
}
//Returns

bool CollideCapsuleTriangle(const T3DVec3* verticies, const CapsuleCollider* capsule, T3DVec3* penetration_normal, float* penetration_depth)//Will eventually go to sphere-triangle. Requires tri struct, and requires Tip and Base points, 
{
    if (!TestAABBCapsuleTriangle(capsule, verticies))
    {
        //debugf("$$$$$$$$$$$$$$$$$CollideCapsuleTriangle : failed AABB test, don't try actual collision.\n");
        sphere_tri_counter++;
        return false;
    }
    //Must find the reference point (closest point on capsule line to the triangle) not just the closest point on the capsule
    capsule_tri_counter++;
    T3DVec3 CapsuleNormal;
    t3d_vec3_diff(&CapsuleNormal, &capsule->tip, &capsule->base);
    fast_vec3_norm(&CapsuleNormal);
    T3DVec3 lineEndOffset = CapsuleNormal;
    scaleVector(&lineEndOffset, capsule->radius);
    T3DVec3 capsule_A;
    t3d_vec3_add(&capsule_A, &capsule->base, &lineEndOffset);
    T3DVec3 capsule_B;
    t3d_vec3_diff(&capsule_B, &capsule->tip, &lineEndOffset);//could potentially keep this stuff in collider, not have to calc eachtime?

    //float t = dot(N, (p0 - base) / abs(dot(N, CapsuleNormal)));
    T3DVec3 v1v0;
    t3d_vec3_diff(&v1v0, &verticies[1], &verticies[0]);
    T3DVec3 v2v0;
    t3d_vec3_diff(&v2v0, &verticies[2], &verticies[0]);
    T3DVec3 N;
    t3d_vec3_cross(&N, &v1v0, &v2v0);
    fast_vec3_norm(&N);

    float dotncap = t3d_vec3_dot(&N, &CapsuleNormal);
    dotncap = fabs(dotncap);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    T3DVec3 reference_point;

    if (dotncap > 0.0001f)
    {
        T3DVec3 v0base;
        t3d_vec3_diff(&v0base, &verticies[0], &capsule->base);
        float dotNv0base = t3d_vec3_dot(&N, &v0base);

        float t = dotNv0base / dotncap;

        T3DVec3 line_plane_intersection;// = base + CapsuleNormal * t;
        T3DVec3 tcapNorm = CapsuleNormal;
        scaleVector(&tcapNorm, t);
        t3d_vec3_add(&line_plane_intersection, &tcapNorm, &capsule->base);


            // find closest point on triangle to line_plane_intersection, same as sphere-tri case
        //******************************************************************************************* */
        //instead of the sphere centre projection, we use the line_plane_intersection point
        //Determine whether line_plane_intersection is inside ALL triangle edges

        T3DVec3 c0;
        T3DVec3 L0v0;
        //T3DVec3 v1v0;
        t3d_vec3_diff(&L0v0, &line_plane_intersection, &verticies[0]);
        //t3d_vec3_diff(&v1v0, &verticies[1], &verticies[0]);
        t3d_vec3_cross(&c0, &L0v0, &v1v0);

        T3DVec3 c1;
        T3DVec3 L0v1;
        T3DVec3 v2v1;
        t3d_vec3_diff(&L0v1, &line_plane_intersection, &verticies[1]);
        t3d_vec3_diff(&v2v1, &verticies[2], &verticies[1]);
        t3d_vec3_cross(&c1, &L0v1, &v2v1);

        T3DVec3 c2;
        T3DVec3 L0v2;
        T3DVec3 v0v2;
        t3d_vec3_diff(&L0v2, &line_plane_intersection, &verticies[2]);
        t3d_vec3_diff(&v0v2, &verticies[0], &verticies[2]);
        t3d_vec3_cross(&c2, &L0v2, &v0v2);

        bool inside = ( t3d_vec3_dot(&c0, &N) <= 0.f && t3d_vec3_dot(&c1, &N) <= 0.f && t3d_vec3_dot(&c2, &N) <= 0.f );

        if (inside)
        {
            //t3d_vec3_diff(&intersection_vec, &sphere->center, &point0);
            reference_point = line_plane_intersection;
        }
        else
        {
            //Edge 1:
            T3DVec3 point1;
            ClosestPointOnLineSegment(&point1, &verticies[0], &verticies[1], &line_plane_intersection);
            T3DVec3 E1;
            t3d_vec3_diff(&E1, &line_plane_intersection, &point1);
            float distsq = t3d_vec3_len2(&E1);
            float best_dist = distsq;
            reference_point = point1;

            //Edge 2:
            T3DVec3 point2;
            ClosestPointOnLineSegment(&point2, &verticies[1], &verticies[2], &line_plane_intersection);
            T3DVec3 E2;
            t3d_vec3_diff(&E2, &line_plane_intersection, &point2);
            distsq = t3d_vec3_len2(&E2);
            if (distsq < best_dist)
            {
                reference_point = point2;
                best_dist = distsq;
            }

            //Edge 3:
            T3DVec3 point3;
            ClosestPointOnLineSegment(&point3, &verticies[2], &verticies[0], &line_plane_intersection);
            T3DVec3 E3;
            t3d_vec3_diff(&E3, &line_plane_intersection, &point3);
            distsq = t3d_vec3_len2(&E3);
            if (distsq < best_dist)
            {
                reference_point = point3;
                best_dist = distsq;
            }
        }
    }
    else{
        reference_point = verticies[0];
    }




    //center of the best sphere candidate
    T3DVec3 center;
    //debugf("Capsule_A: %.2f, %.2f, %.2f\n", capsule_A.v[0], capsule_A.v[1], capsule_A.v[2]);
    //debugf("capsule_B: %.2f, %.2f, %.2f\n", capsule_B.v[0], capsule_B.v[1], capsule_B.v[2]);
    //debugf("reference_point: %.2f, %.2f, %.2f\n", reference_point.v[0], reference_point.v[1], reference_point.v[2]);

    ClosestPointOnLineSegment(&center, &capsule_A, &capsule_B, &reference_point);
    
    //CreateSphereCollider
    SphereCollider BestSphere = {
        capsule->radius,
        center
    };
    T3DVec3 sphere_penetration_normal;
    float sphere_penetration_depth;
    //Finish with sphere-triangle intersection, do we need to do the whole thing? only last half?
    bool iscollide = CollideSphereTriangle(verticies, &BestSphere, &sphere_penetration_normal, &sphere_penetration_depth);

    *penetration_normal = sphere_penetration_normal;
    *penetration_depth = sphere_penetration_depth;
    return iscollide;
    //return false;

    /*
    There is a corner case with this. When the capsule line is parallel to the triangle plane, 
    the ray – plane trace will fail. You can detect this case by checking the dot product between
     the ray direction (CapsuleNormal) and triangle normal (N). If it’s zero, then they are parallel.
      When this happens, instead of relying on the tracing, a simple fix is to just take an arbitrary
       point (for example the first corner) from the triangle and use that as reference point. The
        reference point will be used the same way as the intersection point, to determine the closest
         point on the capsule A-B segment.
    */

}
//returns

bool CollideCapsuleMesh(const T3DModel* model, const T3DMat4* mat, const CapsuleCollider* capsule, T3DVec3* penetration_normal, float* penetration_depth)//Will eventually go to sphere-triangle. Requires tri struct, and requires Tip and Base points, 
{
    /*bool iscollide = false;
    T3DVec3 penetration_normal_1;
    float penetration_depth_1;
    T3DVec3 penetration_normal_2;
    float penetration_depth_2;


    T3DVec3 verticies[3];
    verticies[0] = (T3DVec3){{//Purely temporary to avoid errors while working
      verts[0].posA[0] + mat->m[3][0],//0
      verts[0].posA[1] + mat->m[3][1],
      verts[0].posA[2] + mat->m[3][2]
    }};
    verticies[1] = (T3DVec3){{
      verts[0].posB[0] + mat->m[3][0],//1
      verts[0].posB[1] + mat->m[3][1],
      verts[0].posB[2] + mat->m[3][2]
    }};
    verticies[2] = (T3DVec3){{
      verts[1].posA[0] + mat->m[3][0],//2
      verts[1].posA[1] + mat->m[3][1],
      verts[1].posA[2] + mat->m[3][2]
    }};


    if (CollideCapsuleTriangle(verticies, capsule, &penetration_normal_1, &penetration_depth_1))
    {
        iscollide = true;
        *penetration_normal = penetration_normal_1;
        *penetration_depth = penetration_depth_1;
    }

    verticies[0] = (T3DVec3){{//Purely temporary to avoid errors while working
      verts[1].posA[0] + mat->m[3][0],//2
      verts[1].posA[1] + mat->m[3][1],
      verts[1].posA[2] + mat->m[3][2]
    }};
    verticies[1] = (T3DVec3){{
      verts[1].posB[0] + mat->m[3][0],//3
      verts[1].posB[1] + mat->m[3][1],
      verts[1].posB[2] + mat->m[3][2]
    }};
    verticies[2] = (T3DVec3){{
      verts[0].posA[0] + mat->m[3][0],//0
      verts[0].posA[1] + mat->m[3][1],
      verts[0].posA[2] + mat->m[3][2]
    }};
    if (CollideCapsuleTriangle(verticies, capsule, &penetration_normal_2, &penetration_depth_2))
    {
        if (iscollide == true)
        {
            if (penetration_depth_1 > penetration_depth_2)
            {
                *penetration_normal = penetration_normal_1;
                *penetration_depth = penetration_depth_1;
            }
            else{
                *penetration_normal = penetration_normal_2;
                *penetration_depth = penetration_depth_2;
            }
        }
        else{
            iscollide = true;
            *penetration_normal = penetration_normal_2;
            *penetration_depth = penetration_depth_2;
        }
    }
    return iscollide;*/

    capsule_mesh_counter++;
    T3DObject* obj;
    T3DVec3 verticies[3];
    int16_t vertexFP[3][3];
    //debugf("    Chunks: %ld\n", model->chunkCount);
    for(uint32_t c = 0; c < model->chunkCount; c++) {
        chunk_counter++;
        obj = t3d_model_get_nameless_object(model, c);
        if (obj == NULL)
        {
            return false;//shouldn't be return, should be continue???
        }
        //debugf("numParts: %ld\n", obj->numParts);
        for (int i = 0; i < obj->numParts; i++) {
            part_counter++;
            const T3DObjectPart *part = &obj->parts[i];
            //debugf("numIndices: %d\n", part->numIndices);
            for (int j = 0; j < part->numIndices; j += 3) {//this is the most important loop!!!
                indicies_counter++;
                GetVerticles(vertexFP, part, j);
                ConvertVerticies(verticies, vertexFP, mat);
                if (CollideCapsuleTriangle(verticies, capsule, penetration_normal, penetration_depth))
                {
                    return true;
                }
            }
        }

    }

    return false;
}





void ClosestPointOnLineSegment(T3DVec3* res, const T3DVec3* A, const T3DVec3* B, const T3DVec3* Point)//requires two distances (_____) and point
{
    T3DVec3 AB;
    t3d_vec3_diff(&AB, B, A);
    T3DVec3 PointA;
    t3d_vec3_diff(&PointA, Point, A);

    float t;
    if (t3d_vec3_len2(&AB) < 0.001)
    {
        t=1;
    }
    else
    {
        t = t3d_vec3_dot(&PointA, &AB) / t3d_vec3_len2(&AB);
    }
    
    float satT = clamp_Float(t, 0, 1);
    scaleVector(&AB, satT);
    t3d_vec3_add(res, A, &AB);
}

void GetVerticles(int16_t vertex[3][3], const T3DObjectPart *part, int j)//there may be a better built in way of doing this
{

            //int16_t vertex[3][3];


            for (int l = 0; l < 3; l++) {
                int ind = part->indices[j + l];
                for (int k = 0; k < 3; k++) {
                    if ((ind % 2) == 0) {
                        vertex[l][k] = part->vert[ind / 2].posA[k];// divide by aint great, neither is mod. Not sure if can remove them
                    } else {
                        vertex[l][k] = part->vert[ind / 2].posB[k];
                    }
                } 
            }


}
//returns struct of 3 vectors for each point of the tri


float clamp_Float(float value, float min, float max)
{
    if (value > max)
    {
        value = max;
    }
    if (value < min)
    {
        value = min;
    }
    return value;
}

void scaleVector(T3DVec3* vector, float scale)
{
    vector->v[0] *= scale;
    vector->v[1] *= scale;
    vector->v[2] *= scale;
}


bool GetObjectFromModel(T3DObject* obj, const T3DModel* model, uint32_t c)//No longer used, use nameless function
{
    //for(uint32_t c = 0; c < model->chunkCount; c++) {
        char chunkType = model->chunkOffsets[c].type;
        if(chunkType != T3D_CHUNK_TYPE_OBJECT) return false;//break;

        uint32_t offset = model->chunkOffsets[c].offset & 0x00FFFFFF;
        T3DObject* objhurray = (T3DObject*)((char*)model + offset);
        obj = objhurray;
        return true;
    //}
}

void ConvertVerticies(T3DVec3* verticies, const int16_t vertexFP[3][3], const T3DMat4* mat)//use mat-mat func, plug vert pos into matrix
{//how to do rotation???
    verticies[0] = (T3DVec3){{
      vertexFP[0][0] + mat->m[3][0],//0
      vertexFP[0][1] + mat->m[3][1],
      vertexFP[0][2] + mat->m[3][2]
    }};
    verticies[1] = (T3DVec3){{
      vertexFP[1][0] + mat->m[3][0],//1
      vertexFP[1][1] + mat->m[3][1],
      vertexFP[1][2] + mat->m[3][2]
    }};
    verticies[2] = (T3DVec3){{
      vertexFP[2][0] + mat->m[3][0],//2
      vertexFP[2][1] + mat->m[3][1],
      vertexFP[2][2] + mat->m[3][2]
    }};
}


T3DObject* t3d_model_get_nameless_object(const T3DModel *model, uint32_t i) {//store list of objects in actor struct??? Don't do this every collision?
  //for(uint32_t i = 0; i < model->chunkCount; i++) {
    if(model->chunkOffsets[i].type == T3D_CHUNK_TYPE_OBJECT) {
      uint32_t offset = model->chunkOffsets[i].offset & 0x00FFFFFF;
      T3DObject *obj = (T3DObject*)((char*)model + offset);
      //if(obj->name && strcmp(obj->name, name) == 0)
      return obj;
    }
  //}
  return NULL;
}

void fast_vec3_norm(T3DVec3 *res) { 
  float len = sqrtf(t3d_vec3_len2(res));
  if(len < 0.0001f)len = 0.0001f;
  len = 1.0f / len;
  res->v[0] *= len;
  res->v[1] *= len;
  res->v[2] *= len;
}

bool TestAABBCapsuleTriangle(const CapsuleCollider* capsule, const T3DVec3* vertices)
{
    T3DVec3 tri_min;
    T3DVec3 tri_max;

    /* debugf("Begin broad test, let's see if verts the same:\n");
          debugf("%f, %f, %f\n", vertices[0].v[0], vertices[0].v[1], vertices[0].v[2]);
          debugf("%f, %f, %f\n", vertices[0+1].v[0], vertices[0+1].v[1], vertices[0+1].v[2]);
          debugf("%f, %f, %f\n", vertices[0+2].v[0], vertices[0+2].v[1], vertices[0+2].v[2]);
          debugf("Oh, and what about Max and Min:\n");
          debugf("%f, %f, %f\n", capsule->Capsule_AABB_Max.v[0], capsule->Capsule_AABB_Max.v[1], capsule->Capsule_AABB_Max.v[2]);
          debugf("%f, %f, %f\n", capsule->Capsule_AABB_Min.v[0], capsule->Capsule_AABB_Min.v[1], capsule->Capsule_AABB_Min.v[2]);*/

    for (int i = 0; i < 3; i++)
    {
        float min_value = vertices[0].v[i];
        if (vertices[1].v[i] < min_value)
        {
            min_value = vertices[1].v[i];
        }
        if (vertices[2].v[i] < min_value)
        {
            min_value = vertices[2].v[i];
        }
        tri_min.v[i] = min_value;
    }
    for (int i = 0; i < 3; i++)
    {
        float max_value = vertices[0].v[i];
        if (vertices[1].v[i] > max_value)
        {
            max_value = vertices[1].v[i];
        }
        if (vertices[2].v[i] > max_value)
        {
            max_value = vertices[2].v[i];
        }
        tri_max.v[i] = max_value;
    }
    /*debugf("Finally, what about the calculated tri_max and tri_min:\n");
    debugf("%f, %f, %f\n", tri_max.v[0], tri_max.v[1], tri_max.v[2]);
    debugf("%f, %f, %f\n", tri_min.v[0], tri_min.v[1], tri_min.v[2]);*/

    /*if (ans) 
    {
        debugf("capsule_min: %.2f, %.2f, %.2f\n", capsule_min.v[0], capsule_min.v[1], capsule_min.v[2]);
        debugf("capsule_max: %.2f, %.2f, %.2f\n", capsule_max.v[0], capsule_max.v[1], capsule_max.v[2]);
        debugf("tri_min: %.2f, %.2f, %.2f\n", tri_min.v[0], tri_min.v[1], tri_min.v[2]);
        debugf("tri_max: %.2f, %.2f, %.2f\n", tri_max.v[0], tri_max.v[1], tri_max.v[2]);   
    }*/

    return TestAABBvsAABB(&capsule->Capsule_AABB_Min, &capsule->Capsule_AABB_Max, &tri_min, &tri_max);
        /*capsule->Capsule_AABB_Min.v[0] <= tri_max.v[0] && capsule->Capsule_AABB_Max.v[0] >= tri_min.v[0] &&
        capsule->Capsule_AABB_Min.v[1] <= tri_max.v[1] && capsule->Capsule_AABB_Max.v[1] >= tri_min.v[1] &&
        capsule->Capsule_AABB_Min.v[2] <= tri_max.v[2] && capsule->Capsule_AABB_Max.v[2] >= tri_min.v[2]
        );*/
    //return true;
}


bool TestAABBvsAABB(const T3DVec3* aMin, const T3DVec3* aMax, const T3DVec3* bMin, const T3DVec3* bMax)
{
    return aMin->v[0] <= bMax->v[0] && aMax->v[0] >= bMin->v[0] &&
        aMin->v[1] <= bMax->v[1] && aMax->v[1] >= bMin->v[1] &&
        aMin->v[2] <= bMax->v[2] && aMax->v[2] >= bMin->v[2];
    /*T3DVec3 midMin;
    T3DVec3 minMax;
    t3d_vec3_diff(&midMin, aMin, bMax);*/
}

