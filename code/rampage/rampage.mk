
ASSETS_LIST += \
	filesystem/rampage/0.sprite \
	filesystem/rampage/1.sprite \
	filesystem/rampage/2.sprite \
	filesystem/rampage/3.sprite \
	filesystem/rampage/4.sprite \
	filesystem/rampage/5.sprite \
	filesystem/rampage/score-p0.sprite \
	filesystem/rampage/score-p1.sprite \
	filesystem/rampage/score-p2.sprite \
	filesystem/rampage/score-p3.sprite \
	filesystem/rampage/winner-0.sprite \
	filesystem/rampage/winner-1.sprite \
	filesystem/rampage/winner-2.sprite \
	filesystem/rampage/winner-3.sprite \
	filesystem/rampage/winner-0-low.sprite \
	filesystem/rampage/winner-1-low.sprite \
	filesystem/rampage/winner-2-low.sprite \
	filesystem/rampage/winner-3-low.sprite \
	filesystem/rampage/destroy.sprite \
	filesystem/rampage/destroy-low.sprite \
	filesystem/rampage/tie.sprite \
	filesystem/rampage/finish.sprite \
	filesystem/rampage/finish-low.sprite \
	filesystem/rampage/spark_blob.i8.sprite \
	filesystem/rampage/player.t3dm \
	filesystem/rampage/ground.t3dm \
	filesystem/rampage/ground_cover.t3dm \
	filesystem/rampage/tank0.t3dm \
	filesystem/rampage/building_1story.t3dm \
	filesystem/rampage/building_2story.t3dm \
	filesystem/rampage/building_3story.t3dm \
	filesystem/rampage/billboards.ci4.sprite \
	filesystem/rampage/billboard0.t3dm \
	filesystem/rampage/billboard1.t3dm \
	filesystem/rampage/billboard2.t3dm \
	filesystem/rampage/billboard3.t3dm \
	filesystem/rampage/billboard4.t3dm \
	filesystem/rampage/billboard5.t3dm \
	filesystem/rampage/Jira_01.t3dm \
	filesystem/rampage/level.t3dm \
	filesystem/rampage/bullet.t3dm \
	filesystem/rampage/swing_effect.t3dm \
	filesystem/rampage/spark_effect.t3dm \
	filesystem/rampage/blockade.t3dm \
	filesystem/rampage/directionsign.t3dm \
	filesystem/rampage/streetlight.t3dm \
	filesystem/rampage/trafficlight.t3dm \
	filesystem/rampage/van.t3dm \
	filesystem/rampage/tank1_32x4.ci4.sprite \
	filesystem/rampage/tank1_64x4.ci4.sprite \
	filesystem/rampage/tank2_32x4.ci4.sprite \
	filesystem/rampage/tank2_64x4.ci4.sprite \
	filesystem/rampage/building_1story.ci4.sprite \
	filesystem/rampage/building_2story.ci4.sprite \
	filesystem/rampage/building_3story.ci4.sprite \
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
	filesystem/rampage/rubble.sprite \
	filesystem/rampage/canal.sprite \
	filesystem/rampage/walls.ci4.sprite \
	filesystem/rampage/props.sprite \
	filesystem/rampage/stompin.wav64 \
	filesystem/rampage/Collapse.wav64 \
	filesystem/rampage/countdown.wav64 \
	filesystem/rampage/hit.wav64 \
	filesystem/rampage/Roar1.wav64 \
	filesystem/rampage/RoarShort.wav64 \
	filesystem/rampage/startjingle.wav64 \
	filesystem/rampage/ground.layout

# these rules can help faciliate iterating assets without needing to manually export
# you just need to set the environment variable BLENDER_4_0 to the executable path of
# blender 4.0. It is important that it is 4.0 and not 4.1 onward

# assets/rampage/%.glb: assets/rampage/%.blend tools/mesh_export.py
# 	@mkdir -p $(@D)
# 	$(BLENDER_4_0) $< --background --python tools/mesh_export.py -- $(<:%.blend=%.glb)

# assets/rampage/ground.layout: assets/rampage/ground.blend tools/export_linked_objects.py
# 	@mkdir -p $(@D)
# 	$(BLENDER_4_0) $< --background --python tools/export_linked_objects.py -- $@

filesystem/rampage/ground.layout: assets/rampage/ground.layout
	cp $< $@