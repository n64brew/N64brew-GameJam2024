/**
 * @file
 *
 * This file contains the definitions for the rigid body class, the
 * basic building block of all the physics system.
 */

#ifndef RIGID_BODY_H
#define RIGID_BODY_H

typedef struct RigidBody
{

    Vector3 acceleration;
    Vector3 velocity;
    Vector3 position;
    Vector3 rotation;

    Vector3 previous_position;

    // Quaternion orientation;

} RigidBody;

#endif
