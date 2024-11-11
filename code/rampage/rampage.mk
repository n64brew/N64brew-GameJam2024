
ASSETS_LIST += \
	filesystem/rampage/building.t3dm \
	filesystem/rampage/player.t3dm \
	filesystem/rampage/ground.t3dm \
	filesystem/rampage/tank0.t3dm \
	filesystem/rampage/house.t3dm \
	filesystem/rampage/Jira_01.t3dm \
	filesystem/rampage/level.t3dm \
	filesystem/rampage/tank1_32x4.ci4.sprite \
	filesystem/rampage/tank1_64x4.ci4.sprite \
	filesystem/rampage/tank2_32x4.ci4.sprite \
	filesystem/rampage/tank2_64x4.ci4.sprite \
	filesystem/rampage/house_wall.ci4.sprite \
	filesystem/rampage/Jira32.sprite \
	filesystem/rampage/pavement_1.sprite \
	filesystem/rampage/pavement_2.sprite \
	filesystem/rampage/pavement_3.sprite \
	filesystem/rampage/pavement_default.sprite \
	filesystem/rampage/road_bend.sprite \
	filesystem/rampage/road_crossing.sprite \
	filesystem/rampage/road_intersection.sprite \
	filesystem/rampage/road_straight.sprite \
	filesystem/rampage/road_tjunction.sprite \
	filesystem/rampage/road_warning1.sprite

assets/rampage/%.glb: assets/rampage/%.blend tools/mesh_export.py
	@mkdir -p $(@D)
	$(BLENDER_4) $< --background --python tools/mesh_export.py -- $(<:%.blend=%.glb)