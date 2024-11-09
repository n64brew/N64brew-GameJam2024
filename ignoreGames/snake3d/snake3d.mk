
ASSETS_LIST += \
	filesystem/snake3d/map.t3dm \
	filesystem/snake3d/shadow.t3dm \
	filesystem/snake3d/snake.t3dm \
	filesystem/snake3d/sand12.ci4.sprite \
	filesystem/snake3d/stone.ci4.sprite \
	filesystem/snake3d/shadow.i8.sprite \
	filesystem/snake3d/bottled_bubbles.xm64 \
	filesystem/snake3d/m6x11plus.font64

filesystem/snake3d/m6x11plus.font64: MKFONT_FLAGS += --outline 1 --size 36
