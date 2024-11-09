#include <libdragon.h>
#include "../../minigame.h"
#include "sequence3.h"

// Target screen resolution that we render at.
// Choosing a resolution above 240p (interlaced) can't be recommended for video playback.
#define SCREEN_WIDTH 288
#define SCREEN_HEIGHT 208

// Number of frame back buffers we reserve.
// These buffers are used to render the video ahead of time.
// More buffers help ensure smooth video playback at the cost of more memory.
#define NUM_BUFFERS 8

///////////////////////////////////////////////////////////
//                  Globals                              //
///////////////////////////////////////////////////////////

float fps;
mpeg2_t *mp2;
yuv_blitter_t yuvBlitter;

bool sequence3_initialized = false;
bool sequence3_video_finished = false;

void sequence3_init()
{
    ///////////////////////////////////////////////////////////
    //                  Set up Display                       //
    ///////////////////////////////////////////////////////////

    display_init(
        (resolution_t){SCREEN_WIDTH, SCREEN_HEIGHT, INTERLACE_OFF},
        DEPTH_32_BPP, // 32-bit display mode is mandatory for video playback.
        NUM_BUFFERS,
        GAMMA_NONE,
        FILTERS_DISABLED // FILTERS_DISABLED disables all VI post-processing to achieve the sharpest possible image. If you'd like to soften the image a little bit, switch to FITLERS_RESAMPLE.
    );

    ///////////////////////////////////////////////////////////
    //                  Set up Video                         //
    ///////////////////////////////////////////////////////////

    // Initialize the YUV conversion library.
    yuv_init();

    // Check if the video is available in the filesystem.
    FILE *f = fopen("rom:/mallard/video.m1v", "rb");
    assertf(f, "Video not found!\n");
    fclose(f);

    // Open the video using the mpeg2 module and create a YUV blitter to draw it
    mp2 = mpeg2_open("rom:/mallard/video.m1v");
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

    sequence3_initialized = true;
}

void sequence3_cleanup()
{
    // Close the video track and free the allocated memory.
    mpeg2_close(mp2);
    yuv_blitter_free(&yuvBlitter);
    rspq_wait();
    display_close();

    // Reset the state.
    sequence3_initialized = false;
    sequence3_video_finished = false;

    // End the sequence.
    sequence_3_video = false;
    sequence_4_BLANK = true;
}

void sequence_3(float deltatime)
{
    if (sequence3_video_finished)
    {
        sequence3_cleanup();
        return;
    }

    if (!sequence3_initialized)
        sequence3_init();

    //////////////////////////////////////////////////////////////
    //                  Play Video                              //
    //////////////////////////////////////////////////////////////

    if (!mpeg2_next_frame(mp2))
    {
        // TODO: Use rewind + next_frame to replay the video. (Press Z)
        // mpeg2_rewind(mp2);
        // mpeg2_next_frame(mp2);

        sequence3_video_finished = true;
        return;
    }

    rdpq_attach(display_get(), NULL);

    // if (!paused)
    // {
    yuv_frame_t frame = mpeg2_get_frame(mp2);
    yuv_blitter_run(&yuvBlitter, &frame);
    // }
    // else
    // {
    //     // TODO: Pause the video. Right now it just loops through a few frmes that it has (presumably) buffered.
    // }

    rdpq_detach_show();
}