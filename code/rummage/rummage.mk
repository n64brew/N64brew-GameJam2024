ASSETS_LIST += \
	filesystem/rummage/room.t3dm \
	filesystem/rummage/furniture.t3dm \
	filesystem/rummage/furniture_tex.ci4.sprite \
	filesystem/rummage/vault.t3dm \
	filesystem/rummage/player.t3dm \
	filesystem/rummage/key.wav64 \
	filesystem/rummage/rummage.wav64 \
	filesystem/rummage/music.wav64 \
	filesystem/rummage/thickhead.font64

filesystem/rummage/thickhead.font64: MKFONT_FLAGS += --outline 1 --size 36

N64_CFLAGS += -fms-extensions