#ifndef __SPARK_EFFECT_H__
#define __SPARK_EFFECT_H__

#include "./math/vector3.h"
#include <libdragon.h>
#include "./frame_malloc.h"

#define MAX_PARTICLE_COUNT  8

struct SparkParticle {
    struct Vector3 position[2];
    struct Vector3 velocity;
    struct Vector3 widthOffset;
};

struct SparkEffect {
    struct SparkParticle particles[MAX_PARTICLE_COUNT];
    float time_left;
};

void spark_effect_init(struct SparkEffect* effect);
void spark_effect_start(struct SparkEffect* effect, struct Vector3* emit_from);

void spark_effect_update(struct SparkEffect* effect, float delta_time);
void spark_effect_render(struct SparkEffect* effect, struct frame_malloc* fm);

void spark_effect_destroy(struct SparkEffect* effect);

void spark_effects_init();
void spark_effects_spawn(struct Vector3* emit_from);
void spark_effects_update(float delta_time);
void spark_effects_render(struct frame_malloc* fm);
void spark_effects_destroy();

#endif