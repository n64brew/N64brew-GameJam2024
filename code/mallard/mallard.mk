ASSETS_LIST += \
	filesystem/mallard/video.wav64 \
	filesystem/mallard/video.m1v

filesystem/mallard/video.wav64: build/code/mallard/video.wav
	@mkdir -p $(dir $@)
	@echo "    [SFX] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

build/code/mallard/video.wav: assets/mallard/video.mp4
	@mkdir -p $(dir $@)
	@echo "    [MP4 to WAV] $@"
	ffmpeg -y -i "$<" -vn -acodec pcm_s16le -ar 32000 -ac 1 "$@"

filesystem/mallard/video.m1v: assets/mallard/video.mp4
	@mkdir -p $(dir $@)
	@echo "    [MP4 to M1V] $@"
	ffmpeg -y -i "$<" -vb 800K -vf 'scale=288:-1,crop=288:208:(in_w/2-144):(in_h/2-104)' -r 20 "$@"