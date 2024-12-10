ASSETS_LIST += \
	filesystem/old_gods/crate00.ci8.sprite \
	filesystem/old_gods/sand12.ci4.sprite \
	filesystem/old_gods/checker.sprite \
	filesystem/old_gods/shadow.i8.sprite \
	filesystem/old_gods/sea_foam.sprite \
	filesystem/old_gods/player1_panel_64_64.sprite \
	filesystem/old_gods/player2_panel_64_64.sprite \
	filesystem/old_gods/player3_panel_64_64.sprite \
	filesystem/old_gods/player4_panel_64_64.sprite \
	filesystem/old_gods/panel_pink_64_64.sprite \
	filesystem/old_gods/panel_white_64_64.sprite \
	filesystem/old_gods/white.sprite \
	filesystem/old_gods/stone.ci4.sprite \
	filesystem/old_gods/pattern.i8.sprite \
	filesystem/old_gods/ZenDots-Regular.font64 \
	filesystem/old_gods/Jumpman_H1.font64 \
	filesystem/old_gods/Jumpman_H2.font64 \
	filesystem/old_gods/Jumpman_P.font64 \
	filesystem/old_gods/box.t3dm \
	filesystem/old_gods/cylinder.t3dm \
	filesystem/old_gods/food.t3dm \
	filesystem/old_gods/sea_foam.t3dm \
	filesystem/old_gods/sea_trail.t3dm \
	filesystem/old_gods/shadow.t3dm \
	filesystem/old_gods/map2.t3dm \
	filesystem/old_gods/snake.t3dm \
	filesystem/old_gods/rat.t3dm \
	filesystem/old_gods/torus.t3dm \
	filesystem/old_gods/shark.t3dm \
	filesystem/old_gods/bottled_bubbles.xm64 \
	filesystem/old_gods/Item2A.wav64 \
	filesystem/old_gods/sandy_seaside.wav64 \
	filesystem/old_gods/Ability_Learn.wav64 \
	filesystem/old_gods/paper_crush.wav64 \

filesystem/old_gods/Jumpman_H1.font64: MKFONT_FLAGS += --outline -1 --size 64 --char-spacing 2
filesystem/old_gods/Jumpman_H2.font64: MKFONT_FLAGS += --outline -1 --size 32 --char-spacing 2
filesystem/old_gods/Jumpman_P.font64: MKFONT_FLAGS += --outline -1 --size 32 --char-spacing 2

# UNFLoader files
UNFLLOADER_DIR = code/old_gods/UNFLoader/
DEBUGFILES = $(UNFLLOADER_DIR)debug.c $(UNFLLOADER_DIR)usb.c
CFLAGS += -Icode/old_gods/AF_Math/include -Icode/old_gods/AF_Lib/include -I$(UNFLLOADER_DIR)

#==== CREDITS =====
#pikcup rat https://opengameart.org/content/paper-crush-sounds
# god eat sound effect https://opengameart.org/content/7-assorted-sound-effects-menu-level-up
#ui and othe effect sounds https://opengameart.org/content/ui-and-item-sounds-sample-1
# jumpman font https://fontmeme.com/fonts/jumpman-font/
# Palm Tree https://sketchfab.com/3d-models/palm-tree-e7677eb753ce4c16bc3c5dbaa8aefa27
# tune https://opengameart.org/content/sandy-seaside-2
# sound effects https://opengameart.org/content/ui-and-item-sound-effect-jingles-sample-2