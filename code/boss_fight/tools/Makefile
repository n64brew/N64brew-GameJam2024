CXXFLAGS += -O3 -std=c++20 -I./src/lib -I$(T3D_INST)/tools/gltf_importer/src/
OBJDIR = build
SRCDIR = src

OBJ_SCENE = build/mainScene.o build/meshBVH.o
OBJ_COLL  = build/mainColl.o build/meshBVH.o

all: gltf_to_coll gltf_to_scene

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

gltf_to_coll: $(OBJ_COLL)
	$(CXX) $(CXXFLAGS) -o $@ $^ $ $(LINKFLAGS)

gltf_to_scene: $(OBJ_SCENE)
	$(CXX) $(CXXFLAGS) -o $@ $^ $ $(LINKFLAGS)

clean:
	rm -rf ./build ./gltf_to_coll ./gltf_to_scene
