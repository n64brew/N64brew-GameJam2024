ASSETS_LIST += \
	filesystem/old_gods/sea_foam.sprite \
	filesystem/old_gods/particle.sprite \
	filesystem/old_gods/player1_panel_64_64.sprite \
	filesystem/old_gods/player2_panel_64_64.sprite \
	filesystem/old_gods/player3_panel_64_64.sprite \
	filesystem/old_gods/player4_panel_64_64.sprite \
	filesystem/old_gods/panel_pink_64_64.sprite \
	filesystem/old_gods/panel_white_64_64.sprite \
	filesystem/old_gods/white.sprite \
	filesystem/old_gods/smoke.sprite \
	filesystem/old_gods/grave.sprite \
	filesystem/old_gods/pentogram.sprite \
	filesystem/old_gods/ZenDots-Regular.font64 \
	filesystem/old_gods/Jumpman_H1.font64 \
	filesystem/old_gods/Jumpman_H2.font64 \
	filesystem/old_gods/Jumpman_P.font64 \
	filesystem/old_gods/sea_foam.t3dm \
	filesystem/old_gods/sea_trail.t3dm \
	filesystem/old_gods/shadow.t3dm \
	filesystem/old_gods/map2.t3dm \
	filesystem/old_gods/snake.t3dm \
	filesystem/old_gods/rat.t3dm \
	filesystem/old_gods/shark.t3dm \
	filesystem/old_gods/attackWave.t3dm \
	filesystem/old_gods/smoke.t3dm \
	filesystem/old_gods/alter.t3dm \
	filesystem/old_gods/grave.t3dm \
	filesystem/old_gods/torus.t3dm \
	filesystem/old_gods/box.t3dm \
	filesystem/old_gods/bottled_bubbles.xm64 \
	filesystem/old_gods/Item2A.wav64 \
	filesystem/old_gods/sandy_seaside.wav64 \
	filesystem/old_gods/Ability_Learn.wav64 \
	filesystem/old_gods/paper_crush.wav64 \


filesystem/old_gods/Jumpman_H1.font64: MKFONT_FLAGS += --outline -1 --size 64 --char-spacing 2
filesystem/old_gods/Jumpman_H2.font64: MKFONT_FLAGS += --outline -1 --size 32 --char-spacing 2
filesystem/old_gods/Jumpman_P.font64: MKFONT_FLAGS += --outline -1 --size 32 --char-spacing 2

build/code/old_gods/%.o: CFLAGS += -Icode/old_gods/AF_Math/include -Icode/old_gods/AF_Lib/include

#==== CREDITS =====
# SOUNDS
# pick cup rat Credits: Vinrax (CC-BY 3.0) https://opengameart.org/content/paper-crush-sounds
# god eat sound effect Credits: Joth (CCO) https://opengameart.org/content/7-assorted-sound-effects-menu-level-up
# Sandy Seaside Credits: (CCO) Spring Spring: https://opengameart.org/content/sandy-seaside-2
# sound effects Credits: ViRiX Dreamcore (CC-BY 3.0): https://opengameart.org/content/ui-and-item-sound-effect-jingles-sample-2
# ui and othe effect sounds Credits: ViRiX Dreamcore (CC-BY 3.0) (David McKee) https://opengameart.org/content/ui-and-item-sounds-sample-1

# FONT
# jumpman font Credits: Pixel Sagas https://fontmeme.com/fonts/jumpman-font/
# zen dots Credits: Yoshimichi Ohira (SIL OPEN FONT LICENSE Version 1.1) https://fonts.google.com/specimen/Zen+Dots/license

# 3D Models
# Palm Tree Model Credits: abdulraffay1433 (CC) https://sketchfab.com/3d-models/palm-tree-e7677eb753ce4c16bc3c5dbaa8aefa27
# Snek, Rat  Model Credits: Quaternius (CC0) https://quaternius.com/packs/easyenemy.html

