/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once

#include "../utils/math.h"
#include <array>
#include <t3d/t3dmath.h>
#include <unordered_map>

struct SfxConf {
  float volume{1.0};
  uint8_t loop{0};
  uint8_t is2D{0};
  uint8_t variation{0};
};

class AudioManager {
  private:
    struct SFXInstance {
      wav64_t wave{};
      uint8_t *sampleDataStart{};
      uint8_t *sampleDataCurr{};
      uint8_t channel{};
      uint8_t bps{};
    };

    struct SFX {
      wav64_t source{};
      uint8_t *sampleData{nullptr};
      std::array<SFXInstance, 4> instances{};
    };

    std::unordered_map<uint64_t, SFX> sfxMap;
    wav64_t bgm{};
    wav64_t infoSFXStart{};
    wav64_t infoSFXWin{};
    T3DVec3 currCamPos{0.0f, 0.0f, 0.0f};
    T3DVec3 listenerDir{0.0f, 0.0f, -1.0f};

    float volBGM{0.7f};
    float volSFX{0.9f};
    Math::Timer bgmVolume{};

    void setVolume3D(int channel, const T3DVec3 &soundPos, float baseVolume = 1.0f);
    static void waveformRead(void *ctx, samplebuffer_t *sbuf, int wpos, int wlen, bool seeking);

  public:
    uint64_t ticks{0};

    AudioManager();
    ~AudioManager();

    void update(const T3DVec3 &camPos, const T3DVec3 &camTarget, float deltaTime);

    void playBGM(uint64_t name);
    void stopBGM();
    void setBGMVolume(float vol);

    uint32_t playSFX(uint64_t name, const T3DVec3 &pos, SfxConf conf = {});

    uint32_t playSFX(uint64_t name, SfxConf conf = {}) {
      conf.is2D = 1;
      return playSFX(name, T3DVec3{}, conf);
    }

    void playInfoSFX(uint64_t name);

    uint32_t getActiveChannelMask();
};