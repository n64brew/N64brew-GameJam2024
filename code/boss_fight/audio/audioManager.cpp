/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "audioManager.h"
#include "../main.h"
#include <libdragon.h>
#include <string>
#include <unistd.h>

namespace {
  constexpr int CHANNEL_COUNT = 32;

  constexpr int CHANNEL_BGM = 1;
  constexpr int CHANNEL_INFO = 3;
  constexpr int CHANNEL_SFX = 4;
  constexpr int CHANNEL_SFX_COUNT = 8;

  constexpr float BGM_FADE_TIME = 2.0f;
  uint32_t lastIdx{};

  int findFreeChannel() {
    lastIdx += 1; // Note: for some reason not cycling through sometimes causes SFX to not play
    for(int i=0; i<CHANNEL_SFX_COUNT; ++i) {
      int idx = CHANNEL_SFX + ((i + lastIdx) % CHANNEL_SFX_COUNT);
      if(!mixer_ch_playing(idx))return idx;
    }
    return -1;
  }

  uint32_t getWaveSize(wav64_t *wav) {
    return wav->wave.len * wav->wave.channels * (wav->wave.bits / 8);
  }

  void constructPath(char *path, uint64_t name, std::size_t pathLen) {
    const char* nameStr = reinterpret_cast<const char*>(&name);
    char *p = path + pathLen - 15;
    const char* nameStrEnd = nameStr + 8;
    while(*nameStr != '\0' && nameStr != nameStrEnd) {
      *(p++) = *(nameStr++);
    }
    *(p++) = '.'; *(p++) = 'w';
    *(p++) = 'a'; *(p++) = 'v';
    *(p++) = '6'; *(p++) = '4';
    *p = '\0';
  }
}

void AudioManager::waveformRead(void *ctx, samplebuffer_t *sbuf, int wpos, int wlen, bool seeking) {
  auto* inst = (SFXInstance*)ctx;
  if (seeking) {
    inst->sampleDataCurr = inst->sampleDataStart + (wpos << inst->bps);
  }

  uint8_t* ram_addr = (uint8_t*)samplebuffer_append(sbuf, wlen);
  int bytes = wlen << inst->bps;
  memcpy(ram_addr, inst->sampleDataCurr, bytes);
  inst->sampleDataCurr += bytes;
}

AudioManager::AudioManager() {
  lastIdx = CHANNEL_SFX;
  char path[]{"core/01234567.wav64\0"};
  constructPath(path, "Start"_u64, sizeof(path)-1);
  wav64_open(&infoSFXStart, path);
  constructPath(path, "Winner"_u64, sizeof(path)-1);
  wav64_open(&infoSFXWin, path);
}

AudioManager::~AudioManager() {
  for(auto &sfx : sfxMap) {
    free_uncached(sfx.second.sampleData);
    wav64_close(&sfx.second.source);
  }
  wav64_close(&bgm);
  wav64_close(&infoSFXStart);
  wav64_close(&infoSFXWin);
}

void AudioManager::update(const T3DVec3 &camPos, const T3DVec3 &camTarget, float deltaTime) {
  currCamPos = camPos;
  listenerDir = camTarget - camPos;
  t3d_vec3_norm(listenerDir);

  ticks = get_ticks();
  bgmVolume.update(deltaTime);
  float fadeNorm = bgmVolume.value / BGM_FADE_TIME;
  fadeNorm *= volBGM;
  if(fadeNorm <= 0.0) {
    if(mixer_ch_playing(CHANNEL_BGM)) {
      mixer_ch_stop(CHANNEL_BGM);
      wav64_close(&bgm);
    }
  } else {
    mixer_ch_set_vol(CHANNEL_BGM, fadeNorm, fadeNorm);
  }

  //mixer_try_play();
  ticks = get_ticks() - ticks;
}

void AudioManager::playBGM(uint64_t name) {
  char path[]{FS_BASE_PATH "bgm/01234567.wav64\0"};
  constructPath(path, name, sizeof(path)-1);

  wav64_open(&bgm, path);
  wav64_set_loop(&bgm, true);
  mixer_ch_set_vol(CHANNEL_BGM, 0.01f, 0.01f);
  mixer_ch_set_limits(CHANNEL_BGM, 0, 48000, 0);
  wav64_play(&bgm, CHANNEL_BGM);

  bgmVolume.target = BGM_FADE_TIME;
  bgmVolume.value = 0.1f;
}

void AudioManager::stopBGM() {
  bgmVolume.target = 0;
}

void AudioManager::setBGMVolume(float vol) {
  bgmVolume.target = vol;
}

void AudioManager::setVolume3D(int channel, const T3DVec3 &soundPos, float baseVolume)
{
  auto listenerToSfx = soundPos - currCamPos;
  float dist = t3d_vec3_len(listenerToSfx);
  if(dist < 0.0001f)dist = 0.0001f;
  float volume = (1.0f / dist) * baseVolume * 3.0f;
  volume = fminf(volume, 1.0f);

  listenerToSfx /= dist;
  T3DVec3 cross;
  t3d_vec3_cross(cross, listenerDir, {0.0f, 1.0f, 0.0f});
  float pan = t3d_vec3_dot(listenerToSfx, cross);
  pan = pan * 0.5f + 0.5f;
  /*debugf("SFX-3D: %.2f %.2f %.2f -> %.2f %.2f %.2f: pan: %f, vol: %f\n",
    soundPos.x, soundPos.y, soundPos.z,
    currCamPos.x, currCamPos.y, currCamPos.z,
    pan, volume);*/
  mixer_ch_set_vol_pan(channel, volume, pan);
}

uint32_t AudioManager::playSFX(uint64_t name, const T3DVec3 &pos, SfxConf conf) {

  auto it = sfxMap.find(name);
  if(it == sfxMap.end()) {
    wav64_t sfx;
    char path[]{FS_BASE_PATH "sfx/01234567.wav64\0"};
    constructPath(path, name, sizeof(path)-1);

    wav64_open(&sfx, path);
    it = sfxMap.insert({name, {sfx}}).first;

    uint32_t dataSize = getWaveSize(&it->second.source);
    it->second.source.wave.read = waveformRead;
    it->second.source.wave.ctx = &it->second;
    it->second.sampleData = (uint8_t*)malloc_uncached(dataSize);
    read(it->second.source.current_fd, CachedAddr(it->second.sampleData), dataSize);

    for(auto & instance : it->second.instances) {
      instance.sampleDataStart = it->second.sampleData;
      instance.sampleDataCurr = it->second.sampleData;
      instance.bps = (it->second.source.wave.bits == 8 ? 0 : 1) + (it->second.source.wave.channels == 2 ? 1 : 0);
      instance.wave = it->second.source;
      instance.wave.wave.ctx = &instance;
    }
    //data_cache_hit_writeback(it->second.sampleData, dataSize);
  }

  // check if any channel is free
  int ch = findFreeChannel();
  if(ch < 0) {
    //debugf("SFX: no free channels!\n");
    return 0;
  }

  // find free instance in SFX
  for(auto & instance : it->second.instances) {
    if(instance.channel == 0 || !mixer_ch_playing(instance.channel)) {
      instance.channel = ch;
      float vol = conf.volume * volSFX;
      if(conf.is2D) {
        mixer_ch_set_vol(ch, vol, vol);
      } else {
        setVolume3D(ch, pos, vol);
      }
      mixer_ch_play(ch, &instance.wave.wave);
      if(conf.variation) {
        float var = (conf.variation / 255.0f) * Math::rand01() * 10000.0f;
        mixer_ch_set_freq(ch, instance.wave.wave.frequency - var);
      }
      return 0;
    }
  }
  //debugf("SFX: no free instance!\n");
  return 0;
}

uint32_t AudioManager::getActiveChannelMask() {
  uint32_t mask = 0;
  uint32_t maskBit = 1;
  for(int i=3; i<CHANNEL_COUNT; ++i) {
    if(mixer_ch_playing(i)) {
      mask |= maskBit;
    }
    maskBit <<= 1;
  }
  return mask;
}

void AudioManager::playInfoSFX(uint64_t name) {
  if(name == "Winner"_u64) {
    wav64_play(&infoSFXWin, CHANNEL_INFO);
  } else {
    wav64_play(&infoSFXStart, CHANNEL_INFO);
  }
}
