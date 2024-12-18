#include "./spark_effect.h"

#include "./math/vector2.h"
#include "./math/mathf.h"
#include "./assets.h"
#include <t3d/t3d.h>

#define GRAVITY_CONSTANT        -600.0f
#define PARTICLE_TAIL_DELAY     0.125f
#define PARTICLE_TIME           0.75f

#define MIN_TANGENT_VEL         20.0f
#define MAX_TANGENT_VEL         30.0f

#define MIN_NORMAL_VEL          120.0f
#define MAX_NORMAL_VEL          160.0f

#define HALF_WIDTH              3.0f


static rspq_block_t* triangles;

void spark_effect_init(struct SparkEffect* effect) {
    effect->time_left = 0.0f;
}

void spark_effect_start(struct SparkEffect* effect, struct Vector3* emit_from) {
    effect->time_left = PARTICLE_TIME;

    struct Vector3 right;
    struct Vector3 up;

    vector3Perp(&gUp, &right);
    vector3Normalize(&right, &right);
    vector3Cross(&gUp, &right, &up);

    for (int i = 0; i < MAX_PARTICLE_COUNT; ++i) {
        struct SparkParticle* particle = &effect->particles[i];

        struct Vector2 tangentDir;
        vector2RandomUnitCircle(&tangentDir);
        float tangentMag = randomInRangef(MIN_TANGENT_VEL, MAX_TANGENT_VEL);
        float normalMag = randomInRangef(MIN_NORMAL_VEL, MAX_NORMAL_VEL);

        vector3Scale(&gUp, &particle->velocity, normalMag);
        vector3AddScaled(&particle->velocity, &right, tangentDir.x * tangentMag, &particle->velocity);
        vector3AddScaled(&particle->velocity, &up, tangentDir.y * tangentMag, &particle->velocity);

        particle->position[1] = *emit_from;
        vector3AddScaled(emit_from, &particle->velocity, PARTICLE_TAIL_DELAY, &particle->position[0]);

        vector3Cross(&particle->velocity, &gUp, &particle->widthOffset);

        float widthMag = vector3MagSqrd(&particle->widthOffset);

        if (widthMag < 0.00001f) {
            vector3Scale(&gRight, &particle->widthOffset, HALF_WIDTH);
            particle->widthOffset = gRight;
        } else {
            vector3Scale(&particle->widthOffset, &particle->widthOffset, HALF_WIDTH / sqrtf(widthMag));
        }
    }
}

void spark_effect_update(struct SparkEffect* effect, float delta_time) {
    if (!effect->time_left) {
        return;
    }

    for (int i = 0; i < MAX_PARTICLE_COUNT; ++i) {
        struct SparkParticle* particle = &effect->particles[i];

        vector3AddScaled(&particle->position[0], &particle->velocity, delta_time, &particle->position[0]);
        vector3AddScaled(&particle->position[1], &particle->velocity, delta_time, &particle->position[1]);

        // this line simulates tracking the yvelocity of the tail separate
        // without needing to track that velocity separately
        // tailYVelocity = yVelocity - effect->def->particleTailDelay * GRAVITY_CONSTANT
        // tailPos.y = tailPos.y + tailYVelocity * FIXED_DELTA_TIME
        // tailPos.y = tailPos.y + (yVelocity - effect->def->particleTailDelay * GRAVITY_CONSTANT) * FIXED_DELTA_TIME
        // tailPos.y = tailPos.y + yVelocity * FIXED_DELTA_TIME - effect->def->particleTailDelay * GRAVITY_CONSTANT * FIXED_DELTA_TIME
        particle->position[1].y -= PARTICLE_TAIL_DELAY * (GRAVITY_CONSTANT * delta_time);


        particle->velocity.y += delta_time * GRAVITY_CONSTANT;
    }

    effect->time_left -= delta_time;

    if (effect->time_left < 0.0f) {
        effect->time_left = 0.0f;
    }
}

static color_t spark_effect_gradient[] = {
    {32, 0, 0, 0},
    {196, 64, 16, 200},
    {220, 128, 32, 255},
    {230, 200, 40, 255},
    {240, 240, 40, 255},
};

#define GRADIENT_COUNT  (sizeof(spark_effect_gradient) / sizeof(spark_effect_gradient[0]))

void spark_effect_render(struct SparkEffect* effect, struct frame_malloc* fm) {
    if (!effect->time_left) {
        return;
    }

    int data_size = sizeof(T3DVertPacked) * MAX_PARTICLE_COUNT * 2;

    T3DVertPacked* vertices = frame_malloc(fm, data_size);

    if (!vertices) {
        return;
    }

    T3DVertPacked* curr = vertices;

    float float_value = effect->time_left * (GRADIENT_COUNT - 1) * (1.0f / PARTICLE_TIME);

    int int_value = (int)floorf(float_value);
    float lerp_value = float_value - int_value;

    color_t color_value;

    if (int_value >= GRADIENT_COUNT - 1) {
        color_value = spark_effect_gradient[GRADIENT_COUNT - 1];
    } else {
        color_t lerp_from = spark_effect_gradient[int_value];
        color_t lerp_to = spark_effect_gradient[int_value + 1];

        float inv_lerp = 1.0f - lerp_value;

        color_value.r = (int)(lerp_from.r * inv_lerp + lerp_to.r * lerp_value);
        color_value.g = (int)(lerp_from.g * inv_lerp + lerp_to.g * lerp_value);
        color_value.b = (int)(lerp_from.b * inv_lerp + lerp_to.b * lerp_value);
        color_value.a = (int)(lerp_from.a * inv_lerp + lerp_to.a * lerp_value);
    }

    rdpq_set_prim_color(color_value);

    for (int i = 0; i < MAX_PARTICLE_COUNT; i += 1) {
        for (int half = 0; half < 2; half += 1) {

            curr->normA = 0;
            curr->normB = 0;
            curr->rgbaA = 0xFFFFFFFF;
            curr->rgbaB = 0xFFFFFFFF;

            struct Vector3 pos;
            vector3Add(&effect->particles[i].position[half], &effect->particles[i].widthOffset, &pos);
            curr->posA[0] = (short)pos.x;
            curr->posA[1] = (short)pos.y;
            curr->posA[2] = (short)pos.z;

            vector3Sub(&effect->particles[i].position[half], &effect->particles[i].widthOffset, &pos);
            curr->posB[0] = (short)pos.x;
            curr->posB[1] = (short)pos.y;
            curr->posB[2] = (short)pos.z;

            curr->stA[0] = 0;
            curr->stA[1] = half ? 16 << 5 : 0;

            curr->stB[0] = 16 << 5;
            curr->stB[1] = half ? 16 << 5 : 0;

            ++curr;
        }
    }

    data_cache_hit_writeback_invalidate(vertices, data_size);
    t3d_vert_load(vertices, 0, MAX_PARTICLE_COUNT * 4);

    rspq_block_run(triangles);
    t3d_tri_sync();
}

void spark_effect_destroy(struct SparkEffect* effect) {

}

#define MAX_ACTIVE_EFFECTS  8

static struct SparkEffect spark_effects[MAX_ACTIVE_EFFECTS];

void spark_effects_init() {
    rspq_block_begin();

    for (int i = 0; i < MAX_PARTICLE_COUNT; i += 1) {
        int offset = i * 4;

        t3d_tri_draw(offset, offset + 1, offset + 3);
        t3d_tri_draw(offset, offset + 3, offset + 2);
    }

    triangles = rspq_block_end();

    for (int i = 0; i < MAX_ACTIVE_EFFECTS; i += 1) {
        spark_effect_init(&spark_effects[i]);
    }
}

void spark_effects_spawn(struct Vector3* emit_from) {
    struct SparkEffect* next = NULL;

    for (int i = 0; i < MAX_ACTIVE_EFFECTS; i += 1) {
        struct SparkEffect* curr = &spark_effects[i];
        if (next == NULL || curr->time_left < next->time_left) {
            next = curr;
        }

        if (next->time_left == 0.0f) {
            break;
        }
    }

    if (next) {
        spark_effect_start(next, emit_from);
    }
}

void spark_effects_update(float delta_time) {
    for (int i = 0; i < MAX_ACTIVE_EFFECTS; i += 1) {
        spark_effect_update(&spark_effects[i], delta_time);
    }
}

void spark_effects_render(struct frame_malloc* fm) {
    rspq_block_run(rampage_assets_get()->spark_split.material);
    for (int i = 0; i < MAX_ACTIVE_EFFECTS; i += 1) {
        spark_effect_render(&spark_effects[i], fm);
    }
}

void spark_effects_destroy() {
    rspq_block_free(triangles);
}