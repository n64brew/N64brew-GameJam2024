ROMNAME = gamejam2024
ROMTITLE = "N64BREW GAMEJAM 2024"

BUILD_DIR = build
ASSETS_DIR = assets
MINIGAME_DIR = code
FILESYSTEM_DIR = filesystem
MINIGAMEDSO_DIR = $(FILESYSTEM_DIR)/minigames

SRC = main.c core.c minigame.c menu.c

filesystem/squarewave.font64: MKFONT_FLAGS += --outline 1 --range all

###

include $(N64_INST)/include/n64.mk
include $(N64_INST)/include/t3d.mk

MINIGAMES_LIST = $(notdir $(wildcard $(MINIGAME_DIR)/*))
DSO_LIST = $(addprefix $(MINIGAMEDSO_DIR)/, $(addsuffix .dso, $(MINIGAMES_LIST)))

IMAGE_LIST = $(wildcard $(ASSETS_DIR)/*.png) $(wildcard $(ASSETS_DIR)/core/*.png)
FONT_LIST  = $(wildcard $(ASSETS_DIR)/*.ttf)
MODEL_LIST  = $(wildcard $(ASSETS_DIR)/*.glb)
SOUND_LIST  = $(wildcard $(ASSETS_DIR)/*.wav) $(wildcard $(ASSETS_DIR)/core/*.wav)
SOUND2_LIST  = $(wildcard $(ASSETS_DIR)/*.mp3) $(wildcard $(ASSETS_DIR)/core/*.mp3)
MUSIC_LIST  = $(wildcard $(ASSETS_DIR)/*.xm)
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(IMAGE_LIST:%.png=%.sprite))
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(FONT_LIST:%.ttf=%.font64))
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(MODEL_LIST:%.glb=%.t3dm))
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(SOUND_LIST:%.wav=%.wav64))
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(SOUND2_LIST:%.mp3=%.wav64))
ASSETS_LIST += $(subst $(ASSETS_DIR),$(FILESYSTEM_DIR),$(MUSIC_LIST:%.xm=%.xm64))

ifeq ($(DEBUG), 1)
	N64_CFLAGS += -g -O0
	N64_LDFLAGS += -g
else
	N64_CFLAGS += -O2
endif

all: $(ROMNAME).z64

$(FILESYSTEM_DIR)/%.sprite: $(ASSETS_DIR)/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -o $(dir $@) "$<"

$(FILESYSTEM_DIR)/%.font64: $(ASSETS_DIR)/%.ttf
	@mkdir -p $(dir $@)
	@echo "    [FONT] $@"
	$(N64_MKFONT) $(MKFONT_FLAGS) -o $(dir $@) "$<"

$(FILESYSTEM_DIR)/%.t3dm: $(ASSETS_DIR)/%.glb
	@mkdir -p $(dir $@)
	@echo "    [T3D-MODEL] $@"
	$(T3D_GLTF_TO_3D) $(T3DM_FLAGS) "$<" $@
	$(N64_BINDIR)/mkasset -c 2 -o $(dir $@) $@

$(FILESYSTEM_DIR)/%.wav64: $(ASSETS_DIR)/%.wav
	@mkdir -p $(dir $@)
	@echo "    [SFX] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

$(FILESYSTEM_DIR)/%.wav64: $(ASSETS_DIR)/%.mp3
	@mkdir -p $(dir $@)
	@echo "    [SFX] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

$(FILESYSTEM_DIR)/%.xm64: $(ASSETS_DIR)/%.xm
	@mkdir -p $(dir $@)
	@echo "    [XM] $@"
	$(N64_AUDIOCONV) $(AUDIOCONV_FLAGS) -o $(dir $@) "$<"

define MINIGAME_template
SRC_$(1) = \
	$$(wildcard $$(MINIGAME_DIR)/$(1)/*.c) \
	$$(wildcard $$(MINIGAME_DIR)/$(1)/*/*.c) \
	$$(wildcard $$(MINIGAME_DIR)/$(1)/*.cpp) \
	$$(wildcard $$(MINIGAME_DIR)/$(1)/*/*.cpp)
$$(MINIGAMEDSO_DIR)/$(1).dso: $$(SRC_$(1):%.cpp=$$(BUILD_DIR)/%.o)
$$(MINIGAMEDSO_DIR)/$(1).dso: $$(SRC_$(1):%.c=$$(BUILD_DIR)/%.o)
-include $$(MINIGAME_DIR)/$(1)/$(1).mk
endef

$(foreach minigame, $(MINIGAMES_LIST), $(eval $(call MINIGAME_template,$(minigame))))

MAIN_ELF_EXTERNS := $(BUILD_DIR)/$(ROMNAME).externs
$(MAIN_ELF_EXTERNS): $(DSO_LIST)
$(BUILD_DIR)/$(ROMNAME).dfs: $(ASSETS_LIST) $(DSO_LIST)
$(BUILD_DIR)/$(ROMNAME).elf: $(SRC:%.c=$(BUILD_DIR)/%.o) $(MAIN_ELF_EXTERNS)
$(ROMNAME).z64: N64_ROM_TITLE=$(ROMTITLE)
$(ROMNAME).z64: $(BUILD_DIR)/$(ROMNAME).dfs $(BUILD_DIR)/$(ROMNAME).msym

$(BUILD_DIR)/$(ROMNAME).msym: $(BUILD_DIR)/$(ROMNAME).elf

clean:
	rm -rf $(BUILD_DIR) $(FILESYSTEM_DIR) $(DSO_LIST) $(ROMNAME).z64 

-include $(wildcard $(BUILD_DIR)/*.d) $(wildcard $(BUILD_DIR)/*/*.d) $(wildcard $(BUILD_DIR)/*/*/*.d) $(wildcard $(BUILD_DIR)/*/*/*/*.d)

.PHONY: all clean
