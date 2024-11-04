
ASSETS_LIST += \
	filesystem/rampage/building.t3dm \
	filesystem/rampage/player.t3dm

assets/rampage/%.glb: assets/rampage/%.blend tools/mesh_export.py
	@mkdir -p $(@D)
	$(BLENDER_4) $< --background --python tools/mesh_export.py -- $(<:%.blend=%.glb)