#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "strings.h" // Include the header file
#include "ascii.h"   // Include the new header file

#include <stdio.h>
#include <unistd.h>

const MinigameDef minigame_def = {
    .gamename = "A Mallard 64",
    .developername = "Josh Kautz",
    .description = "",
    .instructions = "",
};

// Number of frame back buffers we reserve.
// These buffers are used to render the video ahead of time.
// More buffers help ensure smooth video playback at the cost of more memory.
#define NUM_DISPLAY 28

// Maximum target audio frequency.
//
// Needs to be 48 kHz if Opus audio compression is used.
// In this example, we are using VADPCM audio compression
// which means we can use the real frequency of the audio track.
#define AUDIO_HZ 32000.0f

// Target screen resolution that we render at.
// Choosing a resolution above 240p (interlaced) can't be recommended for video playback.
#define SCREEN_WIDTH 288
#define SCREEN_HEIGHT 208
const resolution_t RESOLUTION_288x208 = {SCREEN_WIDTH, SCREEN_HEIGHT, INTERLACE_OFF};

float fps;
wav64_t audio_track;
mpeg2_t *video_track;
yuv_blitter_t yuv;
yuv_fmv_parms_t yuv_fmv_params = {
    .zoom = YUV_ZOOM_NONE};

float start_held_time = 0.0f;
bool start_held = false;

bool paused = false;

rdpq_font_t *font;

/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void minigame_init()
{
    ascii_art();

    ///////////////////////////////////////////////////////////
    //                  Set up Display                       //
    ///////////////////////////////////////////////////////////

    display_init(
        RESOLUTION_288x208,
        // 32-bit display mode is mandatory for video playback.
        DEPTH_32_BPP,
        NUM_DISPLAY,
        GAMMA_NONE,
        // FILTERS_DISABLED disables all VI post-processing to achieve the sharpest
        // possible image. If you'd like to soften the image a little bit, switch to
        // FITLERS_RESAMPLE.
        FILTERS_DISABLED);

    ///////////////////////////////////////////////////////////
    //                  Set up Font                          //
    ///////////////////////////////////////////////////////////

    font = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_VAR);
    rdpq_text_register_font(1, font);

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
    video_track = mpeg2_open("rom:/mallard/video.m1v");
    yuv = yuv_blitter_new_fmv(
        // Resolution of the video we expect to play.
        // Video needs to have a width divisible by 32 and a height divisible by 16.
        //
        // Here we have a video resolution of 288x160 which is a nice, valid resolution
        // that leaves a margin of 32 pixels on the side - great for making sure
        // CRT TVs with overscan still display the entire frame of your video.
        // The resolution is not an exact 16:9 ratio (16:8.88) but it's close enough that
        // most people won't notice. The lower resolution can also help with performance.
        mpeg2_get_width(video_track),
        mpeg2_get_height(video_track),
        // Set blitter's output area to our entire display
        display_get_width(),
        display_get_height(),
        // Override default FMV parms to not zoom the video.
        // This will leave our desired CRT TV-friendly margin around the video.
        &yuv_fmv_params);

    // Engage the fps limiter to ensure proper video pacing.
    fps = mpeg2_get_framerate(video_track);
    display_set_fps_limit(fps);

    ///////////////////////////////////////////////////////////
    //                  Set up Audio                         //
    ///////////////////////////////////////////////////////////

    // Open the audio track and start playing it in channel 0.
    wav64_open(&audio_track, "rom:/mallard/video.wav64");
    wav64_play(&audio_track, 0);
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
    rdpq_attach(display_get(), NULL);

    //////////////////////////////////////////////////////////////
    //                  Pause or Resume Video                   //
    //////////////////////////////////////////////////////////////

    if (!paused)
    {
        if (!mpeg2_next_frame(video_track))
        {
            minigame_end();
        }

        // Get the next video frame and feed it into our previously set up blitter.
        yuv_frame_t frame = mpeg2_get_frame(video_track);
        yuv_blitter_run(&yuv, &frame);
    }
    else
    {
        // TODO: Pause the video. Right now it just loops through a few frmes that it has (presumably) buffered.
    }

    ///////////////////////////////////////////////////////////
    //                  Render UI - Pause                    //
    ///////////////////////////////////////////////////////////

    rdpq_set_mode_standard();
    if (paused)
        rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, 30, 30, "Paused");

    ///////////////////////////////////////////////////////////
    //                  Render UI - Quitting                 //
    ///////////////////////////////////////////////////////////

    if (start_held)
    {
        rdpq_text_printf(NULL, FONT_BUILTIN_DEBUG_MONO, SCREEN_WIDTH - 100, 30, "Quitting in %.2f", 3.0f - start_held_time);

        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_set_prim_color(color_from_packed32(0xFFFFFFFF));
        rdpq_fill_rectangle(SCREEN_WIDTH - 100, 35, SCREEN_WIDTH - 30, 37);

        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_set_prim_color(color_from_packed32(0xFFFFFFFF));
        rdpq_fill_rectangle(SCREEN_WIDTH - 100, 43, SCREEN_WIDTH - 30, 45);

        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_set_prim_color(color_from_packed32(0xFFFFFFFF));
        rdpq_fill_rectangle(SCREEN_WIDTH - 100, 35, SCREEN_WIDTH - 128, 45);

        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_set_prim_color(color_from_packed32(0xFFFFFFFF));
        rdpq_fill_rectangle(SCREEN_WIDTH - 32, 35, SCREEN_WIDTH - 30, 45);

        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_set_prim_color(color_from_packed32(0xFFFFFFFF));
        int width = (int)(50 * (start_held_time / 3.0f));
        rdpq_fill_rectangle(SCREEN_WIDTH - 100, 35, SCREEN_WIDTH - 100 + width, 45);
    }

    //////////////////////////////////////////////////////////////
    //                  Pause or Resume Audio                   //
    //////////////////////////////////////////////////////////////

    // TODO: Pause the audio. Or stop it and get it to start at the same spot.

    ////////////////////////////////////////////////////////////
    //                  Handle Joypad Input                   //
    ////////////////////////////////////////////////////////////

    for (size_t i = 0; i < core_get_playercount(); i++)
    {
        joypad_port_t controllerPort = core_get_playercontroller(i);
        joypad_buttons_t pressed = joypad_get_buttons_pressed(controllerPort);
        joypad_buttons_t held = joypad_get_buttons_held(controllerPort);

        if (!joypad_is_connected(controllerPort))
        {
            continue;
        }

        // Exit the game.
        if (held.start)
        {
            if (!start_held)
            {
                // Record the time when the button is first pressed
                start_held = true;
            }
            else
            {
                start_held_time += deltatime;
                fprintf(stderr, "Controller %u has held start for %f\n", controllerPort, start_held_time);

                // If the button is held for more than 3 seconds, end the minigame
                if (start_held_time > 3.0f)
                {
                    minigame_end();
                }
            }
        }
        else
        {
            start_held = false;
            start_held_time = 0;
        }

        // Pause the video.
        if (pressed.a)
        {
            paused = !paused;
            fprintf(stderr, "Player pressed A. Video is now %s.\n", paused ? "paused" : "unpaused");
        }
    }

    rdpq_detach_show();
}

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup()
{
    wav64_close(&audio_track);

    mpeg2_close(video_track);

    display_close();

    rdpq_text_unregister_font(1);
    rdpq_font_free(font);

    fprintf(stderr, "End Cleanup\n");
}