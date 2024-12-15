#include <unistd.h>
#include <time.h>
#include <libdragon.h>
#include "gamestatus.h"
#include "gfx.h"

gamestatus_t gamestatus;

void timesys_init(){
    gamestatus.currenttime = 0.0f;
    gamestatus.realtime = TICKS_TO_MS(timer_ticks()) / 1000.0f;

    gamestatus.deltatime = 0.0f;
    gamestatus.deltarealtime = 0.0f;

    gamestatus.gamespeed = 1.0f;
    gamestatus.paused = false;

    gamestatus.state = GAMESTATE_GETREADY;
    gamestatus.statetime = 0;

    gamestatus.fixedframerate = 30;
    gamestatus.fixedtime = 0.0f;
    gamestatus.fixeddeltatime = 0.0f;
}

void timesys_update(){
    double last = gamestatus.realtime;
    double current = TICKS_TO_MS(timer_ticks()) / 1000.0f;
    double deltareal = current - last;
    double delta = deltareal * gamestatus.gamespeed;
    gamestatus.statetime = fclampr(gamestatus.statetime, 0.0f, INFINITY);

    if(!gamestatus.paused){
        double lastgametime = gamestatus.currenttime;
        gamestatus.currenttime += delta;
        gamestatus.deltatime = gamestatus.currenttime - lastgametime;
    } else 
        gamestatus.deltatime = 0.0f;
          
    if(gamestatus.state != GAMESTATE_PAUSED)
        gamestatus.statetime -= delta;
    
    gamestatus.realtime = current;
    gamestatus.deltarealtime = deltareal;
}

bool timesys_update_fixed(){
    gamestatus.fixeddeltatime = 0.0f;
    
    if(gamestatus.paused) return false;

    double fixed = (1.0f / gamestatus.fixedframerate);
    double nexttime = gamestatus.fixedtime + fixed;

    if(nexttime > CURRENT_TIME) return false;
    
    gamestatus.fixedtime = nexttime;
    gamestatus.fixeddeltatime = fixed;
    return true;
}

void timesys_close(){
    
}