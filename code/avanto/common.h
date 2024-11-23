#define MAX_GROUND_CHANGES 4
// From main.c
#define NUM_SFX_CHANNELS 1
#define FIRST_SFX_CHANNEL (31-NUM_SFX_CHANNELS)

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

float get_ground_height(float z, struct ground *ground);
int get_next_sfx_channel();
void init_sfx();
void skeleton_init(struct skeleton *s,
    const T3DModel *model,
    size_t num_anims);
void skeleton_free(struct skeleton *s);
void entity_init(struct entity *e,
    const T3DModel *model,
    const T3DVec3 *scale,
    const T3DVec3 *rotation,
    const T3DVec3 *pos,
    T3DSkeleton *skeleton,
    T3DModelDrawConf draw_conf);
void entity_free(struct entity *e);
