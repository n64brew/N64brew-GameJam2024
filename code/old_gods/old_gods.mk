ASSETS_LIST += \
	filesystem/old_gods/crate00.ci8.sprite \
	filesystem/old_gods/sand12.ci4.sprite \
	filesystem/old_gods/checker.sprite \
	filesystem/old_gods/shadow.i8.sprite \
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
	filesystem/old_gods/shadow.t3dm \
	filesystem/old_gods/map2.t3dm \
	filesystem/old_gods/snake.t3dm \
	filesystem/old_gods/rat.t3dm \
	filesystem/old_gods/torus.t3dm \
	filesystem/old_gods/bottled_bubbles.xm64 \

filesystem/old_gods/Jumpman_H1.font64: MKFONT_FLAGS += --outline -1 --size 64 --char-spacing 2
filesystem/old_gods/Jumpman_H2.font64: MKFONT_FLAGS += --outline -1 --size 32 --char-spacing 2
filesystem/old_gods/Jumpman_P.font64: MKFONT_FLAGS += --outline -1 --size 32 --char-spacing 2