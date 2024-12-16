#ifndef TRIANGLE_H
#define TRIANGLE_H

typedef struct
{
    Vector3 start;
    Vector3 end;
} Edge;

// Structure for a triangular plane
typedef struct Triangle
{
    Vector3 vertA;
    Vector3 vertB;
    Vector3 vertC;
    Vector3 normal;
    Edge edge;
} Triangle;

void triangle_setVertices(Triangle *triangle, const Vector3 *vertexA, const Vector3 *vertexB, const Vector3 *vertexC);
bool triangle_containsPoint(const Triangle *triangle, const Vector3 *point);

// Sets up the triangle plane and computes its normal
void triangle_setVertices(Triangle *triangle, const Vector3 *a, const Vector3 *b, const Vector3 *c)
{
    triangle->vertA = *a;
    triangle->vertB = *b;
    triangle->vertC = *c;

    // Calculate the normal by taking the cross product of two edges
    Vector3 edge1 = vector3_difference(b, a);
    Vector3 edge2 = vector3_difference(c, a);
    Vector3 normal = vector3_returnCrossProduct(&edge1, &edge2);
    vector3_normalize(&normal);
    triangle->normal = normal;
}

// Check if a point is on or near the plane of the triangle
bool triangle_containsPoint(const Triangle *triangle, const Vector3 *point)
{
    // Compute vector from point to one vertex
    Vector3 point_to_a = vector3_difference(point, &triangle->vertA);

    // Compute dot product to find the distance to the plane along the normal
    float distance = vector3_returnDotProduct(&point_to_a, &triangle->normal);

    // Consider the point on the plane if the distance is near zero (tolerance can be adjusted)
    return fabsf(distance) < TOLERANCE;
}

// Initialize edges of the hexagon from vertices
void hex_initEdges(Edge *edges, Vector3 *vertices)
{
    for (int i = 0; i < 6; ++i)
    {
        edges[i].start = vertices[i];
        edges[i].end = vertices[(i + 1) % 6]; // Loop back to the first vertex
    }
}

// Initialize each triangle in the hexagon
void hex_init(Triangle *hexagon, Vector3 *center, Vector3 *vertices)
{
    triangle_setVertices(&hexagon[0], center, &vertices[0], &vertices[1]);
    triangle_setVertices(&hexagon[1], center, &vertices[1], &vertices[2]);
    triangle_setVertices(&hexagon[2], center, &vertices[2], &vertices[3]);
    triangle_setVertices(&hexagon[3], center, &vertices[3], &vertices[4]);
    triangle_setVertices(&hexagon[4], center, &vertices[4], &vertices[5]);
    triangle_setVertices(&hexagon[5], center, &vertices[5], &vertices[0]);

    hex_initEdges(&hexagon->edge, vertices);
}

#endif // TRIANGLE_H
