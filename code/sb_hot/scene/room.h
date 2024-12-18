/*
 * This file includes code from Tiny3D.
 * Tiny3D is licensed under the MIT License.
 *
 * Original code by Max Bebök
 * Adapted by s4ys
 * November 2024
 *
 * Description of changes or adaptations made:
 * - Modify water like wobble to use a sum of sines instead a single sine wave
 *
 *
 * Original source: https://github.com/HailToDodongo/tiny3d/tree/main/examples/04_dynamic
 */

#ifndef ROOM_H
#define ROOM_H

#define DEATH_PLANE_HEIGHT -50.0f
#define LOWER_LIMIT_HEIGHT -200.0f

#define NUM_HARMONICS 4

typedef struct
{
    float amplitude[NUM_HARMONICS];
    float frequency[NUM_HARMONICS];
    float phase[NUM_HARMONICS];
} HarmonicData;

// Precomputed harmonic data for the wobble effect
static const HarmonicData wobbleData = {
    .amplitude = {10.0f, 5.0f, 3.0f, 1.0f},
    .frequency = {4.5f, 9.0f, 13.5f, 18.0f},
    .phase = {0.0f, 0.5f, 1.0f, 1.5f}};

// Hook/callback to modify tile settings set by t3d_model_draw
void tile_scroll(void *userData, rdpq_texparms_t *tileParams, rdpq_tile_t tile)
{
    float offset = *(float *)userData;

    if (tile == TILE0)
    {
        tileParams->s.translate = offset * 0.5f;
        tileParams->t.translate = offset * 0.8f;

        tileParams->s.translate = fm_fmodf(tileParams->s.translate, 32.0f);
        tileParams->t.translate = fm_fmodf(tileParams->t.translate, 32.0f);
    }
}

void move_lava(Scenery *scenery)
{

    scenery[0].transform_offset += 0.008f;
    scenery[0].tile_offset += 0.1f;

    // returns the global vertex buffer for a model.
    // If you have multiple models and want to only update one, you have to manually iterate over the objects.
    // see the implementation of t3d_model_draw_custom in that case.
    T3DVertPacked *verts = t3d_model_get_vertices(scenery[0].model);
    float globalHeight = 0;

    // Calculate globalHeight as the sum of the harmonics
    for (int i = 0; i < NUM_HARMONICS; ++i)
    {
        globalHeight += wobbleData.amplitude[i] *
                        fm_sinf(scenery[0].transform_offset * wobbleData.frequency[i] + wobbleData.phase[i]);
    }

    for (uint16_t i = 0; i < scenery[0].model->totalVertCount; ++i)
    {
        int16_t *pos = t3d_vertbuffer_get_pos(verts, i);

        // Water-like wobble effect using a sum of sines
        float height = 0.0f;
        for (int j = 0; j < NUM_HARMONICS; ++j)
        {
            height += wobbleData.amplitude[j] *
                      fm_sinf(
                          scenery[0].transform_offset * wobbleData.frequency[j] +
                          wobbleData.phase[j] + pos[0] * (30.1f + j) + pos[1] * (20.1f + j));
        }

        pos[2] = height + globalHeight;

        // Adjust color more subtly based on height
        float colorVariation = height * 0.02f; // Reduced scaling factor
        float baseIntensity = 0.75f;           // Higher base intensity for consistent lighting
        float color = baseIntensity + colorVariation;

        uint8_t *rgba = t3d_vertbuffer_get_rgba(verts, i);
        rgba[0] = fminf(color, 0.9f) * 255; // Clamp to avoid overflow
        rgba[1] = fminf(color, 0.9f) * 200;
        rgba[2] = fminf(color, 0.9f) * 200;
        rgba[3] = 0xFF;
    }

    // Don't forget to flush the cache again! (or use an uncached buffer in the first place)
    data_cache_hit_writeback(verts, sizeof(T3DVertPacked) * scenery[0].model->totalVertCount / 2);
}

void room_draw(Scenery *scenery)
{

    rdpq_mode_zbuf(false, true);
    t3d_model_draw_custom(scenery[0].model, (T3DModelDrawConf){
                                                .userData = &scenery[0].tile_offset,
                                                .tileCb = tile_scroll,
                                            });
    rdpq_mode_zbuf(true, true);
}

#endif