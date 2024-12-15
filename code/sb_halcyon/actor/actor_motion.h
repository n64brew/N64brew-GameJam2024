#ifndef ACTOR_MOVEMENT_H
#define ACTOR_MOVEMENT_H

#define ACTOR_GRAVITY -2250

// function prototypes

void actorMotion_setHorizontalAcceleration(Actor *actor, float target_speed, float acceleration_rate);

void actorMotion_setHorizontalInertiaAcceleration(Actor *actor, float target_speed, float acceleration_rate);

void actorMotion_setStopingAcceleration(Actor *actor);

void actorMotion_setJumpAcceleration(Actor *actor, float target_speed, float acceleration_rate);

void actorMotion_integrate(Actor *actor, float frame_time);

void actorMotion_setHorizontalAcceleration(Actor *actor, float target_speed, float acceleration_rate)
{
    actor->target_velocity.x = target_speed * fm_sinf(rad(actor->target_yaw));
    actor->target_velocity.y = target_speed * -fm_cosf(rad(actor->target_yaw));

    actor->body.acceleration.x = acceleration_rate * (actor->target_velocity.x - actor->body.velocity.x);
    actor->body.acceleration.y = acceleration_rate * (actor->target_velocity.y - actor->body.velocity.y);
}

void actorMotion_setHorizontalInertiaAcceleration(Actor *actor, float target_speed, float acceleration_rate)
{
    actor->target_velocity.x = target_speed * fm_sinf(rad(actor->body.rotation.z));
    actor->target_velocity.y = target_speed * -fm_cosf(rad(actor->body.rotation.z));

    actor->body.acceleration.x = acceleration_rate * (actor->target_velocity.x - actor->body.velocity.x);
    actor->body.acceleration.y = acceleration_rate * (actor->target_velocity.y - actor->body.velocity.y);
}

void actorMotion_setStopingAcceleration(Actor *actor)
{
    actor->body.acceleration.x = actor->settings.idle_acceleration_rate * (0 - actor->body.velocity.x);
    actor->body.acceleration.y = actor->settings.idle_acceleration_rate * (0 - actor->body.velocity.y);
}

void actorMotion_setJumpAcceleration(Actor *actor, float target_speed, float acceleration_rate)
{
    actor->body.acceleration.z = acceleration_rate * (target_speed - actor->body.velocity.z);
}

void actorMotion_integrate(Actor *actor, float frame_time)
{

    if (actor->body.acceleration.x != 0 || actor->body.acceleration.y != 0 || actor->body.acceleration.z != 0)
    {
        vector3_addScaledVector(&actor->body.velocity, &actor->body.acceleration, frame_time);
    }

    if (fabsf(actor->body.velocity.x) < 10.0f && fabsf(actor->body.velocity.y) < 10.0f)
    {
        actor->body.velocity.x = 0;
        actor->body.velocity.y = 0;
    }

    if (actor->body.velocity.x != 0 || actor->body.velocity.y != 0 || actor->body.velocity.z != 0)
    {
        vector3_addScaledVector(&actor->body.position, &actor->body.velocity, frame_time);
        if (actor->body.velocity.z < actor->settings.fall_max_speed)
            actor->body.velocity.z = actor->settings.fall_max_speed;
        if (actor->body.velocity.z > actor->settings.jump_max_speed)
            actor->body.velocity.z = actor->settings.jump_max_speed;
        if (actor->body.position.z > actor->settings.jump_max_height)
        {
            actor->body.velocity.z -= 10.0f;
        }
    }

    if (actor->body.velocity.x != 0 || actor->body.velocity.y != 0)
    {

        actor->body.rotation.z = deg(fm_atan2f(-actor->body.velocity.x, -actor->body.velocity.y));

        Vector2 horizontal_velocity = {actor->body.velocity.x, actor->body.velocity.y};
        actor->horizontal_speed = vector2_magnitude(&horizontal_velocity);
    }
}

void actorMotion_setIdle(Actor *actor)
{
    actorMotion_setStopingAcceleration(actor);

    if (fabsf(actor->body.velocity.x) < 1.0f && fabsf(actor->body.velocity.y) < 1.0f)
    {

        vector3_init(&actor->body.velocity);
        actor->horizontal_speed = 0;
        actor->target_yaw = actor->body.rotation.z;
    }
}

void actorMotion_setRunning(Actor *actor)
{
    actorMotion_setHorizontalAcceleration(actor, actor->horizontal_target_speed, actor->settings.run_acceleration_rate);
    // actorMotion_setHorizontalAcceleration (actor, actor->settings.run_target_speed, actor->settings.run_acceleration_rate);
}

void actorMotion_setJump(Actor *actor)
{
    if (actor->input.jump_hold && !actor->input.jump_released && actor->input.jump_time_held < actor->settings.jump_timer_max)
    {
        // Scale horizontal boost based on how long the jump button is held
        float hold_factor = (float)actor->input.jump_time_held / actor->settings.jump_timer_max;
        float boosted_speed = actor->horizontal_speed + (actor->settings.jump_horizontal_boost * hold_factor);

        // Set horizontal acceleration to the boosted speed
        actorMotion_setHorizontalAcceleration(actor, boosted_speed, actor->settings.aerial_control_rate);

        // Maintain vertical acceleration for the jump
        actorMotion_setJumpAcceleration(actor, actor->settings.jump_target_speed, actor->settings.jump_acceleration_rate);
    }
    else if (actor->body.velocity.z > 0)
    {

        // Scale horizontal boost based on how long the jump button is held
        float hold_factor = (float)actor->input.jump_time_held / actor->settings.jump_timer_max;
        float degraded_speed = actor->horizontal_speed - (hold_factor * 0.5f);

        // Maintain aerial control while falling
        actorMotion_setHorizontalAcceleration(actor, degraded_speed, actor->settings.aerial_control_rate);
        actor->body.acceleration.z = ACTOR_GRAVITY;
    }
    else
    {
        // Transition to falling state
        actor->state = FALLING;
        actor->input.jump_time_held = 0;
        return;
    }
}

void actorMotion_setFalling(Actor *actor)
{
    actor->grounded = 0;
    actorMotion_setHorizontalAcceleration(actor, actor->horizontal_speed, actor->settings.aerial_control_rate);
    actor->body.acceleration.z = ACTOR_GRAVITY;

    if (actor->body.position.z <= actor->grounding_height)
    {

        actor->grounded = 1;
        actor->body.acceleration.z = 0;
        actor->body.velocity.z = 0;
        actor->body.position.z = actor->grounding_height;

        actor->state = STAND_IDLE;

        return;
    }
}

void actor_setMotion(Actor *actor, float frame_time)
{
    switch (actor->state)
    {

    case STAND_IDLE:
    {
        actorMotion_setIdle(actor);
        break;
    }
    case RUNNING:
    {
        actorMotion_setRunning(actor);
        break;
    }
    case JUMP:
    {
        actorMotion_setJump(actor);
        break;
    }
    case FALLING:
    {
        actorMotion_setFalling(actor);
        break;
    }
    }

    actorMotion_integrate(actor, frame_time);
}

#endif