# sauna.sprite https://www.rawpixel.com/image/5923005
# loyly.wav64 https://freesound.org/people/Nakhas/sounds/522882/
# sj-polkka.xm64 https://musescore.com/user/179734/scores/5907389
#                samples from https://freewavesamples.com/
# door.wav64 https://opengameart.org/content/creaky-door-hinge-spooky
# pine-64-*.sprite https://opengameart.org/content/freebies-mundo-commissions
# cabin model and cabin.sprite https://opengameart.org/content/low-poly-log-cabin
# snow.sprite https://opengameart.org/content/terrain-textures-pack-from-stunt-rally-23
# ice-gs.sprite https://opengameart.org/content/terrain-textures-pack-from-stunt-rally-23
# planks-gs.sprite https://opengameart.org/content/handpainted-wood
# metsa-4-bit-* based on https://opengameart.org/content/freebies-mundo-commissions
#        and https://opengameart.org/content/jkms-tree-set
# water-bg.sprite https://opengameart.org/content/water-texture-pack
# finish.sprite own
# splash.wav64 https://opengameart.org/content/40-cc0-water-splash-slime-sfx
# tail.sprite own
# balloon.sprite own
# penalty.sprite own
# shadow.* own
# map.t3dm own
ASSETS_LIST += \
	filesystem/avanto/sauna.sprite \
	filesystem/avanto/sauna-depth.sprite \
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
	filesystem/avanto/shadow.sprite

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

$(FILESYSTEM_DIR)/avanto/sauna-depth.sprite: $(ASSETS_DIR)/avanto/sauna-depth.png
	@mkdir -p $(dir $@)
	@echo "    [AVANTO SPRITE] $@"
	$(N64_MKSPRITE) $(AVANTO MKSPRITE_FLAGS) -f RGBA16 -o $(dir $@) "$<"

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
