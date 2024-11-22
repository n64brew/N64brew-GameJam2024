
ASSETS_LIST += \
	filesystem/rampage/0.sprite \
	filesystem/rampage/1.sprite \
	filesystem/rampage/2.sprite \
	filesystem/rampage/3.sprite \
	filesystem/rampage/4.sprite \
	filesystem/rampage/5.sprite \
	filesystem/rampage/destroy.sprite \
	filesystem/rampage/finish.sprite \
	filesystem/rampage/building.t3dm \
	filesystem/rampage/player.t3dm \
	filesystem/rampage/ground.t3dm \
	filesystem/rampage/tank0.t3dm \
	filesystem/rampage/house.t3dm \
	filesystem/rampage/building_2story.t3dm \
	filesystem/rampage/Jira_01.t3dm \
	filesystem/rampage/level.t3dm \
	filesystem/rampage/bullet.t3dm \
	filesystem/rampage/pointer.t3dm \
	filesystem/rampage/swing_effect.t3dm \
	filesystem/rampage/tank1_32x4.ci4.sprite \
	filesystem/rampage/tank1_64x4.ci4.sprite \
	filesystem/rampage/tank2_32x4.ci4.sprite \
	filesystem/rampage/tank2_64x4.ci4.sprite \
	filesystem/rampage/house_wall.ci4.sprite \
	filesystem/rampage/building1.ci4.sprite \
	filesystem/rampage/Jira32.sprite \
	filesystem/rampage/Jira_GS.i4.sprite \
	filesystem/rampage/pavement_1.sprite \
	filesystem/rampage/pavement_2.sprite \
	filesystem/rampage/pavement_3.sprite \
	filesystem/rampage/pavement_default.sprite \
	filesystem/rampage/road_bend.sprite \
	filesystem/rampage/road_crossing.sprite \
	filesystem/rampage/road_intersection.sprite \
	filesystem/rampage/road_straight.sprite \
	filesystem/rampage/road_tjunction.sprite \
	filesystem/rampage/road_warning1.sprite \
	filesystem/rampage/QuirkyRobot.font64 \
	filesystem/rampage/stompin.wav64 \
	filesystem/rampage/Collapse.wav64 \
	filesystem/rampage/countdown.wav64 \
	filesystem/rampage/hit.wav64 \
	filesystem/rampage/Roar1.wav64 \
	filesystem/rampage/RoarShort.wav64 \
	filesystem/rampage/startjingle.wav64

assets/rampage/%.glb: assets/rampage/%.blend tools/mesh_export.py
	@mkdir -p $(@D)
	$(BLENDER_4_0) $< --background --python tools/mesh_export.py -- $(<:%.blend=%.glb)

filesystem/rampage/QuirkyRobot.font64: MKFONT_FLAGS += --size 32