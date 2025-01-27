/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "pauseMenu.h"
#include "../scene.h"
#include "../../main.h"
#include "../../debug/debugDraw.h"

namespace {
  constexpr color_t COLOR_ACTIVE{0xFF, 0xF5, 0xEE, 0xFF};
  constexpr color_t COLOR_INACTIVE{0x80, 0x80, 0x80, 0xFF};

  constexpr float BAR_START_Y = 50.0f;
  constexpr uint32_t FB_SCALE_FACTOR = 2;

  float BANNER_START_X{};
  float blinkTimer{0.0f};
  constexpr float CENTER_X{SCREEN_WIDTH / 2};

  sprite_t *texBanner;
  sprite_t *texResume;
  sprite_t *texExit;

  surface_t fbBackup;

  rspq_block_t *setupDPL;

  int currOption{0};
  joypad_8way_t lastDir{};

  uint8_t grayScale(color_t c) {
    return (uint8_t)((c.r * 0.299f) + (c.g * 0.587f) + (c.b * 0.114f));
  }

  inline uint8_t blur(uint8_t *data) {
    constexpr uint32_t sx = SCREEN_WIDTH / 2;
    uint8_t *idx00 = data - sx - 1;
    uint8_t *idx01 = data - sx;
    uint8_t *idx02 = data - sx + 1;
    uint8_t *idx10 = data - 1;
    uint8_t *idx11 = data;
    uint8_t *idx12 = data + 1;
    uint8_t *idx20 = data + sx - 1;
    uint8_t *idx21 = data + sx;
    // skip last 3x3 value to do div. by 8 later

    uint32_t sum = *idx00 + *idx01 + *idx02 + *idx10 + *idx11 + *idx12 + *idx20 + *idx21;
    return sum / 8;
  }

  void backupFramebuffer(surface_t *lastFB) {
    uint64_t ticks = get_ticks();
    uint32_t idxDst = 0;

    auto *dataSrc = (uint16_t*)lastFB->buffer;
    auto *dataDst = (uint8_t*)fbBackup.buffer;

    for(uint32_t y=0; y<SCREEN_HEIGHT; y+=FB_SCALE_FACTOR) {
      for(uint32_t x=0; x<SCREEN_WIDTH; x+=FB_SCALE_FACTOR) {
        dataDst[idxDst++] = grayScale(color_from_packed16(dataSrc[(x + y * SCREEN_WIDTH)]));
      }
    }

    constexpr uint32_t sx = SCREEN_WIDTH / FB_SCALE_FACTOR;
    constexpr uint32_t sy = SCREEN_HEIGHT / FB_SCALE_FACTOR;
    auto dataDstEnd = dataDst + sx * sy;
    dataDst += sy*2; // skip 2 rows to not blur OOB
    dataDstEnd -= sy*2;

    for(; dataDst<dataDstEnd; ++dataDst) {
      *dataDst = blur(dataDst);
    }

    ticks = get_ticks() - ticks;
    debugf("Copy time: %lldus\n", TICKS_TO_US(ticks));
  }

}

PauseMenu::PauseMenu() {
  texBanner = sprite_load(FS_BASE_PATH "ui/bannerPause.rgba16.sprite");
  texResume = sprite_load(FS_BASE_PATH "ui/txtResume.ia8.sprite");
  texExit = sprite_load(FS_BASE_PATH "ui/txtExit.ia8.sprite");

  fbBackup = surface_alloc(FMT_I8,
    SCREEN_WIDTH / FB_SCALE_FACTOR,
    SCREEN_HEIGHT / FB_SCALE_FACTOR
  );

  BANNER_START_X = SCREEN_WIDTH / 2 - texBanner->width / 2;
  currOption = 0;
  lastDir = joypad_8way_t::JOYPAD_8WAY_NONE;

  rspq_block_begin();
      rdpq_mode_begin();
        rdpq_mode_combiner(RDPQ_COMBINER1((TEX0,0,PRIM,0), (0,0,0,PRIM)));
        rdpq_mode_alphacompare(10);
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_mode_filter(FILTER_BILINEAR);
      rdpq_mode_end();

    rdpq_blitparms_t p{};
    p.scale_x = FB_SCALE_FACTOR;
    p.scale_y = FB_SCALE_FACTOR;

    rdpq_set_prim_color({234,200,193, 0xFF});
    rdpq_tex_blit(&fbBackup, 0, 0, &p);

    rdpq_mode_combiner(RDPQ_COMBINER_TEX_FLAT);

    // static title banner
    rdpq_set_prim_color({0xFF, 0xFF, 0xFF, 0xFF});
    rdpq_sprite_blit(texBanner, BANNER_START_X, BAR_START_Y, nullptr);

  setupDPL = rspq_block_end();
}

PauseMenu::~PauseMenu() {
  rspq_block_free(setupDPL);
  sprite_free(texBanner);
  sprite_free(texResume);
  sprite_free(texExit);
  surface_free(&fbBackup);
}

void PauseMenu::update(Scene &scene, float deltaTime)
{
  surface_t *lastFB = scene.getLastFB();
  blinkTimer += deltaTime;
  auto ctrl = core_get_playercontroller(PLAYER_1);
  auto pressed = joypad_get_buttons_pressed(ctrl);

  bool needsClose = false;
  if(pressed.start) {
    isPaused = !isPaused;
    if(isPaused) {
      scene.getAudio().playSFX("MenuOpen"_u64);
      scene.getAudio().setBGMVolume(0.4f);
      rspq_wait();
      backupFramebuffer(lastFB);
    } else {
      needsClose = true;
      scene.getAudio().setBGMVolume(1.0f);
    }
  }

  if(!isPaused)return;

  if(pressed.a) {
    if(currOption == 0)needsClose = true;
    if(currOption == 1) {
      scene.getAudio().playSFX("UiOk"_u64, {.volume = 0.6f});
      scene.requestExit();
    }
  }

  if(needsClose) {
    isPaused = false;
    scene.getAudio().playSFX("UiOk"_u64, {.volume = 0.4f});
    scene.getAudio().setBGMVolume(1.0f);
  }

  auto dir = joypad_get_direction(ctrl, JOYPAD_2D_ANY);

  if(dir != lastDir) {
    if(dir == joypad_8way_t::JOYPAD_8WAY_UP) {currOption--; blinkTimer = T3D_PI/2;}
    if(dir == joypad_8way_t::JOYPAD_8WAY_DOWN) {currOption++; blinkTimer = T3D_PI/2;}
    currOption = currOption & 1;
    lastDir = dir;

    if(dir != joypad_8way_t::JOYPAD_8WAY_NONE) {
      scene.getAudio().playSFX("UiSelect"_u64, {.volume = 0.4f});
    }
  }
}

void PauseMenu::draw()
{
  if(!isPaused)return;
  Debug::printStart(); // Why? text broken otherwise
  rspq_block_run(setupDPL);

  rdpq_blitparms_t paramDef{};
  rdpq_blitparms_t paramActive{
    .scale_x = fm_sinf(blinkTimer * 8.0f) * 0.12f + 0.9f,
    .scale_y = fm_cosf(blinkTimer * 8.0f) * 0.12f + 0.9f,
  };

  float posY = BAR_START_Y + 90;
  sprite_t *options[2]{texResume, texExit};
  for(int i=0; i<2; ++i) {
    rdpq_set_prim_color(currOption == i ? COLOR_ACTIVE : COLOR_INACTIVE);
    auto &p = currOption == i ? paramActive : paramDef;
    p.cx = options[i]->width / 2;
    p.cy = options[i]->height / 2;
    rdpq_sprite_blit(options[i], CENTER_X, posY, &p);
    posY += 26;
  }
}
