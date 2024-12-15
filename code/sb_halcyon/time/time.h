#ifndef TIME_H
#define TIME_H

// structures

typedef struct
{

    uint32_t frame_counter;
    float frame_time_s;
    float fixed_time_s;
    float frame_rate;
    double subtick;

} TimeData;

// functions prototypes

void time_init(TimeData *time);
void time_setData(TimeData *time, bool paused);

// functions implementations

/* sets time data values to zero */
void time_init(TimeData *time)
{

    time->frame_counter = 0;
    time->frame_rate = 0.0f;
    time->subtick = 1;

    time->frame_time_s = 0.0f;
    time->fixed_time_s = 0.04f;
}

/* sets timing data */
void time_setData(TimeData *time, bool paused)
{
    // Update timing values
    if (!paused)
        time->frame_counter++;
    time->frame_rate = display_get_fps();
    time->subtick = (core_get_subtick() == 0) ? 1 : core_get_subtick();
    time->frame_time_s = display_get_delta_time();
    time->fixed_time_s = time->frame_time_s * (float)time->subtick;
}

#endif