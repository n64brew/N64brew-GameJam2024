build/code/boss_fight/%.o: N64_CXXFLAGS += -std=gnu++20 -fno-exceptions -O2

BOSS_FIGHT_assets_coll = $(wildcard assets/boss_fight/*.coll)
BOSS_FIGHT_assets_scene = $(wildcard assets/boss_fight/*.scene)

BOSS_FIGHT_assets_png = $(wildcard assets/boss_fight/*.png) $(wildcard assets/boss_fight/grass/*.png) \
	$(wildcard assets/boss_fight/ui/*.png) $(wildcard assets/boss_fight/ptx/*.png) \
	$(wildcard assets/boss_fight/obj/*.png)

BOSS_FIGHT_assets_glb = $(wildcard assets/boss_fight/*.glb) $(wildcard assets/boss_fight/obj/*.glb)
BOSS_FIGHT_assets_ttf = $(wildcard assets/boss_fight/*.ttf)
BOSS_FIGHT_assets_mp3 = $(wildcard assets/boss_fight/bgm/*.mp3)
BOSS_FIGHT_assets_wav = $(wildcard assets/boss_fight/sfx/*.wav)

BOSS_FIGHT_assets_conv = $(patsubst assets/%,filesystem/%,$(BOSS_FIGHT_assets_coll)) \
              			 $(patsubst assets/%,filesystem/%,$(BOSS_FIGHT_assets_scene)) \
              			 $(patsubst assets/%,filesystem/%,$(BOSS_FIGHT_assets_png:%.png=%.sprite)) \
              			 $(patsubst assets/%,filesystem/%,$(BOSS_FIGHT_assets_glb:%.glb=%.t3dm)) \
              			 $(patsubst assets/%,filesystem/%,$(BOSS_FIGHT_assets_ttf:%.ttf=%.font64)) \
              			 $(patsubst assets/%,filesystem/%,$(BOSS_FIGHT_assets_mp3:%.mp3=%.wav64)) \
              			 $(patsubst assets/%,filesystem/%,$(BOSS_FIGHT_assets_wav:%.wav=%.wav64))

#assets/boss_fight/map.scene: assets/boss_fight/map.glb
#	@echo "    [SCENE] $@"
#	code/boss_fight/tools/gltf_to_scene "$<" assets/boss_fight/map.scene

#assets/boss_fight/map.coll: assets/boss_fight/map.glb
#	@echo "    [COLL] $@"
#	code/boss_fight/tools/gltf_to_coll "$<" assets/boss_fight/map.coll

filesystem/boss_fight/%.coll: assets/boss_fight/%.coll
	@mkdir -p $(dir $@)
	@echo "    [COLL] $@"
	$(N64_BINDIR)/mkasset -c 3 -w 256 -o filesystem/boss_fight "$<"

filesystem/boss_fight/%.scene: assets/boss_fight/%.scene
	@mkdir -p $(dir $@)
	@echo "    [SCENE] $@"
	$(N64_BINDIR)/mkasset -c 2 -w 256 -o filesystem/boss_fight "$<"

BOSS_FIGHT_AUDIOCONV_FLAGS = --wav-resample 22050 --wav-mono

filesystem/boss_fight/bgm/%.wav64: assets/boss_fight/bgm/%.mp3
	@mkdir -p $(dir $@)
	@echo "    [SFX] $@"
	$(N64_AUDIOCONV) $(BOSS_FIGHT_AUDIOCONV_FLAGS) -o $(dir $@) "$<"

filesystem/boss_fight/sfx/%.wav64: assets/boss_fight/sfx/%.wav
	@mkdir -p $(dir $@)
	@echo "    [SFX] $@"
	$(N64_AUDIOCONV) $(BOSS_FIGHT_AUDIOCONV_FLAGS) --wav-compress 0 -o $(dir $@) "$<"
	# $(N64_BINDIR)/mkasset -c 3 -o $(dir $@) $@


# 8000, 16000, 22050, 32000, 44100
filesystem/boss_fight/sfx/Coin.wav64: BOSS_FIGHT_AUDIOCONV_FLAGS = --wav-resample 32000 --wav-mono
filesystem/boss_fight/sfx/Men0.wav64: BOSS_FIGHT_AUDIOCONV_FLAGS = --wav-resample 32000 --wav-mono

filesystem/boss_fight/sfx/PotBreak.wav64: BOSS_FIGHT_AUDIOCONV_FLAGS = --wav-resample 16000 --wav-mono
filesystem/boss_fight/sfx/VoidOn.wav64: BOSS_FIGHT_AUDIOCONV_FLAGS = --wav-resample 18000 --wav-mono
filesystem/boss_fight/sfx/BoxHit.wav64: BOSS_FIGHT_AUDIOCONV_FLAGS = --wav-resample 18000 --wav-mono

filesystem/boss_fight/bgm/Main.wav64: BOSS_FIGHT_AUDIOCONV_FLAGS = --wav-resample 24000 --wav-mono --wav-compress 1
filesystem/boss_fight/bgm/Boss.wav64: BOSS_FIGHT_AUDIOCONV_FLAGS = --wav-resample 19000 --wav-mono --wav-compress 1
filesystem/boss_fight/map.t3dm: T3DM_FLAGS += --bvh

ASSETS_LIST += $(BOSS_FIGHT_assets_conv)
#	filesystem/boss_fight/feylon_reactor.wav64
