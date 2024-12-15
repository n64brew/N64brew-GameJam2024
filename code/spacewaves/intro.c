#include <libdragon.h>

// Number of frame back buffers we reserve.
// These buffers are used to render the video ahead of time.
// More buffers help ensure smooth video playback at the cost of more memory.
#define NUM_DISPLAY 8

// Maximum target audio frequency.
//
// Needs to be 48 kHz if Opus audio compression is used.
// In this example, we are using VADPCM audio compression
// which means we can use the real frequency of the audio track.
#define AUDIO_HZ 32000.0f

void intro(void)
{
	yuv_init();

	// Open the movie using the mpeg2 module and create a YUV blitter to draw it.
	mpeg2_t* video_track = mpeg2_open("rom:/spacewaves/intro.m1v");

	int video_width = mpeg2_get_width(video_track);
	int video_height = mpeg2_get_height(video_track);

	// When playing back a video, there are essentially two options:
	// 1) Configure a fixed resolution (eg: 320x240), and then make
	//    the video fit it, with letterboxing if necessary. This is requires
	//    actually drawing / rescaling the video with RDP and filling the
	//    rest of the framebuffers with black.
	// 2) Configure a resolution which exactly matches the video resolution,
	//     and let VI perform the necessary centering / letterboxing.
	//
	// 2 is more efficient for full motion videos because no additional memory
	// is wasted in framebuffers to hold black pixels, so we go with it.

	display_init((resolution_t){
			// Initialize a framebuffer resolution which precisely matches the video
			.width = video_width, .height = video_height,
			.interlaced = INTERLACE_OFF,
			// Set the desired aspect ratio to that of the video. By default,
			// display_init would force 4:3 instead, which would be wrong here.
			// eg: if a video is 320x176, we want to display it as 16:9-ish.
			.aspect_ratio = (float)video_width / video_height,
			// Uncomment this line if you want to have some additional black
			// borders to fully display the video on real CRTs.
			// .overscan_margin = VI_CRT_MARGIN,
		},
		// 32-bit display mode is mandatory for video playback.
		DEPTH_32_BPP,
		NUM_DISPLAY, GAMMA_NONE,
		// Activate bilinear filtering while rescaling the video
		FILTERS_RESAMPLE
	);

	yuv_blitter_t yuv = yuv_blitter_new_fmv(
		// Resolution of the video we expect to play.
		// Video needs to have a width divisible by 32 and a height divisible by 16.
		video_width, video_height,
		// Set blitter's output area to our entire display. Given the above
		// initialization, this will actually match the video width/height, but
		// if we instead opted for a fixed resolution (eg: 320x240), it would be
		// the YUV blitter that would letterbox the video by adding black borders
		// where necessary.
		display_get_width(), display_get_height(),
		// You can further customize YUV options through this parameter structure
		// if necessary.
		&(yuv_fmv_parms_t) {}
	);

	// Engage the fps limiter to ensure proper video pacing.
	float fps = mpeg2_get_framerate(video_track) / 2;
	display_set_fps_limit(fps);


	int nframes = 0;

	while (1)
	{
		if (!mpeg2_next_frame(video_track))
		{
			break;
		}

		rdpq_attach(display_get(), NULL);

		// Get the next video frame and feed it into our previously set up blitter.
		yuv_frame_t frame = mpeg2_get_frame(video_track);
		yuv_blitter_run(&yuv, &frame);

		rdpq_detach_show();

		nframes++;


		rspq_wait();
	}
	yuv_blitter_free(&yuv);
    mpeg2_close(video_track);

    display_close();
    yuv_close();
}
