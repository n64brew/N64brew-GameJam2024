/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include <functional>
#include <vector>
#include <libdragon.h>

struct CutsceneEntry
{
  std::function<void(void)> callback{};
  float duration{};
  uint8_t oneTime{0};
};

class Cutscene
{
  private:
    std::vector<CutsceneEntry> events;
    float time{0.0f};
    uint32_t idx{0};

  public:
    Cutscene& event(std::function<void(void)> callback) {
      events.push_back({callback, 0, 1});
      return *this;
    }

    Cutscene& task(float duration, std::function<void(void)> callback) {
      events.push_back({callback, duration, 0});
      events.push_back({[]{}, duration, 1});
      return *this;
    }

    Cutscene& wait(float duration) {
      if(!events.empty() && events[events.size() - 1].duration == 0) {
        events[events.size() - 1].duration = duration;
        return *this;
      }
      events.push_back({[]{}, duration, 1});
      return *this;
    }

    void update(float deltaTime);

    void skipToEnd();

    [[nodiscard]] float getLocalTime() const { return time; }
};