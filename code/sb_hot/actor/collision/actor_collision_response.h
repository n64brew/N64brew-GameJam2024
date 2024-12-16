#ifndef ACTOR_COLLISION_RESPONSE_H
#define ACTOR_COLLISION_RESPONSE_H

void actorCollision_pushTowardsNormal(Actor *actor, ActorContactData *contact)
{
    // Calculate the necessary displacement vector in the direction of the contact normal
    Vector3 displacement_vector = vector3_returnScaled(&contact->data.normal, -contact->data.penetration);

    // Apply the displacement to the actor's position
    vector3_subtract(&actor->body.position, &displacement_vector);
}

// lighter solution to use together with the push towards normal function. gives almost same results, for now i will use the correct algorithm
void actorCollision_projectAcceleration(Actor *actor, ActorContactData *contact)
{
    float t = vector3_returnDotProduct(&actor->body.acceleration, &contact->data.normal);
    vector3_addScaledVector(&actor->body.acceleration, &contact->data.normal, -t);
}

void actorCollision_projectVelocity(Actor *actor, ActorContactData *contact)
{
    float t = vector3_returnDotProduct(&actor->body.velocity, &contact->data.normal);
    vector3_addScaledVector(&actor->body.velocity, &contact->data.normal, -t);
}

void actorCollision_solvePenetration(Actor *actor, ActorContactData *contact, ActorCollider *collider)
{
    // Normalize the actor's velocity vector
    Vector3 velocity_normal = vector3_returnNormalized(&actor->body.velocity);

    // Calculate the intersection of the ray (contact point + velocity normal) with the plane
    float denominator = vector3_returnDotProduct(&velocity_normal, &contact->data.normal);
    float numerator = contact->displacement + collider->body.radius - vector3_returnDotProduct(&contact->data.point, &contact->data.normal);

    float t;
    if (fabsf(denominator) > 0.0001f)
        t = numerator / denominator;
    else
        return;

    Vector3 axis_closest_at_contact = contact->data.point;
    vector3_addScaledVector(&axis_closest_at_contact, &velocity_normal, t);

    Vector3 displacement_vector = axis_closest_at_contact;
    vector3_subtract(&displacement_vector, &contact->axis_closest_to_point);

    contact->velocity_penetration = displacement_vector;
    vector3_invert(&contact->velocity_penetration);

    vector3_add(&actor->body.position, &displacement_vector);
}

void actorCollision_collideAndSlide(Actor *actor, ActorContactData *contact)
{
    float t = vector3_returnDotProduct(&contact->velocity_penetration, &contact->data.normal);
    Vector3 projection = contact->velocity_penetration;
    vector3_addScaledVector(&projection, &contact->data.normal, -t);

    vector3_add(&actor->body.position, &projection);
}

void actorCollision_setGroundResponse(Actor *actor, ActorContactData *contact, ActorCollider *collider)
{
    actorCollision_pushTowardsNormal(actor, contact);
    actor->grounded = true;
    actor->body.acceleration.z = 0;
    actor->body.velocity.z = 0;
    actor->grounding_height = actor->body.position.z + 2.0f;
    actor->state = actor->previous_state;

    // Lower the ground height slightly when on a slope
    if (contact->slope > 7.0f && contact->slope < 50.0f && contact->ground_distance > 0.1f)
    {
        float slope_offset = 0.1f * contact->slope;
        actor->grounding_height -= slope_offset;
        if (actor->body.velocity.x != 0)
            actor->body.position.z = actor->grounding_height;
    }
}

void actorCollision_setCeilingResponse(Actor *actor, ActorContactData *contact)
{
    if (actor->body.velocity.z > 0)
    {
        vector3_scale(&actor->body.velocity, 1 - (contact->angle_of_incidence * 0.01)); // angle of incidence can be up to 90 degrees
        actor->body.velocity = vector3_reflect(&actor->body.velocity, &contact->data.normal);
        actor->body.velocity.z = 0.0f;
    }
    else
    {
        actor->body.velocity.x = 0.0f;
        actor->body.velocity.y = 0.0f;
    }

    // Possible fix for Sloped Ceilings forcing Player downwards
    if (!(actor->grounded))
    {
        actor->state = FALLING;
        actor->hasCollided = false;
    }
}

void actorCollision_setResponse(Actor *actor, ActorContactData *contact, ActorCollider *collider)
{
    actorContactData_setAngleOfIncidence(contact, &actor->body.velocity);
    actorCollision_solvePenetration(actor, contact, collider);

    if (contact->slope > 0 && contact->slope < 50)
    {
        actorCollision_setGroundResponse(actor, contact, collider);
        actorCollision_collideAndSlide(actor, contact);
    }
    else if (contact->slope > 95 && actor->grounded == false)
    {
        actorCollision_collideAndSlide(actor, contact);
        actorCollision_setCeilingResponse(actor, contact);
    }
    else
    {
        actorCollision_setGroundResponse(actor, contact, collider);
        actorCollision_collideAndSlide(actor, contact);
    }

    actorCollider_setVertical(collider, &actor->body.position);
}

void actorCollision_updateFalling(Actor *actor, ActorContactData *actor_contact, ActorCollider *actor_collider)
{
    actorContactData_clear(actor_contact);
    actorCollider_setVertical(actor_collider, &actor->body.position);

    // Check if the actor is neither jumping nor falling
    if (actor->body.position.z != LOWER_LIMIT_HEIGHT && actor->state != JUMP && actor->state != FALLING && actor->hasCollided == false)
    {

        actor->state = FALLING;
        actor->grounded = false;
        actor->grounding_height = LOWER_LIMIT_HEIGHT;
    }
}

void actorCollision_collidePlatforms(Actor *actor, ActorContactData *actor_contact, ActorCollider *actor_collider, Platform *platforms)
{

    actorCollision_updateFalling(actor, actor_contact, actor_collider);

    // Calculate the grid cell the actor is in
    int xCell = (int)fm_floorf((actor->body.position.x + 775) / 350);
    int yCell = (int)fm_floorf((actor->body.position.y + 775) / 350);

    if (xCell < 0 || xCell >= 7 || yCell < 0 || yCell >= 7)
    {
        // Actor is out of bounds; fall and skip collision
        actor->state = FALLING;
        actor->grounded = false;
        actor->grounding_height = LOWER_LIMIT_HEIGHT;
        if (actor->body.position.z <= DEATH_PLANE_HEIGHT)
            actorState_setDeath(actor);
        return;
    }

    const float collisionRangeSq = 175.0f * 175.0f;

    // Reset actor's collision state
    actor->hasCollided = false;

    // Iterate through platforms in the same and adjacent cells
    for (int dx = -1; dx <= 1; dx++)
    {
        for (int dy = -1; dy <= 1; dy++)
        {
            int nx = xCell + dx;
            int ny = yCell + dy;

            if (nx < 0 || nx >= 7 || ny < 0 || ny >= 7)
            {
                // Actor is out of bounds; fall and skip collision
                actor->state = FALLING;
                actor->grounded = false;
                actor->grounding_height = LOWER_LIMIT_HEIGHT;
                if (actor->body.position.z <= DEATH_PLANE_HEIGHT)
                    actorState_setDeath(actor);
                continue;
            }

            PlatformGridCell *cell = &platformGrid[nx][ny];
            for (size_t i = 0; i < cell->count; i++)
            {
                Platform *platform = &platforms[cell->platformIndices[i]];
                fm_vec3_t actorPos = Vector3_to_fast(actor->body.position);
                fm_vec3_t platformPos = Vector3_to_fast(platform->position);
                float distanceSq = fm_vec3_distance2(&actorPos, &platformPos);
                if (distanceSq <= collisionRangeSq)
                {

                    // Check collision with each box in the platform's collider
                    for (int j = 0; j < 3; j++)
                    {
                        Box *box = &platform->collider.box[j];

                        // If the actor hits a box
                        if (actorCollision_contactBox(actor_collider, box))
                        {
                            // Set collision response
                            actorCollision_contactBoxSetData(actor_contact, actor_collider, box);
                            actorCollision_collideAndSlide(actor, actor_contact);
                            actorCollision_setGroundResponse(actor, actor_contact, actor_collider);

                            // If the actor is lower the top of the box (center.z+(size.z/2)), move there
                            if (actor->body.position.z < box->center.z + (box->size.z * 0.5f))
                                actor->body.position.z = box->center.z + (box->size.z * 0.5f);

                            // Set collided state parameter
                            actor->hasCollided = true;

                            // Handle platform collision here instead again for the platforms
                            platform->contact = true;

                            return; // Early exit if collision is detected
                        }
                    }
                }
            }
        }
    }

    // Call setState after processing collision responses
    actor_setState(actor, actor->state);

    if (actor->body.position.z <= DEATH_PLANE_HEIGHT)
        actorState_setDeath(actor);
}

#endif