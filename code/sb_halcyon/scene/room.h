/*
 * This file includes code from Tiny3D.
 * Tiny3D is licensed under the MIT License.
 *
 * Original code by Max Bebök
 * Adapted by s4ys
 * November 2024
 *
 * Description of changes or adaptations made:
 * - water like wobble
 *
 *
 * Original source: https://github.com/HailToDodongo/tiny3d/tree/main/examples/04_dynamic
 */

#ifndef ROOM_H
#define ROOM_H

#define DEATH_PLANE_HEIGHT -50.0f
#define LOWER_LIMIT_HEIGHT -200.0f

void move_cloud(Scenery *scenery)
{

    scenery[0].transform_offset += 0.008f;

    // returns the global vertex buffer for a model.
    // If you have multiple models and want to only update one, you have to manually iterate over the objects.
    // see the implementation of t3d_model_draw_custom in that case.
    T3DVertPacked *verts = t3d_model_get_vertices(scenery[0].model);
    float globalHeight = fm_sinf(scenery[0].transform_offset * 2.5f);

    for (uint16_t i = 0; i < scenery[0].model->totalVertCount; ++i)
    {
        // To better handle the interleaved vertex format,
        // t3d provides a few helper functions to access attributes
        int16_t *pos = t3d_vertbuffer_get_pos(verts, i);

        // water-like wobble effect
        float height = fm_sinf(
            scenery[0].transform_offset * 4.5f + pos[0] * 30.1f + pos[1] * 20.1f);
        pos[2] = 4.0f * height + globalHeight;
    }

    // Don't forget to flush the cache again! (or use an uncached buffer in the first place)
    data_cache_hit_writeback(verts, sizeof(T3DVertPacked) * scenery[0].model->totalVertCount / 2);
}

void room_draw(Scenery *scenery)
{

    rdpq_mode_zbuf(false, true);
    t3d_model_draw(scenery[0].model);
    rdpq_mode_zbuf(true, true);
}

#endif