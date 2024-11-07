ASSETS_LIST += \
	filesystem/rummage/room.t3dm \
	filesystem/rummage/room_floor_tex.ci4.sprite \
	filesystem/rummage/room_wall_tex.ci4.sprite \
	filesystem/rummage/furniture.t3dm \
	filesystem/rummage/furniture_drawer_tex.ci4.sprite \
	filesystem/rummage/furniture_wood_tex.ci4.sprite \
	filesystem/rummage/vault.t3dm \
	filesystem/rummage/vault_tex.ci4.sprite \
	filesystem/rummage/player.t3dm \
	filesystem/rummage/key.t3dm \
	filesystem/rummage/key.wav64 \
	filesystem/rummage/rummage.wav64 \
	filesystem/rummage/music.wav64 \
	filesystem/rummage/thickhead.font64

filesystem/rummage/thickhead.font64: MKFONT_FLAGS += --outline 1 --size 36

N64_CFLAGS += -fms-extensions