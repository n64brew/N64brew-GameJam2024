#define MAX_GROUND_CHANGES 4

struct entity {
  const T3DModel *model;
  T3DMat4FP *transform;
  T3DSkeleton *skeleton;
  rspq_block_t *display_block;
};

struct skeleton {
  T3DSkeleton skeleton;
  T3DAnim *anims;
  size_t num_anims;
};

struct camera {
  T3DVec3 pos;
  T3DVec3 target;
};

struct character {
  struct entity e;
  T3DVec3 pos;
  float rotation;
  float scale;
  struct skeleton s;
};

struct ground_height_change {
  float start_z;
  float height;
};

struct ground {
  size_t num_changes;
  struct ground_height_change changes[MAX_GROUND_CHANGES];
};

struct scene {
  const char *bg_path;
  const char *z_path;
  sprite_t *bg;
  sprite_t *z;
  float fov;
  struct camera starting_cam;
  void (*do_light)();
  struct ground ground;
};

enum ukko_anims {
  THROW,
};

enum fonts {
  FONT_DEBUG = 1,
};
