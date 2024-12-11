ASSETS_LIST += \
	filesystem/mallard/CelticGaramondTheSecond.font64 \
	filesystem/mallard/HaloDek.font64 \
	filesystem/mallard/HaloDekMedium.font64 \
	filesystem/mallard/HaloDekBig.font64 \
	filesystem/mallard/mallard_intro_music.xm64 \
	filesystem/mallard/mallard_game_music.xm64 \
	filesystem/mallard/libdragon.rgba32.sprite \
	filesystem/mallard/mallard_logo_white.rgba32.sprite \
	filesystem/mallard/mallard_logo_black.rgba32.sprite \
	filesystem/mallard/one/duck_walk_1.rgba32.sprite \
	filesystem/mallard/two/duck_walk_1.rgba32.sprite \
	filesystem/mallard/three/duck_walk_1.rgba32.sprite \
	filesystem/mallard/four/duck_walk_1.rgba32.sprite \
	filesystem/mallard/one/duck_slap.rgba32.sprite \
	filesystem/mallard/two/duck_slap.rgba32.sprite \
	filesystem/mallard/three/duck_slap.rgba32.sprite \
	filesystem/mallard/four/duck_slap.rgba32.sprite \
	filesystem/mallard/one/duck_run.rgba32.sprite \
	filesystem/mallard/two/duck_run.rgba32.sprite \
	filesystem/mallard/three/duck_run.rgba32.sprite \
	filesystem/mallard/four/duck_run.rgba32.sprite \
	filesystem/mallard/one/duck_idle.rgba32.sprite \
	filesystem/mallard/two/duck_idle.rgba32.sprite \
	filesystem/mallard/three/duck_idle.rgba32.sprite \
	filesystem/mallard/four/duck_idle.rgba32.sprite \
	filesystem/mallard/one/duck_damage.rgba32.sprite \
	filesystem/mallard/two/duck_damage.rgba32.sprite \
	filesystem/mallard/three/duck_damage.rgba32.sprite \
	filesystem/mallard/four/duck_damage.rgba32.sprite \
	filesystem/mallard/snowman/snowman_idle_evil.rgba32.sprite \
	filesystem/mallard/snowman/snowman_damage_evil.rgba32.sprite \
	filesystem/mallard/snowman/snowman_jump_evil.rgba32.sprite \
	filesystem/mallard/mallard_game_paused_text.rgba32.sprite \
	filesystem/mallard/mallard_background_park.rgba32.sprite

filesystem/mallard/HaloDekBig.font64: $(ASSETS_DIR)/mallard/HaloDekBig.ttf
	@mkdir -p $(dir $@)
	@echo "    [FONT] $@"
	$(N64_MKFONT) $(MKFONT_FLAGS) --verbose --range 50-50 --range 41-41 --range 55-55 --range 53-53 --range 44-45 --range 2e-2e --size 60 --outline 1 -o $(dir $@) "$<"

filesystem/mallard/HaloDekMedium.font64: $(ASSETS_DIR)/mallard/HaloDekMedium.ttf
	@mkdir -p $(dir $@)
	@echo "    [FONT] $@"
	$(N64_MKFONT) $(MKFONT_FLAGS) --verbose --range 30-39 --range 2e-2e --range 50-50 --range 20-20 --range 57-57 --range 49-49 --range 4E-4E --range 53-53 --range 44-44 --range 52-52 --range 41-41 --size 36 --outline 1 -o $(dir $@) "$<"