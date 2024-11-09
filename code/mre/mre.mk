ASSETS_LIST += \
	filesystem/mre/video.wav64 \
	filesystem/mre/video.m1v

filesystem/mre/video.wav64: build/code/mre/video.wav
	@mkdir -p $(dir $@)
	@echo "    [SFX] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

build/code/mre/video.wav: assets/mre/video.mp4
	@mkdir -p $(dir $@)
	@echo "    [MP4 to WAV] $@"
	ffmpeg -y -i "$<" -vn -acodec pcm_s16le -ar 32000 -ac 1 "$@"

filesystem/mre/video.m1v: assets/mre/video.mp4
	@mkdir -p $(dir $@)
	@echo "    [MP4 to M1V] $@"
	ffmpeg -y -i "$<" -vb 800K -vf 'scale=288:-1,crop=288:208:(in_w/2-144):(in_h/2-104)' -r 20 "$@"