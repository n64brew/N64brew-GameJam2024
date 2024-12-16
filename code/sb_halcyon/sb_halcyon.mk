# Project-specific assets
# TODO variable names
HALCYON_ASSETS = filesystem/strawberry_byte
HALCYON_SOUND_DIR = $(HALCYON_ASSETS)/sound
HALCYON_UI_DIR = $(HALCYON_ASSETS)/ui

HALCYON_T3DM_FILES := $(HALCYON_ASSETS)/wolfie.t3dm \
                      $(HALCYON_ASSETS)/s4ys.t3dm \
                      $(HALCYON_ASSETS)/dogman.t3dm \
                      $(HALCYON_ASSETS)/mew.t3dm \
                      $(HALCYON_ASSETS)/platform.t3dm \
                      $(HALCYON_ASSETS)/lava.t3dm \
                      $(HALCYON_ASSETS)/platform2.t3dm \
                      $(HALCYON_ASSETS)/cloud_base.t3dm

HALCYON_SPRITE_FILES := $(HALCYON_ASSETS)/wolf_eye.sprite \
                        $(HALCYON_ASSETS)/frog_eye.sprite \
                        $(HALCYON_ASSETS)/libdragon_logo.sprite \
                        $(HALCYON_ASSETS)/nose.sprite \
                        $(HALCYON_ASSETS)/n64brew.sprite \
                        $(HALCYON_ASSETS)/mew_eye.sprite \
                        $(HALCYON_ASSETS)/mew_ear.sprite \
                        $(HALCYON_ASSETS)/jam_logo.sprite \
                        $(HALCYON_ASSETS)/dogman_eye.sprite \
                        $(HALCYON_ASSETS)/dogman_eyebrow.sprite \
                        $(HALCYON_ASSETS)/dogman_mouth.sprite \
                        $(HALCYON_ASSETS)/fast64.sprite \
                        $(HALCYON_ASSETS)/bricks48.i8.sprite \
                        $(HALCYON_ASSETS)/lava00.rgba16.sprite \
                        $(HALCYON_ASSETS)/lava08.rgba16.sprite

HALCYON_SOUND_FILES := $(HALCYON_SOUND_DIR)/hexagone.wav64 \
                       $(HALCYON_SOUND_DIR)/sky_high.xm64 \
                       $(HALCYON_SOUND_DIR)/grunt-01.wav64 \
                       $(HALCYON_SOUND_DIR)/stones-falling.wav64 \
                       $(HALCYON_SOUND_DIR)/strong_wind_blowing.wav64

HALCYON_UI_SPRITE_FILES := $(HALCYON_UI_DIR)/buttons/control_stick.ia8.sprite \
                           $(HALCYON_UI_DIR)/buttons/d_pad_triggers.ia8.sprite \
                           $(HALCYON_UI_DIR)/buttons/c_buttons0.rgba32.sprite \
                           $(HALCYON_UI_DIR)/buttons/c_buttons1.rgba32.sprite \
                           $(HALCYON_UI_DIR)/buttons/face_buttons0.rgba32.sprite \
                           $(HALCYON_UI_DIR)/buttons/face_buttons1.rgba32.sprite \
                           $(HALCYON_UI_DIR)/logos/libdragon.ia4.sprite \
                           $(HALCYON_UI_DIR)/logos/mixamo.ia4.sprite \
                           $(HALCYON_UI_DIR)/logos/t3d.ia8.sprite \
                           $(HALCYON_UI_DIR)/logos/sb_b0.rgba32.sprite \
                           $(HALCYON_UI_DIR)/logos/sb_b1.rgba32.sprite \
                           $(HALCYON_UI_DIR)/logos/sb_top.rgba32.sprite \
                           $(HALCYON_UI_DIR)/panels/gloss.ia4.sprite \
                           $(HALCYON_UI_DIR)/panels/clouds.ia8.sprite \
                           $(HALCYON_UI_DIR)/panels/pattern_tessalate.ia4.sprite

HALCYON_FONT_FILES := $(HALCYON_UI_DIR)/fonts/TitanOne-Regular.font64 \
                      $(HALCYON_UI_DIR)/fonts/OilOnTheWater-ee5O.font64

# Final assets list
ASSETS_LIST += $(HALCYON_T3DM_FILES) $(HALCYON_SPRITE_FILES) $(HALCYON_SOUND_FILES) $(HALCYON_UI_SPRITE_FILES) $(HALCYON_FONT_FILES)

# t3d flags
$(HALCYON_ASSETS)/s4ys.t3dm: T3DM_FLAGS = --base-scale=1
$(HALCYON_ASSETS)/wolfie.t3dm: T3DM_FLAGS = --base-scale=1
$(HALCYON_ASSETS)/dogman.t3dm: T3DM_FLAGS = --base-scale=1
$(HALCYON_ASSETS)/mew.t3dm: T3DM_FLAGS = --base-scale=1
$(HALCYON_ASSETS)/platform.t3dm: T3DM_FLAGS = --base-scale=1
$(HALCYON_ASSETS)/lava.t3dm: T3DM_FLAGS = --base-scale=1
$(HALCYON_ASSETS)/platform2.t3dm: T3DM_FLAGS = --base-scale=1
$(HALCYON_ASSETS)/cloud_base.t3dm: T3DM_FLAGS = --base-scale=1

# audioconv flags
$(HALCYON_ASSETS)/hexagone.wav64: AUDIOCONV_FLAGS = --wav-compress=3
$(HALCYON_ASSETS)/sky_high.xm64: AUDIOCONV_FLAGS = ''
$(HALCYON_ASSETS)/stones-falling.wav64: AUDIOCONV_FLAGS = --wav-compress=3 --wav-mono
$(HALCYON_ASSETS)/strong_wind_blowing.wav64: AUDIOCONV_FLAGS = --wav-compress=3 --wav-mono
$(HALCYON_ASSETS)/grunt-01.wav64: AUDIOCONV_FLAGS = --wav-compress=3 --wav-mono


# font64 flags
$(HALCYON_UI_DIR)/fonts/OilOnTheWater-ee5O.font64: MKFONT_FLAGS += --outline 2 --size 18
$(HALCYON_UI_DIR)/fonts/TitanOne-Regular.font64: MKFONT_FLAGS += --outline 1 --size 12