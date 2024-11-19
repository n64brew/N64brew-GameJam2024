ASSETS_LIST += \
	filesystem/mallard/CelticGaramondTheSecond.font64 \
	filesystem/mallard/HaloDek.font64 \
	filesystem/mallard/HaloDekBig.font64 \
	filesystem/mallard/mallard_intro_music.xm64 \
	filesystem/mallard/libdragon.rgba32.sprite \
	filesystem/mallard/mallard_logo_white.rgba32.sprite \
	filesystem/mallard/mallard_logo_black.rgba32.sprite \
	filesystem/mallard/mallard_idle.rgba32.sprite \
	filesystem/mallard/mallard_background_park.rgba32.sprite

filesystem/mallard/HaloDekBig.font64: $(ASSETS_DIR)/mallard/HaloDekBig.ttf
	@mkdir -p $(dir $@)
	@echo "    [FONT] $@"
	$(N64_MKFONT) $(MKFONT_FLAGS) --size 60 --outline 1 -o $(dir $@) "$<"