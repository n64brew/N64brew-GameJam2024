ASSETS_LIST += \
	filesystem/tohubohu/room.t3dm \
	filesystem/tohubohu/room_floor_tex.ci4.sprite \
	filesystem/tohubohu/room_wall_tex.ci4.sprite \
	filesystem/tohubohu/furniture.t3dm \
	filesystem/tohubohu/furniture_drawer_tex.ci4.sprite \
	filesystem/tohubohu/furniture_wood_tex.ci4.sprite \
	filesystem/tohubohu/vault.t3dm \
	filesystem/tohubohu/vault_tex.ci4.sprite \
	filesystem/tohubohu/vault_digits_tex.ci4.sprite \
	filesystem/tohubohu/vault_lock_tex.ci4.sprite \
	filesystem/tohubohu/player.t3dm \
	filesystem/tohubohu/key.t3dm \
	filesystem/tohubohu/key.wav64 \
	filesystem/tohubohu/rummage.wav64 \
	filesystem/tohubohu/open.wav64 \
	filesystem/tohubohu/attack.wav64 \
	filesystem/tohubohu/hurt.wav64 \
	filesystem/tohubohu/music.wav64 \
	filesystem/tohubohu/thickhead.font64

filesystem/tohubohu/thickhead.font64: MKFONT_FLAGS += --outline 1 --size 36

build/code/tohubohu/%.o: N64_CFLAGS += -fms-extensions