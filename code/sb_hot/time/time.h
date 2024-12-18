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
void time_setData(TimeData *time);

// functions implementations

/* sets time data values to zero */
void time_init(TimeData *time)
{
    time->frame_counter = 0;
    time->frame_time_s = 0.0f;
    time->fixed_time_s = 0.04f; // Hardcoded
    time->frame_rate = 0.0f;
    time->subtick = 0.0;
}

/* sets timing data */
void time_setData(TimeData *time)
{
    // Update timing values
    time->frame_rate = display_get_fps(); // Retrieve current frames per second
    time->subtick = core_get_subtick();   // Retrieve subtick of the frame
    time->frame_counter++;
}

#endif