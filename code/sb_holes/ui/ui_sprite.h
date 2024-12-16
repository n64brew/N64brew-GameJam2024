#ifndef UI_SPRITE_H
#define UI_SPRITE_H

#include "ui_file.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // Constant dimensions for sprites
    const int spriteWidth = 16, spriteHeight = 16;

    // Static pointers to sprites representing different controller elements
    static sprite_t *sprite_controlStick;
    static sprite_t *sprite_dPadTriggers;
    static sprite_t *sprite_cButtons0;
    static sprite_t *sprite_cButtons1;
    static sprite_t *sprite_faceButtons0;
    static sprite_t *sprite_faceButtons1;

    // Static pointers to sprites for panels
    static sprite_t *sprite_gloss;
    static sprite_t *sprite_tessalate;
    static sprite_t *sprite_clouds;

    // Static pointers to sprites for logos
    static sprite_t *sprite_libdragon;
    static sprite_t *sprite_mixamo;
    static sprite_t *sprite_t3d;
    static sprite_t *sprite_strawberryTop;
    static sprite_t *sprite_strawberry0;
    static sprite_t *sprite_strawberry1;

    // Surfaces for rendering UI elements
    surface_t surf_UIpanels;
    surface_t surf_UIsprites;

    /* Declarations */

    void ui_spriteLoad(void);
    void ui_syncSprite(int color);
    void ui_spriteDraw(rdpq_tile_t tile, sprite_t *sprite, int idx, int x, int y);
    void ui_spriteDrawPanel(rdpq_tile_t tile, sprite_t *sprite, int color, int x0, int y0, int x1, int y1, int s, int t, int s1, int t1);
    void ui_spriteDrawDynamic(rdpq_tile_t tile, sprite_t *sprite, color_t color, int x0, int y0, int x1, int y1, int s, int t, int s1, int t1);
    void ui_spriteCleanup(void);

    /* Definitions */

    // Loads and assigns sprites to their corresponding pointers based on file paths set by ui_fileSprites.
    void ui_spriteLoad(void)
    {
        ui_fileSprites();
        ui_fileLogos();

        // Load IA format sprites (grayscale with alpha for UI overlays).
        sprite_gloss = sprite_load(uiSpritePanelFileName[0]);
        sprite_tessalate = sprite_load(uiSpritePanelFileName[1]);
        sprite_clouds = sprite_load(uiSpritePanelFileName[2]);

        sprite_controlStick = sprite_load(uiSpriteButtonFileName[0]);
        sprite_dPadTriggers = sprite_load(uiSpriteButtonFileName[1]);
        sprite_libdragon = sprite_load(uiSpriteLogoFileName[4]);
        sprite_mixamo = sprite_load(uiSpriteLogoFileName[5]);
        sprite_t3d = sprite_load(uiSpriteLogoFileName[3]);

        // Load RGBA32 format sprites (full color with transparency for UI buttons).
        sprite_cButtons0 = sprite_load(uiSpriteButtonFileName[2]);
        sprite_cButtons1 = sprite_load(uiSpriteButtonFileName[3]);
        sprite_faceButtons0 = sprite_load(uiSpriteButtonFileName[4]);
        sprite_faceButtons1 = sprite_load(uiSpriteButtonFileName[5]);
        sprite_strawberry0 = sprite_load(uiSpriteLogoFileName[0]);
        sprite_strawberry1 = sprite_load(uiSpriteLogoFileName[1]);
        sprite_strawberryTop = sprite_load(uiSpriteLogoFileName[2]);
    }

    // Optional RDPQ sync and set for sprites. Similar to ui_syncText, but sets the combiner for textures and allows for primitive color to added.
    void ui_syncSprite(int color)
    {
        rdpq_sync_pipe();
        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_set_prim_color(ui_color(color));
        rdpq_sync_tile();
    }

    // Draws a simple 16x16 sprite.
    void ui_spriteDraw(rdpq_tile_t tile, sprite_t *sprite, int idx, int x, int y)
    {
        int s = 0, t = 0;
        int idxCopy = idx;

        if (idx > 4)
        {
            idx = idx % 4;
            s = spriteWidth * idx;
        }
        else
        {
            s = spriteWidth * idx;
        }

        t = (idxCopy / 4) * spriteHeight;

        ui_syncSprite(WHITE);

        surf_UIsprites = sprite_get_pixels(sprite);

        rdpq_tex_upload_sub(tile, &surf_UIsprites, NULL, s, t, s + spriteWidth, t + spriteHeight);
        rdpq_texture_rectangle(tile, x, y, x + spriteWidth, y + spriteHeight, s, t);
    }

    // Draws a scalable sprite with predefined primitive color by index.
    void ui_spriteDrawPanel(rdpq_tile_t tile, sprite_t *sprite, int colorIdx, int x0, int y0, int x1, int y1, int s, int t, int s1, int t1)
    {

        ui_syncSprite(colorIdx);

        surf_UIpanels = sprite_get_pixels(sprite);

        rdpq_tex_upload(tile, &surf_UIpanels, NULL);
        rdpq_texture_rectangle_scaled(tile, x0, y0, x1, y1, s, t, s1, t1);
    }

    // Draws a scalable sprite with added primitive color.
    void ui_spriteDrawDynamic(rdpq_tile_t tile, sprite_t *sprite, color_t color, int x0, int y0, int x1, int y1, int s, int t, int s1, int t1)
    {

        rdpq_sync_pipe();
        rdpq_set_mode_standard();
        rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_set_prim_color(color);
        rdpq_sync_tile();

        surf_UIpanels = sprite_get_pixels(sprite);

        rdpq_tex_upload(tile, &surf_UIpanels, NULL);
        rdpq_texture_rectangle_scaled(tile, x0, y0, x1, y1, s, t, s1, t1);
    }

    // Frees static pointers to sprites.
    void ui_spriteCleanup(void)
    {
        sprite_free(sprite_controlStick);
        sprite_free(sprite_dPadTriggers);
        sprite_free(sprite_cButtons0);
        sprite_free(sprite_cButtons1);
        sprite_free(sprite_faceButtons0);
        sprite_free(sprite_faceButtons1);
        sprite_free(sprite_gloss);
        sprite_free(sprite_clouds);
        sprite_free(sprite_tessalate);
        sprite_free(sprite_libdragon);
        sprite_free(sprite_mixamo);
        sprite_free(sprite_t3d);
        sprite_free(sprite_strawberryTop);
        sprite_free(sprite_strawberry0);
        sprite_free(sprite_strawberry1);
        surface_free(&surf_UIpanels);
        surface_free(&surf_UIsprites);
    }

#ifdef __cplusplus
}
#endif

#endif // UI_SPRITE_H