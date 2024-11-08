
ASSETS_LIST += \
	filesystem/rampage/building.t3dm \
	filesystem/rampage/player.t3dm \
	filesystem/rampage/ground.t3dm \
	filesystem/rampage/tank0.t3dm \
	filesystem/rampage/tank1_32x4.ci4.sprite \
	filesystem/rampage/tank1_64x4.ci4.sprite \
	filesystem/rampage/tank2_32x4.ci4.sprite \
	filesystem/rampage/tank2_64x4.ci4.sprite

assets/rampage/%.glb: assets/rampage/%.blend tools/mesh_export.py
	@mkdir -p $(@D)
	$(BLENDER_4) $< --background --python tools/mesh_export.py -- $(<:%.blend=%.glb)