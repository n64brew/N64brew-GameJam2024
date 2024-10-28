#ifndef __GAME_H
#define __GAME_H

#include <libdragon.h>

#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wformat-signedness"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wformat-security"
#pragma GCC diagnostic ignored "-Wformat-overflow"
#pragma GCC diagnostic ignored "-Wformat-truncation"

// #include <t3d/t3dskeleton.h>
// #include <t3d/t3danim.h>
// #include <t3d/t3ddebug.h>

#pragma GCC diagnostic pop

#include "../../../core.h"
#include "../../../minigame.h"

#include "./player.hpp"


#include <functional>
#include <memory>

class Display
{
    private:
    public:
        const surface_t* depthBuffer;
        Display() {
            display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);
            depthBuffer = display_get_zbuf();
        };
        ~Display() {
            display_close();
        };
};

class T3D
{
    private:
    public:
        T3D() {
            t3d_init((T3DInitParams){});
        };
        ~T3D() {
            t3d_destroy();
        };
};

class RDPQFont
{
    private:
        int id;
    public:
        std::unique_ptr<rdpq_font_t, decltype(&rdpq_font_free)> font;
        RDPQFont(const char *name, int id):
            id(id),
            font({rdpq_font_load(name), rdpq_font_free}) 
        {
            rdpq_text_register_font(id, font.get());
        };
        ~RDPQFont() {
            rdpq_text_unregister_font(id);
        };
};


class Game
{
    private:
        PlayerController playerManager;
        Display display;
        T3D t3d;

        void timer_callback();
        T3DViewport viewport;
        RDPQFont font;
        std::unique_ptr<timer_link_t, decltype(&delete_timer)> timer;

        // Map
        std::unique_ptr<T3DMat4FP, decltype(&free_uncached)> mapMatFP;
        std::unique_ptr<rspq_block_t, decltype(&rspq_block_free)> dplMap;
        std::unique_ptr<T3DModel, decltype(&t3d_model_free)> modelMap;

        void setupMap(std::unique_ptr<T3DMat4FP, decltype(&free_uncached)> &mapMatFP);
        void renderMap();

    public:
        Game();
        ~Game();
        void setup();
        void update(float deltatime);
        void fixed_update(float deltatime);
};

#endif /* __GAME_H */

