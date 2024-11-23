# sauna.sprite https://www.rawpixel.com/image/5923005
# loyly.wav64 https://freesound.org/people/Nakhas/sounds/522882/
# sj-polkka.xm64 https://musescore.com/user/179734/scores/5907389
#                samples from https://freewavesamples.com/
ASSETS_LIST += \
	filesystem/avanto/sauna.sprite \
	filesystem/avanto/sauna-depth.sprite \
	filesystem/avanto/guy.t3dm \
	filesystem/avanto/skin.sprite \
	filesystem/avanto/ukko.t3dm \
	filesystem/avanto/ukko-skin.sprite \
	filesystem/avanto/loyly.wav64 \
	filesystem/avanto/sj-polkka.xm64

AVANTO_AUDIOCONV_FLAGS += --wav-mono --wav-resample 22050 --wav-compress 3
$(FILESYSTEM_DIR)/avanto/%.wav64: $(ASSETS_DIR)/avanto/%.mp3
	@mkdir -p $(dir $@)
	@echo "    [AVANTO MP3 SFX] $@"
	$(N64_AUDIOCONV) $(AVANTO_AUDIOCONV_FLAGS) -o $(dir $@) "$<"

AVANTO_MKSPRITE_FLAGS=-c 3
$(FILESYSTEM_DIR)/avanto/%.sprite: $(ASSETS_DIR)/avanto/%.png
	@mkdir -p $(dir $@)
	@echo "    [AVANTO SPRITE] $@"
	$(N64_MKSPRITE) $(AVANTO MKSPRITE_FLAGS) -o $(dir $@) "$<"
