SPECIAL_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
CUSTOM_GLTF_COLLISION := $(SPECIAL_DIR)"my_tools/gltf_collision_importer/gltf_collision"



ASSETS_LIST += \
	filesystem/snowmen/map.t3dm \
	filesystem/snowmen/shadow.t3dm \
	filesystem/snowmen/snake.t3dm \
	filesystem/snowmen/sand12.ci4.sprite \
	filesystem/snowmen/stone.ci4.sprite \
	filesystem/snowmen/shadow.i8.sprite \
	filesystem/snowmen/bottled_bubbles.xm64 \
	filesystem/snowmen/m6x11plus.font64 \
	filesystem/snowmen/p1.t3dm \
	filesystem/snowmen/BootTex.ci4.sprite \
	filesystem/snowmen/ChestTex_ci4.sprite \
	filesystem/snowmen/FaceTex.ci4.sprite \
	filesystem/snowmen/crate00.ci8.sprite \
	filesystem/snowmen/box.t3dm \
	filesystem/snowmen/snow4.sprite \
	filesystem/snowmen/walls1.sprite \
	filesystem/snowmen/yellow_small.sprite \
	filesystem/snowmen/blue_small.sprite \
	filesystem/snowmen/red_small.sprite \
	filesystem/snowmen/snow_small.sprite \
	filesystem/snowmen/star_small.sprite \
	filesystem/snowmen/Wood_small.sprite \
	filesystem/snowmen/hedge_ivy_small.sprite \
	filesystem/snowmen/tree.t3dm \
	filesystem/snowmen/SnowyMapTest6_4.t3dm \
	filesystem/snowmen/SnowyMapTest6_7.t3dm \
	filesystem/snowmen/SnowyMapTest6_4_Collision.t3dm \
	filesystem/snowmen/sphere-00smol.sprite \
	filesystem/snowmen/snowball_test_1.t3dm \
	filesystem/snowmen/sphere-17_small.sprite \
	filesystem/snowmen/hat.sprite \
	filesystem/snowmen/hat_blue.sprite \
	filesystem/snowmen/hat_green.sprite \
	filesystem/snowmen/hat_yellow.sprite \
	filesystem/snowmen/mitt.sprite \
	filesystem/snowmen/totalSnowmanTest_5.t3dm \
	filesystem/snowmen/deco_rock_1.t3dm \
	filesystem/snowmen/deco_hat.t3dm \
	filesystem/snowmen/deco_mitt.t3dm \
	filesystem/snowmen/deco_scarf.t3dm \
	filesystem/snowmen/deco_stick.t3dm \
	filesystem/snowmen/deco_carrot.t3dm \
	filesystem/snowmen/spawner_stones.t3dm \
	filesystem/snowmen/spawner_hat.t3dm \
	filesystem/snowmen/spawner_mitt.t3dm \
	filesystem/snowmen/spawner_scarf.t3dm \
	filesystem/snowmen/spawner_sticks.t3dm \
	filesystem/snowmen/spawner_carrot.t3dm \
	filesystem/snowmen/snowman_snowball0.t3dm \
	filesystem/snowmen/snowman_snowball1.t3dm \
	filesystem/snowmen/snowman_snowball2.t3dm \
	filesystem/snowmen/snowman_hat.t3dm \
	filesystem/snowmen/snowman_hat_blue.t3dm \
	filesystem/snowmen/snowman_hat_green.t3dm \
	filesystem/snowmen/snowman_hat_yellow.t3dm \
	filesystem/snowmen/snowman_carrot.t3dm \
	filesystem/snowmen/snowman_face.t3dm \
	filesystem/snowmen/snowman_scarf.t3dm \
	filesystem/snowmen/snowman_mitt.t3dm \
	filesystem/snowmen/snowman_stick.t3dm \
	filesystem/snowmen/snowman_stonestorso.t3dm \
	filesystem/snowmen/snowman_stonesbottom.t3dm \
	filesystem/snowmen/pickup_snowball.t3dm \
	filesystem/snowmen/snowman_face_pic_red.sprite \
	filesystem/snowmen/snowman_face_pic_blue.sprite \
	filesystem/snowmen/snowman_face_pic_yellow.sprite \
	filesystem/snowmen/snowman_face_pic_green.sprite \
	filesystem/snowmen/snowman_face_pic_red.t3dm \
	filesystem/snowmen/snowman_face_pic_blue.t3dm \
	filesystem/snowmen/snowman_face_pic_green.t3dm \
	filesystem/snowmen/snowman_face_pic_yellow.t3dm \
	filesystem/snowmen/catherine_blue.t3dm \
	filesystem/snowmen/catherine_green.t3dm \
	filesystem/snowmen/catherine_yellow.t3dm \
	filesystem/snowmen/base_star.t3dm \
	filesystem/snowmen/christmas_day.xm64 \
	filesystem/snowmen/Punch__007.wav64 \
	filesystem/snowmen/swing.wav64 \
	filesystem/snowmen/chainmail1.wav64 \
	filesystem/snowmen/arrow.t3dm \
	filesystem/snowmen/SnowyMapTest6_4_Collision.col

filesystem/snowmen/%.col: assets/snowmen/%.col
	@mkdir -p $(dir $@)
	@echo "    [CUSTOM_COLLISION] $@"
	cp "$<" $@

# Reenable this after we find out how to build a tool as part of the pipeline
# filesystem/snowmen/%.col: assets/snowmen/%.glb
# 	@echo "    [CUSTOM_COLLISION] $@"
# 	$(CUSTOM_GLTF_COLLISION) "$<" $@
# 	$(N64_BINDIR)/mkasset -c 2 -o $(dir $@) $@

filesystem/snowmen/%.t3dm: assets/snowmen/%.glb
	@mkdir -p $(dir $@)
	@echo "    [T3D-MODEL] $@"
	$(T3D_GLTF_TO_3D) "$<" $@
	$(N64_BINDIR)/mkasset -c 2 -o $(dir $@) $@

filesystem/snowmen/m6x11plus.font64: MKFONT_FLAGS += --outline 1 --size 36



