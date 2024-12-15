/*
 * This file includes code from Tiny3D.
 * Tiny3D is licensed under the MIT License.
 *
 * Original code by Max Bebök
 * Adapted by s4ys
 * November 2024
 *
 * Description of changes or adaptations made:
 * - Generate positions randomly within an AABB
 * - Utilize `gradient_fire` as `gradient_alpha` to add random alpha values to white
 *
 *
 * Original source: https://github.com/HailToDodongo/tiny3d/tree/main/examples/18_particles
 */

#ifndef PARTICLES_H
#define PARTICLES_H

typedef struct
{
    uint32_t count;
    uint32_t bufSize;
    TPXParticle *buf;
    T3DMat4FP *mat;

} Particles;

Particles cloudMist;

void ptx_init(Particles *ptx)
{
    tpx_init((TPXInitParams){});
    ptx->count = 100;
    ptx->bufSize = sizeof(TPXParticle) * (ptx->count + 2);
    ptx->buf = malloc_uncached(ptx->bufSize);
    ptx->mat = malloc_uncached(sizeof(T3DMat4FP));
}

// Alpha gradient
void gradient_alpha(uint8_t *color, float t)
{
    t = fminf(1.0f, fmaxf(0.0f, t));
    t = 0.1f - t;
    t *= t;

    color[0] = (uint8_t)(240);
    color[1] = (uint8_t)(240);
    color[2] = (uint8_t)(240);
    color[3] = (uint8_t)(200 * (1.0f - (t - 0.5f) / 0.5f));
}

void ptx_randomPos(T3DViewport *vp, Particles *ptx)
{
    for (int i = 0; i < ptx->count; i++)
    {
        int p = i / 2;
        int8_t *ptxPos = (i % 2 == 0) ? ptx->buf[p].posA : ptx->buf[p].posB;

        // Assign random sizes
        ptx->buf[p].sizeA = 20 + (rand() % 10);
        ptx->buf[p].sizeB = 20 + (rand() % 10);

        // Random positions within the bounding box
        float min = -127.9f;
        float max = 126.9f;
        T3DVec3 randomPos;
        randomPos.v[0] = min + ((float)rand() / max) * (max - min);
        randomPos.v[1] = min;
        randomPos.v[2] = min + ((float)rand() / max) * (max - min);

        // Calculate from view space
        T3DVec3 screenPos;
        t3d_viewport_calc_viewspace_pos(vp, &screenPos, &randomPos);

        float t = (float)i / ptx->count;   // Vary by particle index
        screenPos.v[1] += t * (max - min); // Move upward

        // Clamp final values to fit within int8_t range
        ptxPos[0] = (int8_t)screenPos.v[0];
        ptxPos[1] = (int8_t)screenPos.v[1];
        ptxPos[2] = (int8_t)screenPos.v[2];

        gradient_alpha(ptx->buf[p].colorA, (ptxPos[0] + 127) * 0.0012f);
        gradient_alpha(ptx->buf[p].colorB, (ptxPos[0] + 127) * 0.0012f);
    }
}

void ptx_draw(T3DViewport *vp, Platform *platform, Particles *ptx)
{

    static int frameCounter = 0;
    const int updateInterval = 3;

    // Prepare the RDPQ
    rdpq_sync_pipe();
    rdpq_sync_tile();
    rdpq_set_mode_standard();
    rdpq_mode_zbuf(true, true);
    rdpq_mode_zoverride(true, 0, 0);
    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);

    if (platform->position.z <= 50.0f && platform->position.z >= -50.0f)
    {
        if (frameCounter % updateInterval == 0)
        {
            ptx_randomPos(vp, ptx);
        }
        frameCounter++;

        tpx_state_from_t3d();

        tpx_matrix_push_pos(1);
        tpx_matrix_set(ptx->mat, true);
        tpx_state_set_scale(1, 1);
        t3d_mat4fp_from_srt_euler(
            ptx->mat,
            (float[3]){7, 4, 7},
            (float[3]){0, 0, 0},
            (float[3]){0, 250, 0});
        tpx_particle_draw(ptx->buf, ptx->count);
        tpx_matrix_pop(1);
    }
}

void ptx_cleanup(Particles *ptx)
{
    free_uncached(ptx->mat);
    free_uncached(ptx->buf);
    tpx_destroy();
}

#endif // PARTICLES_H