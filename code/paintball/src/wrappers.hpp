#ifndef __WRAPPERS_H
#define __WRAPPERS_H

#include <functional>
#include <memory>

#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3dmodel.h>

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
    public:
    class Skeleton
    {
        private:
            T3DSkeleton skel = {0};
        public:
            Skeleton(const T3DModel *model) {
                skel = t3d_skeleton_create(model);
            };

            // Everything is a copy, we allocate these once anyways
            Skeleton(Skeleton&& rhs) {
                skel = *rhs.get();
                rhs.skel = {0};
            };

            Skeleton& operator=(Skeleton&& rhs) {
                skel = *rhs.get();
                rhs.skel = {0};
                return *this;
            };

            ~Skeleton() {
                t3d_skeleton_destroy(&skel);
            };

            T3DSkeleton* get() {
                return &skel;
            };
    };

    class Anim
    {
        private:
            T3DAnim anim = {0};
        public:
            Anim(const T3DModel *model, const char *name) {
                anim = t3d_anim_create(model, name);
            };

            // Everything is a copy, we allocate these once anyways
            Anim(Anim&& rhs) {
                anim = *rhs.get();
                rhs.anim = {0};
            };

            Anim& operator=(Anim&& rhs) {
                anim = *rhs.get();
                rhs.anim = {0};
                return *this;
            };

            ~Anim() {
                t3d_anim_destroy(&anim);
            };
            T3DAnim* get() {
                return &anim;
            };
    };

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

class Wav64
{
    private:
        wav64_t wav;
    public:
        Wav64(const char *name) {
            wav64_open(&wav, name);
        };
        ~Wav64() {
            wav64_close(&wav);
        };
        wav64_t* get() {
            return &wav;
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
    using T3DVertPacked = std::unique_ptr<T3DVertPacked, decltype(&free_uncached)>;
}

#endif // __WRAPPERS_H