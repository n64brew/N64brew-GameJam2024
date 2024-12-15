#define MAX_GROUND_CHANGES 6
#define EPS 1e-6
#define TIMER_Y 220
#define HUD_HORIZONTAL_BORDER 26
#define HUD_VERTICAL_BORDER 26
#define HUD_INDIVIDUAL_H_SPACE ((320 - HUD_HORIZONTAL_BORDER*2)/4)
#define HUD_BAR_HEIGHT 16
#define HUD_BAR_Y_OFFSET 4
#define HUD_BAR_X_OFFSET 1
#define GRAVITY 240.f
#define MAX_PARTICLE_SOURCES 4
#define SCRIPT_NUM_SIGNALS 4
#define FADE_TIME 1.f
#define MITIGATE_FONT_BUG {rdpq_sync_pipe(); rdpq_sync_tile();}

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
  size_t current_anim;
  bool visible;
  float temperature;
  bool out;
};

struct ground_height_change {
  float start_z;
  float height;
  bool ramp_to_next;
};

struct ground {
  size_t num_changes;
  struct ground_height_change changes[MAX_GROUND_CHANGES];
};

struct scene {
  const char *bg_path;
  sprite_t *bg;
  float fov;
  struct camera starting_cam;
  void (*do_light)();
  struct ground ground;
};

struct script_action {
  int type;
  union {
    struct {
      float rot;
      float speed;
    };
    struct {
      T3DVec3 pos;
      T3DVec3 target;
      union {
        float travel_time;
        float walk_speed;
      };
    };
    bool playing;
    size_t anim;
    bool visibility;
    float time;
    struct {
      wav64_t *sfx;
      size_t channel;
    };
    struct {
      xm64player_t *xm64;
      size_t first_channel;
    };
    size_t signal;
    const struct camera *camera;
    void (*callback)();
  };
};

struct script_state {
  struct character *character;
  const struct script_action *action;
  float time;
};

struct subgame {
  void (*dynamic_loop_pre)(float);
  void (*dynamic_loop_render)(float);
  void (*dynamic_loop_post)(float);
  bool (*fixed_loop)(float);
  void (*cleanup)();
  void (*init)();
};

enum script_actions {
  ACTION_WAIT,
  ACTION_WALK_TO,
  ACTION_WARP_TO,
  ACTION_CLIMB_TO,
  ACTION_ROTATE_TO,
  ACTION_START_ANIM,
  ACTION_SET_VISIBILITY,
  ACTION_DO_WHOLE_ANIM,
  ACTION_PLAY_SFX,
  ACTION_START_XM64,
  ACTION_MOVE_CAMERA_TO,
  ACTION_WAIT_FOR_SIGNAL,
  ACTION_SEND_SIGNAL,
  ACTION_ANIM_SET_PLAYING,
  ACTION_ANIM_UPDATE_TO_TS,
  ACTION_CALLBACK,
  ACTION_END,
};

enum fonts {
  FONT_NORMAL = 1,
  FONT_TIMER,
  FONT_BANNER,
};

enum sw_styles {
  SW_NORMAL,
  SW_BANNER,
  SW_TIMER,
  SW_PLAYER1,
  SW_PLAYER2,
  SW_PLAYER3,
  SW_PLAYER4,
  SW_OUT,
  SW_SELECTED,
};

#define SW_NORMAL_S "^00"
#define SW_BANNER_S "^01"
#define SW_TIMER_S "^02"
#define SW_PLAYER1_S "^03"
#define SW_PLAYER2_S "^04"
#define SW_PLAYER3_S "^05"
#define SW_PLAYER4_S "^06"
#define SW_OUT_S "^07"
#define SW_SELECTED_S "^08"

enum player_anims {
  WALK,
  CLIMB,
  SIT,
  BEND,
  UNBEND,
  STAND_UP,
  PASS_OUT,
  SWIM,
  DANCE,
  NUM_PLAYER_ANIMS,
};

enum particle_type {
  UNDEFINED,
  STEAM,
  SNOW,
  SPLASH,
};

struct particle_meta {
  union {
    struct {
      int8_t cx;
      int8_t cz;
    };
    struct {
      float dir[2];
      int8_t h;
      int8_t d;
    };
  };
};

struct particle_source {
  T3DVec3 pos;
  T3DVec3 rot;
  T3DVec3 scale;
  int8_t particle_size;
  bool render;
  bool paused;

  union {
    struct {
      int8_t x_range;
      int8_t z_range;
      int height;
      union {
        float time_to_rise;
        float time_to_fall;
      };
      float movement_amplitude;
      float _y_move_error;
      size_t max_particles;
      float _to_spawn;
    };
    struct {
      int8_t min_dist;
      int8_t max_dist;
      int8_t min_height;
      int8_t max_height;
      float speed;
      float _time;
    };
  };

  struct particle_meta *_meta;
  T3DMat4FP *_transform;
  TPXParticle *_particles;
  size_t _num_allocated_particles;
  size_t _type;
};

float get_ground_height(float z, struct ground *ground);
float get_ground_angle(float z, struct ground *ground);
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
    T3DModelDrawConf *draw_conf);
void entity_free(struct entity *e);
void script_reset_signals();
bool script_update(struct script_state *state, float delta_time);
void draw_hud();
rspq_block_t *build_empty_hud_block();
void particle_source_init(struct particle_source *source,
    size_t num_particles,
    int type);
void particle_source_free(struct particle_source *source);
void particle_source_iterate(struct particle_source *source,
    float delta_time);
void particle_source_draw(const struct particle_source *source);
void particle_source_reset_steam(struct particle_source *source);
void particle_source_reset_splash(struct particle_source *source,
    size_t num_particles);
void particle_source_update_transform(struct particle_source *source);
float rand_float(float min, float max);
void draw_fade(float fade);
