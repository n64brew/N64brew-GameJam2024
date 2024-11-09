#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"

const MinigameDef minigame_def = {
    .gamename = "A Minimal Reproducible Example",
    .developername = "Josh Kautz",
    .description = "This is a minimal reproducible example for playing video in the libdragon gamejam minigame system.",
    .instructions = ""};

///////////////////////////////////////////////////////////
//                  Globals                              //
///////////////////////////////////////////////////////////

float fps;
mpeg2_t *mp2;
yuv_blitter_t yuvBlitter;

/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void minigame_init()
{
    display_init(
        (resolution_t){288, 209, INTERLACE_OFF},
        DEPTH_32_BPP, // 32-bit display mode is mandatory for video playback.
        8,
        GAMMA_NONE,
        FILTERS_DISABLED // FILTERS_DISABLED disables all VI post-processing to achieve the sharpest possible image. If you'd like to soften the image a little bit, switch to FITLERS_RESAMPLE.
    );

    // Initialize the YUV conversion library.
    yuv_init();

    // Check if the video is available in the filesystem.
    FILE *f = fopen("rom:/mre/video.m1v", "rb");
    assertf(f, "Video not found!\n");
    fclose(f);

    // Open the video using the mpeg2 module and create a YUV blitter to draw it
    mp2 = mpeg2_open("rom:/mre/video.m1v");
    yuvBlitter = yuv_blitter_new_fmv(
        // Resolution of the video we expect to play.
        // Video needs to have a width divisible by 32 and a height divisible by 16.
        // Here we have a video resolution of 288x160 which is a nice, valid resolution
        // that leaves a margin of 32 pixels on the side - great for making sure
        // CRT TVs with overscan still display the entire frame of your video.
        // The resolution is not an exact 16:9 ratio (16:8.88) but it's close enough that
        // most people won't notice. The lower resolution can also help with performance.
        mpeg2_get_width(mp2),
        mpeg2_get_height(mp2),
        display_get_width(),
        display_get_height(),
        // Override default FMV parms to not zoom the video.
        // This will leave our desired CRT TV-friendly margin around the video.
        &(yuv_fmv_parms_t){.zoom = YUV_ZOOM_NONE} // Don't zoom the video.
    );

    // Engage the fps limiter to ensure proper video pacing.
    fps = mpeg2_get_framerate(mp2);
    display_set_fps_limit(fps);
}

/*==============================
    minigame_fixedloop
    Code that is called every loop, at a fixed delta time.
    Use this function for stuff where a fixed delta time is
    important, like physics.
    @param  The fixed delta time for this tick
==============================*/
void minigame_fixedloop(float deltatime)
{
}

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/
void minigame_loop(float deltatime)
{
    if (!mpeg2_next_frame(mp2))
    {
        minigame_end();
        return;
    }

    rdpq_attach(display_get(), NULL);

    yuv_frame_t frame = mpeg2_get_frame(mp2);
    yuv_blitter_run(&yuvBlitter, &frame);

    rdpq_detach_show();
}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup()
{
    // Close the video track and free the allocated memory.
    mpeg2_close(mp2);
    yuv_blitter_free(&yuvBlitter);
    rspq_wait();
    display_close();
}