#ifndef __WRAPPERS_H
#define __WRAPPERS_H

#include <functional>
#include <memory>

#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

class Display
{
    private:
    public:
        const surface_t* depthBuffer;
        Display() {
            display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE);
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
            assertf(font.get(), "Font is null");
            rdpq_text_register_font(id, font.get());
        };
        ~RDPQFont() {
            rdpq_text_unregister_font(id);
        };
};

class RDPQSurface
{
    private:
        surface_t s;
    public:
        RDPQSurface(tex_format_t format, uint16_t width, uint16_t height) {
            s = surface_alloc(format, width, height);
        };
        ~RDPQSurface() {
            surface_free(&s);
        };
        surface_t* get() {
            return &s;
        };
};


namespace U {
    using RSPQBlock = std::unique_ptr<rspq_block_t, decltype(&rspq_block_free)>;
    using T3DMat4FP = std::unique_ptr<T3DMat4FP, decltype(&free_uncached)>;
    using T3DSkeleton = std::unique_ptr<T3DSkeleton, decltype(&t3d_skeleton_destroy)>;
    using T3DAnim = std::unique_ptr<T3DAnim, decltype(&t3d_anim_destroy)>;
    using T3DModel = std::unique_ptr<T3DModel, decltype(&t3d_model_free)>;
    using Timer = std::unique_ptr<timer_link_t, decltype(&delete_timer)>;
    using Sprite = std::unique_ptr<sprite_t, decltype(&sprite_free)>;
    // Would be nice to use a vector, see https://stackoverflow.com/questions/11896960/custom-allocator-in-stdvector
    using TLUT = std::unique_ptr<uint16_t, decltype(&free_uncached)>;
}

#endif // __WRAPPERS_H