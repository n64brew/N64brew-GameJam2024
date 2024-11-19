#ifndef AI_H
#define AI_H

typedef struct {
    float jump_threshold;
    float safe_height;
    uint8_t difficulty;
    uint8_t error_margin;
    uint8_t reaction_delay;
    uint8_t max_reaction_delay;
} AI;

void ai_init(AI *ai, uint8_t difficulty);
void ai_generateControlData(AI *ai, ControllerData *control, Actor *actor, Platform *platforms, size_t platform_count, float camera_angle);

void ai_init(AI *ai, uint8_t difficulty)
{

    // Reset reaction delay for fresh initialization
    ai->reaction_delay = 0;

    // Set other fields based on difficulty
    switch(difficulty)
    {
        case DIFF_EASY:
            ai->jump_threshold = 300.0f;
            ai->safe_height = 240.0f;
            ai->difficulty = DIFF_EASY;
            ai->error_margin = 12;
            ai->max_reaction_delay = 8;
            break;
        case DIFF_MEDIUM:
            ai->jump_threshold = 350.0f;
            ai->safe_height = 225.0f;
            ai->difficulty = DIFF_MEDIUM;
            ai->error_margin = 8;
            ai->max_reaction_delay = 4;
            break;
        case DIFF_HARD:
            ai->jump_threshold = 400.0f;
            ai->safe_height = 212.5f;
            ai->difficulty = DIFF_HARD;
            ai->error_margin = 4;
            ai->max_reaction_delay = 2;
            break;
    }

}

// Helper function to rotate input by camera angle
void ai_updateCam(ControllerData *control, float camera_angle)
{
    // Convert angle to radians
    float angle_rad = camera_angle * (T3D_PI / 180.0f);
    int8_t original_x = control->input.stick_x;
    int8_t original_y = control->input.stick_y;

    // Rotate stick_x and stick_y based on camera angle
    control->input.stick_x = (int8_t)(original_x * fm_cosf(angle_rad) - original_y * fm_sinf(angle_rad));
    control->input.stick_y = (int8_t)(original_x * fm_sinf(angle_rad) + original_y * fm_cosf(angle_rad));
}

// Function to find the nearest platform at a safe height (position.z > 0)
Platform* find_nearest_safe_platform(AI *ai, Actor *actor, Platform* platforms, size_t platform_count) {
    Platform* nearest_platform = NULL;
    float min_distance = FLT_MAX; // Large initial value
    const float current_platform_threshold = 0.01f;

    for (size_t i = 0; i < platform_count; ++i)
    {
        Platform* platform = &platforms[i];

        // Skip platforms not at a safe height
        if (platform->position.z <= ai->safe_height) continue;

        // Calculate the vector distance to the platform
        float distance = vector3_distance(&platform->position, &actor->body.position);

        // Ignore the current platform the AI is standing on
        if (distance < current_platform_threshold) continue;

        // Check if this platform is the nearest valid one
        if (distance < min_distance)
        {
            min_distance = distance;
            nearest_platform = platform;
        }
    }

    return nearest_platform;
}

// Generate control data for the AI
void ai_generateControlData(AI *ai, ControllerData *control, Actor *actor, Platform *platforms, size_t platform_count, float camera_angle)
{
    // Initialize control data to zero for each frame
    memset(control, 0, sizeof(ControllerData));

    // Set difficulty-based reaction delay
    if (ai->reaction_delay < ai->max_reaction_delay) {
        ai->reaction_delay++;
        return;  // Skip processing this frame to simulate slower reaction
    }

    // Find the nearest safe platform
    Platform* target_platform = find_nearest_safe_platform(ai, actor, platforms, platform_count);
    if (target_platform == NULL) return; // No valid platform found, do nothing

    // Calculate direction towards the target platform
    Vector3 direction_to_target = {
        target_platform->position.x - actor->body.position.x,
        target_platform->position.y - actor->body.position.y,
        0  // Only move in x-y plane, no need for z movement
    };
    vector3_normalize(&direction_to_target);

    // Set movement based on target distance
    control->input.stick_x = (int8_t)(direction_to_target.x * 127); // Max joystick range
    control->input.stick_y = (int8_t)(direction_to_target.y * 127);

    // Add slight randomness to mimic human error, increasing with easier difficulties
    control->input.stick_x += (rand() % ai->error_margin) - (ai->error_margin / 2);
    control->input.stick_y += (rand() % ai->error_margin) - (ai->error_margin / 2);

    // Calculate horizontal speed as the magnitude of the x and y velocity components
    float horizontal_speed = sqrtf(actor->body.velocity.x * actor->body.velocity.x + actor->body.velocity.y * actor->body.velocity.y);

    // Check if the actor should jump when horizontal speed is greater than 10
    if (horizontal_speed > ai->jump_threshold && actor->state != FALLING)
    {
        control->pressed.a = 1; // Press jump button
        control->held.a = 1;    // Hold jump button
    }

    // Adjust for camera angle if needed
    ai_updateCam(control, camera_angle);

}


#endif // AI_H