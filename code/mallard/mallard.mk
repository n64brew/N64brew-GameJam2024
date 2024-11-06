ASSETS_LIST += \
	filesystem/mallard/movie.wav64 \
	filesystem/mallard/movie.m1v

# Allows us to use a wav64 asset file.
$(FILESYSTEM_DIR)/%.wav64: $(ASSETS_DIR)/%.wav64
	@mkdir -p $(dir $@)
	@echo "    [WAV64] $@"
	cp "$<" "$@"

# Allows us to use a m1v asset file.
$(FILESYSTEM_DIR)/%.m1v: $(ASSETS_DIR)/%.m1v
	@mkdir -p $(dir $@)
	@echo "    [M1V] $@"
	cp "$<" "$@"