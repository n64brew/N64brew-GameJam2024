#include <libdragon.h>
#include "../../minigame.h"
#include "sequence3.h"
#include "input.h"

// Target screen resolution that we render at.
// Choosing a resolution above 240p (interlaced) can't be recommended for video playback.
#define SCREEN_WIDTH 288
#define SCREEN_HEIGHT 208

// Number of frame back buffers we reserve.
// These buffers are used to render the video ahead of time.
// More buffers help ensure smooth video playback at the cost of more memory.
#define NUM_BUFFERS 8

// Maximum target audio frequency.
//
// Needs to be 48 kHz if Opus audio compression is used.
// In this example, we are using VADPCM audio compression
// which means we can use the real frequency of the audio track.
#define AUDIO_HZ 32000.0f

///////////////////////////////////////////////////////////
//                  Globals                              //
///////////////////////////////////////////////////////////

float fps;
mpeg2_t *mp2;
yuv_blitter_t yuvBlitter;
yuv_frame_t sequence3_frame;

wav64_t audio_track;

bool sequence3_paused = false;
bool sequence3_rewind = false;
float sequence3_b_btn_held_duration = 0.0f;

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

    ///////////////////////////////////////////////////////////
    //                  Set up Audio                         //
    ///////////////////////////////////////////////////////////

    // Open the audio track and start playing it in channel 31.
    wav64_open(&audio_track, "rom:/mallard/video.wav64");
    mixer_ch_play(31, &audio_track.wave);

    sequence3_initialized = true;
}

void sequence3_cleanup()
{
    // Close the video track and free the allocated memory.
    mpeg2_close(mp2);
    yuv_blitter_free(&yuvBlitter);
    rspq_wait();
    display_close();

    // Close the audio track and free the allocated memory.
    wav64_close(&audio_track);

    // Reset the state.
    sequence3_b_btn_held_duration = 0.0f;
    sequence3_paused = false;
    sequence3_rewind = false;
    sequence3_initialized = false;
    sequence3_video_finished = false;

    // End the sequence.
    sequence_3_video = false;
    sequence_4_BLANK = true;
}

void sequence_3(float deltatime)
{

    process_controller(deltatime);

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

    if (sequence3_rewind)
    {
        // Rewind Audio.
        mixer_ch_set_pos(31, 0.0f);

        // Rewind Video.
        mpeg2_rewind(mp2);
        mpeg2_next_frame(mp2);
        sequence3_frame = mpeg2_get_frame(mp2);
        sequence3_rewind = false;
    }

    if (!sequence3_paused)
    {
        mixer_throttle(AUDIO_HZ / fps); // Audio

        if (!mpeg2_next_frame(mp2))
        {
            sequence3_video_finished = true;
            return;
        }

        mixer_try_play(); // Audio

        sequence3_frame = mpeg2_get_frame(mp2);
        rdpq_attach(display_get(), NULL);
        yuv_blitter_run(&yuvBlitter, &sequence3_frame);
        rdpq_detach_show();

        mixer_try_play(); // Audio

        rspq_wait(); // Audio
    }
    else
    {
        rdpq_attach(display_get(), NULL);
        yuv_blitter_run(&yuvBlitter, &sequence3_frame);
        rdpq_detach_show();
    }
}