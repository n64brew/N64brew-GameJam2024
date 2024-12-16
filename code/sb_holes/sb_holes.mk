
ASSETS_LIST += \
	filesystem/sb_holes/map.t3dm \
	filesystem/sb_holes/building.t3dm \
	filesystem/sb_holes/car.t3dm \
	filesystem/sb_holes/hole.t3dm \
	filesystem/sb_holes/hydrant.t3dm \
	filesystem/sb_holes/hole.ia8.sprite \
	filesystem/sb_holes/hydrant.ci8.sprite \
	filesystem/sb_holes/street.ci4.sprite \
	filesystem/sb_holes/window.ci4.sprite \
	filesystem/sb_holes/window1.ci4.sprite \
	filesystem/sb_holes/car.i4.sprite \
	filesystem/sb_holes/car1.i4.sprite \
	filesystem/sb_holes/car.wav64 \
	filesystem/sb_holes/hydrant.wav64 \
	filesystem/sb_holes/TheMorningAfter.xm64 \


filesystem/sb_holes/car.wav64: AUDIOCONV_FLAGS_FLAGS += --wav-compress 3 --wav-mono
filesystem/sb_holes/hydrant.wav64: AUDIOCONV_FLAGS_FLAGS += --wav-compress 3 --wav-mono
