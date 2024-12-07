ASSETS_LIST += \
	filesystem/avanto/sauna.sprite \
	filesystem/avanto/guy.t3dm \
	filesystem/avanto/skin.sprite \
	filesystem/avanto/ukko.t3dm \
	filesystem/avanto/ukko-skin.sprite \
	filesystem/avanto/loyly.wav64 \
	filesystem/avanto/sj-polkka.xm64 \
	filesystem/avanto/banner.font64 \
	filesystem/avanto/timer.font64 \
	filesystem/avanto/door.wav64 \
	filesystem/avanto/pine-64-00.sprite \
	filesystem/avanto/pine-64-01.sprite \
	filesystem/avanto/pine-64-10.sprite \
	filesystem/avanto/pine-64-11.sprite \
	filesystem/avanto/cabin.sprite \
	filesystem/avanto/snow.sprite \
	filesystem/avanto/metsa-4bit-top.sprite \
	filesystem/avanto/metsa-4bit-bottom.sprite \
	filesystem/avanto/ice-gs.sprite \
	filesystem/avanto/planks-gs.sprite \
	filesystem/avanto/water-bg.sprite \
	filesystem/avanto/finish.sprite \
	filesystem/avanto/splash.wav64 \
	filesystem/avanto/map.t3dm \
	filesystem/avanto/tail.sprite \
	filesystem/avanto/balloon.sprite \
	filesystem/avanto/penalty.sprite \
	filesystem/avanto/shadow.t3dm \
	filesystem/avanto/shadow.sprite \
	filesystem/avanto/kiuas.sprite \
	filesystem/avanto/unit-cube.t3dm

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

$(FILESYSTEM_DIR)/avanto/banner.font64: $(ASSETS_DIR)/squarewave.ttf
	@mkdir -p $(dir $@)
	@echo "    [AVANTO FONT] $@"
	$(N64_MKFONT) --outline 2 --range 20-5A -s 100 -o $(dir $@) "$<"
	mv "$(dir $@)/squarewave.font64" "$@"

$(FILESYSTEM_DIR)/avanto/timer.font64: $(ASSETS_DIR)/squarewave.ttf
	@mkdir -p $(dir $@)
	@echo "    [AVANTO FONT] $@"
	$(N64_MKFONT) --outline 1 --range 30-39 -s 48 --ellipsis 30,3 -o $(dir $@) "$<"
	mv "$(dir $@)/squarewave.font64" "$@"
