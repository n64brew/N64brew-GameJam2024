/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "cutscene.h"

void Cutscene::update(float deltaTime) {
  time -= deltaTime;
  if(idx >= events.size())return;

  if(idx > 0 && !events[idx-1].oneTime) {
    events[idx-1].callback();
  }

  if(time <= 0.0f) {
    events[idx].callback();
    time = events[idx].duration;
    ++idx;
  }
}

void Cutscene::skipToEnd() {
  for(; idx < events.size(); ++idx) {
    events[idx].callback();
  }
}
