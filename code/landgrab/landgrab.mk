ASSETS_LIST += \
	filesystem/landgrab/15yearsb.xm64 \
	filesystem/landgrab/phekkis-4_weeks_of_hysteria.xm64 \
	filesystem/landgrab/buzz.wav64 \
	filesystem/landgrab/click.wav64 \
	filesystem/landgrab/pop.wav64 \
	filesystem/landgrab/anita_semi_square.font64 \
	filesystem/landgrab/bbrick.font64 \
	filesystem/landgrab/background.ci8.sprite \
	filesystem/landgrab/cursor_p1.rgba16.sprite \
	filesystem/landgrab/cursor_p2.rgba16.sprite \
	filesystem/landgrab/cursor_p3.rgba16.sprite \
	filesystem/landgrab/cursor_p4.rgba16.sprite \
	filesystem/landgrab/joypad_a.rgba16.sprite \
	filesystem/landgrab/joypad_b.rgba16.sprite \
	filesystem/landgrab/joypad_c_down.rgba16.sprite \
	filesystem/landgrab/joypad_c_left.rgba16.sprite \
	filesystem/landgrab/joypad_c_right.rgba16.sprite \
	filesystem/landgrab/joypad_c_up.rgba16.sprite \
	filesystem/landgrab/joypad_d_down.rgba16.sprite \
	filesystem/landgrab/joypad_d_left.rgba16.sprite \
	filesystem/landgrab/joypad_d_right.rgba16.sprite \
	filesystem/landgrab/joypad_d_up.rgba16.sprite \
	filesystem/landgrab/joypad_dpad.rgba16.sprite \
	filesystem/landgrab/joypad_l.rgba16.sprite \
	filesystem/landgrab/joypad_r.rgba16.sprite \
	filesystem/landgrab/joypad_start.rgba16.sprite \
	filesystem/landgrab/joypad_stick.rgba16.sprite \
	filesystem/landgrab/joypad_z.rgba16.sprite \
	filesystem/landgrab/logo.ci8.sprite \
	filesystem/landgrab/star.ia8.sprite \
	filesystem/landgrab/x.ia8.sprite

filesystem/landgrab/anita_semi_square.font64: MKFONT_FLAGS += --outline 2 --size 16
filesystem/landgrab/bbrick.font64: MKFONT_FLAGS += --outline 2 --size 16
filesystem/landgrab/buzz.wav64: AUDIOCONV_FLAGS += --wav-mono
filesystem/landgrab/click.wav64: AUDIOCONV_FLAGS += --wav-mono
filesystem/landgrab/pop.wav64: AUDIOCONV_FLAGS += --wav-mono
