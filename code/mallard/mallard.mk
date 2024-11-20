ASSETS_LIST += \
	filesystem/mallard/CelticGaramondTheSecond.font64 \
	filesystem/mallard/HaloDek.font64 \
	filesystem/mallard/HaloDekBig.font64 \
	filesystem/mallard/mallard_intro_music.xm64 \
	filesystem/mallard/libdragon.rgba32.sprite \
	filesystem/mallard/mallard_logo_white.rgba32.sprite \
	filesystem/mallard/mallard_logo_black.rgba32.sprite \
	filesystem/mallard/mallard_idle.rgba32.sprite \
	filesystem/mallard/mallard_game_paused_text.rgba32.sprite \
	filesystem/mallard/mallard_background_park.rgba32.sprite

filesystem/mallard/HaloDekBig.font64: $(ASSETS_DIR)/mallard/HaloDekBig.ttf
	@mkdir -p $(dir $@)
	@echo "    [FONT] $@"
	$(N64_MKFONT) $(MKFONT_FLAGS) --verbose --range 50-50 --range 41-41 --range 55-55 --range 53-53 --range 44-45 --range 2e-2e --size 60 --outline 1 -o $(dir $@) "$<"