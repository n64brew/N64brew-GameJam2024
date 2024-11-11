#include "game.h"
#include "astar.h"
#include "../../core.h"
#include "../../minigame.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

#define ENABLE_WIREFRAME 1

#if ENABLE_WIREFRAME
#include "draw.h"
#include <GL/gl.h>
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.h"
#pragma GCC diagnostic pop


#define T3D_MODEL_SCALE 64
#define COLLISION_CORRECTIVE_FACTOR 1.3f
#define MAP_REDUCTION_FACTOR 4
#define PATH_LENGTH 10
#define NO_PATH 9999
#define WAYPOINT_DELAY 60
#define WAYPOINT_DISTANCE_THRESHOLD 5.0f
#define ACTION_DISTANCE_THRESHOLD 10.0f
#define ACTION_TIME 1.0f
#define FURNITURES_COUNT 9
#define VAULTS_COUNT 3

bool playing = false;

typedef struct actor_t {
    T3DModel* model;
    T3DMat4FP* mat_fp;
    rspq_block_t* dpl;
    T3DVec3 scale;
    T3DVec3 rotation;
    T3DVec3 position;
    T3DVec3 direction;
    float w;
    float h;
    float max_y;
    bool hidden;
} actor_t;

typedef struct usable_actor_t {
    actor_t;
    bool rotated;
    c2AABB bbox;
    // Usage zone
    float zone_w;
    float zone_h;
    c2AABB zone_bbox;
} usable_actor_t;

typedef struct {
    usable_actor_t;
    bool has_key;
} furniture_t;

typedef struct {
    usable_actor_t;
    bool is_target;
} vault_t;

typedef enum {
    IDLE = 0,
    MOVING_TO_FURNITURE,
    RUMMAGING,
    MOVING_TO_PLAYER,
    ATTACKING,
    MOVING_TO_VAULT,
    OPENING_VAULT
} ai_state_t;

typedef struct {
    actor_t;
    PlyNum plynum;
    color_t color;
    T3DSkeleton skel;
    T3DAnim anim_idle;
    T3DAnim anim_walk;
    float speed;
    bool had_key;
    bool has_key;
    bool has_won;
    float action_playing_time;
    // AI players
    int idle_delay;
    ai_state_t state;
    bool furnitures[FURNITURES_COUNT];
    bool vaults[VAULTS_COUNT];
    int target_idx;
    T3DVec3 target;
    T3DVec3 path[PATH_LENGTH];  // Next points in path
    int path_pos;
    int path_delay;
} player_t;


// Room
actor_t room;

// Furnitures
furniture_t furnitures[FURNITURES_COUNT];

// Vaults
vault_t vaults[VAULTS_COUNT];

// Players
player_t players[MAXPLAYERS];

// Key
actor_t key;


// Sound FX FIXME increase sfx volume to match countdown sfx
wav64_t sfx_rummage;
wav64_t sfx_key;


c2AABB usable_actor_bounding_box(usable_actor_t* actor) {
    c2AABB bb;
    float w = actor->rotated ? actor->h : actor->w;
    float h = actor->rotated ? actor->w : actor->h;
    bb.min = c2V(actor->position.v[0] - w/2.0f, actor->position.v[2] - h/2.0f);
    bb.max = c2V(actor->position.v[0] + w/2.0f, actor->position.v[2] + h/2.0f);
    return bb;
}

c2AABB usable_zone_bounding_box(usable_actor_t* actor) {
    c2AABB bb;
    T3DVec3 zone;
    t3d_vec3_scale(&zone, &actor->direction, actor->h/2.0f + actor->zone_h/2.0f);
    t3d_vec3_add(&zone, &zone, &actor->position);
    float w = actor->rotated ? actor->zone_h : actor->zone_w;
    float h = actor->rotated ? actor->zone_w : actor->zone_h;
    bb.min = c2V(zone.v[0] - w/2.0f, zone.v[2] - h/2.0f);
    bb.max = c2V(zone.v[0] + w/2.0f, zone.v[2] + h/2.0f);
    return bb;
}

c2AABB actor_bounding_box(actor_t* actor) {
    c2AABB bb;
    float w = actor->w;
    float h = actor->h;
    bb.min = c2V(actor->position.v[0] - w/2.0f, actor->position.v[2] - h/2.0f);
    bb.max = c2V(actor->position.v[0] + w/2.0f, actor->position.v[2] + h/2.0f);
    return bb;
}


// Path finding

int map_width;
int map_height;
T3DVec3 origin;
char* map;

inline static void to_pathmap_coords(T3DVec3 *res, const T3DVec3 *a) {
    t3d_vec3_scale(res, a, 1.0f/MAP_REDUCTION_FACTOR);
    t3d_vec3_diff(res, res, &origin);
}

inline static void from_pathmap_coords(T3DVec3 *res, const T3DVec3 *a) {
    t3d_vec3_add(res, a, &origin);
    t3d_vec3_scale(res, res, MAP_REDUCTION_FACTOR);
}

void update_obstacles() {
    // Precompute obstacles in map coordinates (is_walkable takes 150-400 cyces vs 4000+ when computing on-the-fly)
    for (int i=0; i<FURNITURES_COUNT; i++) {
        T3DVec3 furniture_min = (T3DVec3){{furnitures[i].bbox.min.x, 0, furnitures[i].bbox.min.y}};
        to_pathmap_coords(&furniture_min, &furniture_min);
        T3DVec3 furniture_max = (T3DVec3){{furnitures[i].bbox.max.x, 0, furnitures[i].bbox.max.y}};
        to_pathmap_coords(&furniture_max, &furniture_max);
        for (int x=furniture_min.v[0]+1; x<furniture_max.v[0]; x++) {
            for (int y=furniture_min.v[2]+1; y<furniture_max.v[2]; y++) {
                if (x >= 0 && x < map_width && y >= 0 && y < map_height) {
                    *(map+y*map_width+x) = 1;
                }
            }
        }
    }
}

bool is_walkable(cell_t cell) {
    bool walkable = (cell.x >= 0 && cell.x < map_width && cell.y >= 0 && cell.y < map_height);
    if (walkable) {
        return *(map+cell.y*map_width+cell.x) != 1;
    }
    return walkable;
}

int visited = 0;    // FIXME DEBUG ONLY
void add_neighbours(node_list_t* list, cell_t cell) {
    visited++;
    for (int x=cell.x-1; x<=cell.x+1; x++) {
        for (int y=cell.y-1; y<=cell.y+1; y++) {
            if ((x != cell.x || y != cell.y) && is_walkable((cell_t){x, y})) {
                float cost = (x == cell.x || y == cell.y) ? 1 : 1.414;
                add_neighbour(list, (cell_t){x, y}, cost);
            }
        }
    }
}

float heuristic(cell_t from, cell_t to) {
    // Manhattan distance FIXME use diagonal/octile distance? euclidian distance?
    return (fabs(from.x - to.x) + fabs(from.y - to.y));
}



// FIXME DON'T GET STUCK WHEN THERE IS NO PATH !!!
// FIXME (fix find_path() ??? issue with empty node maybe ?)
// FIXME implement early exit / depth limit? --> ALWAYS LIMIT !!!
void update_path(PlyNum i) {
    // Clear path
    for (int j=0; j<PATH_LENGTH; j++) {
        players[i].path[j].v[0] = NO_PATH;
        players[i].path[j].v[2] = NO_PATH;
    }
    players[i].path_pos = 0;
    // Find new path to target
    T3DVec3 start;
    to_pathmap_coords(&start, &players[i].position);
    T3DVec3 target;
    to_pathmap_coords(&target, &players[i].target);
    cell_t start_node = {(int)start.v[0], (int)start.v[2]};
    cell_t target_node = {(int)target.v[0], (int)target.v[2]};
    //debugf("find_path(%d %d, %d %d)\n", start_node.x, start_node.y, target_node.x, target_node.y);
    visited = 0;
    path_t* path = find_path(start_node, target_node);
    if (get_path_count(path) > 1) {
        //debugf("path points count: %d\n", get_path_count(path));
        for (int j=0; j<get_path_count(path); j++) {
            if (players[i].path_pos >= PATH_LENGTH-1)   break;
            //debugf("getting point #%d\n", j);
            cell_t* node = get_path_cell(path, j);
            //debugf("add point: %d %d\n", node->x, node->y);
            players[i].path[players[i].path_pos].v[0] = node->x;
            players[i].path[players[i].path_pos].v[2] = node->y;
            players[i].path_pos++;
        }
        players[i].path_pos = 0;
    } else {
        //debugf("No path from %d %d to %d %d\n", start_node.x, start_node.y, target_node.x, target_node.y);
    }
    //debugf("visited: %d\n", visited);
    free_path(path);
}

bool has_waypoints(PlyNum i) {
    return players[i].path[players[i].path_pos].v[0] != NO_PATH;
}

bool follow_path(PlyNum i) {
    T3DVec3* next = &players[i].path[players[i].path_pos];
    if (next->v[0] != NO_PATH) {
        // Follow path
        // Move towards next point in path
        // TODO Change target when stuck? (may be blocked by a furniture or a player)
        T3DVec3 next_point = (T3DVec3){{next->v[0], 0, next->v[2]}};
        //debugf("next_point: %f %f\n", next_point.v[0], next_point.v[2]);
        from_pathmap_coords(&next_point, &next_point);
        //debugf("next_point_converted: %f %f\n", next_point.v[0], next_point.v[2]);
        T3DVec3 newDir;
        t3d_vec3_diff(&newDir, &next_point, &players[i].position);
        //debugf("AI NEW DIR LENGTH: %f\n", t3d_vec3_len2(&newDir));
        float speed = sqrtf(t3d_vec3_len2(&newDir));
        if (speed > 4.8) {  // FIXME AI speed limit ??
            speed = 4.8;
        }
        // Smooth movements and stop
        if(speed > 0.15f) {
            newDir.v[0] /= speed;
            newDir.v[2] /= speed;
            players[i].direction = newDir;
            float newAngle = -atan2f(players[i].direction.v[0], players[i].direction.v[2]);
            players[i].rotation.v[1] = t3d_lerp_angle(players[i].rotation.v[1], newAngle, 0.5f);
            players[i].speed = t3d_lerp(players[i].speed, speed * 0.3f, 0.15f);
        } else {
            players[i].speed *= 0.64f;
        }
        // Move player
        players[i].position.v[0] += players[i].direction.v[0] * players[i].speed;
        players[i].position.v[2] += players[i].direction.v[2] * players[i].speed;

        players[i].path_delay--;

        // FIXME after collision resolution?
        if (t3d_vec3_distance(&players[i].position, &next_point) < WAYPOINT_DISTANCE_THRESHOLD) {
            //debugf("reached waypoint #%d\n", players[i].path_pos);
            players[i].path_pos++;
            players[i].path_delay = WAYPOINT_DELAY;
        }

        if (players[i].path_delay < 0) {
            return false;
        }
    }
    
    return has_waypoints(i);
}



bool can_rummage(int i, int j) {
    c2AABB player_i = actor_bounding_box((actor_t*)&players[i]);
    return c2AABBtoAABB(player_i, furnitures[j].zone_bbox);
}

bool rummage(int i, int j) {
    if (can_rummage(i, j)) {
        debugf("Player #%d rummaging through furniture #%d!\n", i, j);
        wav64_play(&sfx_rummage, 31);
        players[i].furnitures[players[i].target_idx] = true;
        players[i].action_playing_time = ACTION_TIME;
        if (furnitures[j].has_key) {
            debugf("Player #%d found key in furniture #%d!\n", i, j);
            players[i].has_key = true;
            furnitures[j].has_key = false;
            return true;
        }
    }
    return false;
}

PlyNum leader() {
    for (int i=0; i<MAXPLAYERS; i++) {
        if (players[i].has_key) {
            return players[i].plynum;
        }
    }
    return MAXPLAYERS;
}

bool can_open(int i, int j) {
    c2AABB player_i = actor_bounding_box((actor_t*)&players[i]);
    return c2AABBtoAABB(player_i, vaults[j].zone_bbox);
}

bool open(int i, int j) {
    if (players[i].has_key && can_open(i, j)) {
        debugf("Player #%d trying open vault #%d!\n", i, j);
        // TODO play sfx?
        players[i].vaults[players[i].target_idx] = true;
        players[i].action_playing_time = ACTION_TIME;   // FIXME Player should not be able to move while trying to open the vault...
        if (vaults[j].is_target) {
            debugf("Player #%d opened the vault #%d!\n", i, j);
            players[i].has_won = true;
            return true;
        }
    }
    return false;
}

void reset_idle_delay(int i) {
    players[i].idle_delay = (2-core_get_aidifficulty())*5 + rand()%((3-core_get_aidifficulty())*3);
}



void game_init()
{
    // Init room
    room.scale = (T3DVec3){{1, 1, 1}};
    room.rotation = (T3DVec3){{0, 0, 0}};
    room.position = (T3DVec3){{0, 0, 0}};
    room.w = 4.8 * T3D_MODEL_SCALE;
    room.h = 4.8 * T3D_MODEL_SCALE;
    room.max_y = 1.81f * T3D_MODEL_SCALE;
    room.model = t3d_model_load("rom:/rummage/room.t3dm");
    room.mat_fp = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_from_srt_euler(room.mat_fp, room.scale.v, room.rotation.v, room.position.v);
    rspq_block_begin();
        t3d_matrix_push(room.mat_fp);
        t3d_model_draw(room.model);
        t3d_matrix_pop(1);
    room.dpl = rspq_block_end();
    room.hidden = false;

    // Place furnitures
    int hideout = rand() % FURNITURES_COUNT;
    debugf("Key is in furniture #%d!\n", hideout);
    T3DModel* furniture_model = t3d_model_load("rom:/rummage/furniture.t3dm");
    for (int i=0; i<FURNITURES_COUNT; i++) {
        furnitures[i].w = 0.92f * T3D_MODEL_SCALE;
        furnitures[i].h = 0.42f * T3D_MODEL_SCALE;
        furnitures[i].max_y = 0.70f * T3D_MODEL_SCALE;
        furnitures[i].zone_w = furnitures[i].w/2.0f;
        furnitures[i].zone_h = furnitures[i].h/4.0f;
        furnitures[i].scale = (T3DVec3){{1, 1, 1}};
        int rotated = rand() % 3;
        furnitures[i].rotated = rotated % 2;
        furnitures[i].rotation = (T3DVec3){{0, rotated * M_PI/2, 0}};
        furnitures[i].position = (T3DVec3){{ ((i%3)-1)*((room.w-furnitures[i].w-50)/2.0f), 0, (((i/3)%3)-1)*((room.h-furnitures[i].h-50)/2.0f) }};
        furnitures[i].direction = (T3DVec3){{furnitures[i].rotated ? rotated-2 : 0, 0, furnitures[i].rotated ? 0 : 1-rotated}};
        furnitures[i].bbox = usable_actor_bounding_box((usable_actor_t*)&furnitures[i]);
        furnitures[i].zone_bbox = usable_zone_bounding_box((usable_actor_t*)&furnitures[i]);
        furnitures[i].model = furniture_model;
        furnitures[i].mat_fp = malloc_uncached(sizeof(T3DMat4FP));
        t3d_mat4fp_from_srt_euler(furnitures[i].mat_fp, furnitures[i].scale.v, furnitures[i].rotation.v, furnitures[i].position.v);
        rspq_block_begin();
            t3d_matrix_push(furnitures[i].mat_fp);
            t3d_model_draw(furnitures[i].model);
            t3d_matrix_pop(1);
        furnitures[i].dpl = rspq_block_end();
        furnitures[i].has_key = (i == hideout);
        furnitures[i].hidden = false;
    }

    // Place vaults
    int target = rand() % VAULTS_COUNT;
    debugf("Target is vault #%d!\n", target);
    T3DModel* vault_model = t3d_model_load("rom:/rummage/vault.t3dm");
    for (int i=0; i<VAULTS_COUNT; i++) {
        vaults[i].w = 1.09f * T3D_MODEL_SCALE;
        vaults[i].h = 0.11f * T3D_MODEL_SCALE;
        vaults[i].max_y = 1.50f * T3D_MODEL_SCALE;
        vaults[i].zone_w = vaults[i].w/1.5f;
        vaults[i].zone_h = vaults[i].h/2.0f;
        vaults[i].scale = (T3DVec3){{1, 1, 1}};
        vaults[i].rotated = (i-1) % 2;
        vaults[i].rotation = (T3DVec3){{0, (i-1)*M_PI/2, 0}};
        vaults[i].position = (T3DVec3){{ (i-1)*(room.w-vaults[i].h)/2.0f, 0, -1*(room.h-vaults[i].h)/2.0f*(i%2) }};
        vaults[i].direction = (T3DVec3){{1-i, 0, i%2}};
        vaults[i].bbox = usable_actor_bounding_box((usable_actor_t*)&vaults[i]);
        vaults[i].zone_bbox = usable_zone_bounding_box((usable_actor_t*)&vaults[i]);
        vaults[i].model = vault_model;
        vaults[i].mat_fp = malloc_uncached(sizeof(T3DMat4FP));
        t3d_mat4fp_from_srt_euler(vaults[i].mat_fp, vaults[i].scale.v, vaults[i].rotation.v, vaults[i].position.v);
        rspq_block_begin();
            t3d_matrix_push(vaults[i].mat_fp);
            t3d_model_draw(vaults[i].model);
            t3d_matrix_pop(1);
        vaults[i].dpl = rspq_block_end();
        vaults[i].is_target = (i == target);
        vaults[i].hidden = false;
    }

    // Place players
    uint32_t playercount = core_get_playercount();
    const color_t colors[] = {
        PLAYERCOLOR_1,
        PLAYERCOLOR_2,
        PLAYERCOLOR_3,
        PLAYERCOLOR_4
    };
    T3DModel* player_model = t3d_model_load("rom:/rummage/player.t3dm");
    for (int i=0; i<MAXPLAYERS; i++) {
        players[i].w = 1.42f * 0.2f * T3D_MODEL_SCALE;
        players[i].h = 1.42f * 0.2f * T3D_MODEL_SCALE;
        players[i].max_y = 3.60f * 0.2f * T3D_MODEL_SCALE;
        players[i].scale = (T3DVec3){{0.2f, 0.2f, 0.2f}};
        players[i].rotation = (T3DVec3){{0, (i-1)*M_PI/2, 0}};
        players[i].position = (T3DVec3){{ ((i-1)%2)*50, 0, ((i-2)%2)*50 }};
        players[i].direction = (T3DVec3){{0, 0, 0}};
        players[i].model = player_model;
        players[i].mat_fp = malloc_uncached(sizeof(T3DMat4FP));
        players[i].skel = t3d_skeleton_create(players[i].model);
        players[i].anim_idle = t3d_anim_create(players[i].model, "Action_Base");
        t3d_anim_attach(&players[i].anim_idle, &players[i].skel);
        players[i].anim_walk = t3d_anim_create(players[i].model, "Action_Marche");
        t3d_anim_attach(&players[i].anim_walk, &players[i].skel);
        players[i].color = colors[i];
        t3d_mat4fp_from_srt_euler(players[i].mat_fp, players[i].scale.v, players[i].rotation.v, players[i].position.v);
        rspq_block_begin();
            t3d_matrix_push(players[i].mat_fp);
            rdpq_set_prim_color(players[i].color);
            t3d_model_draw_skinned(players[i].model, &players[i].skel);
            t3d_matrix_pop(1);
        players[i].dpl = rspq_block_end();
        players[i].plynum = i;
        players[i].speed = 0.0f;
        players[i].had_key = false;
        players[i].has_key = false;
        players[i].has_won = false;
        players[i].action_playing_time = 0;
        players[i].hidden = false;
        // AI player
        if (i >= playercount) {
            reset_idle_delay(i);
            players[i].state = IDLE;
            memset(&players[i].furnitures, 0, sizeof(bool) * FURNITURES_COUNT);
            memset(&players[i].vaults, 0, sizeof(bool) * FURNITURES_COUNT);
            players[i].target_idx = -1;
            players[i].target = (T3DVec3){{NO_PATH, 0, NO_PATH}};
            for (int j=0; j<PATH_LENGTH; j++) {
                players[i].path[j].v[0] = NO_PATH;
                players[i].path[j].v[1] = 0;
                players[i].path[j].v[2] = NO_PATH;
            }
            players[i].path_pos = 0;
            players[i].path_delay = WAYPOINT_DELAY;
        }
    }

    // Init key
    key.scale = (T3DVec3){{1, 1, 1}};
    key.rotation = (T3DVec3){{0, 0, 0}};
    key.position = (T3DVec3){{0, 0, 0}};
    key.w = 0.4f * T3D_MODEL_SCALE;
    key.h = 0.4f * T3D_MODEL_SCALE;
    key.max_y = 0.42f * T3D_MODEL_SCALE;
    key.model = t3d_model_load("rom:/rummage/key.t3dm");
    key.mat_fp = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_from_srt_euler(key.mat_fp, key.scale.v, key.rotation.v, key.position.v);
    rspq_block_begin();
        t3d_matrix_push(key.mat_fp);
        t3d_model_draw(key.model);
        t3d_matrix_pop(1);
    key.dpl = rspq_block_end();
    key.hidden = true;

    // Sound FX
    wav64_open(&sfx_rummage, "rom:/rummage/rummage.wav64");
    wav64_open(&sfx_key, "rom:/rummage/key.wav64");

    // Init pathfinder
    map_width = room.w/MAP_REDUCTION_FACTOR;
    map_height = room.h/MAP_REDUCTION_FACTOR;
    origin = (T3DVec3){{-map_width/2.0f, 0, -map_height/2.0f}};
    //debugf("Allocate pathfinder obstacle map (%d bytes)\n", sizeof(char) * map_width * map_height);
    map = calloc(1, sizeof(char) * map_width * map_height);
    update_obstacles();
}


void game_logic(float deltatime)
{
    if (is_playing()) {
        // Player controls
        uint32_t playercount = core_get_playercount();
        for (size_t i = 0; i < MAXPLAYERS; i++) {
            if (players[i].action_playing_time > deltatime) {
                players[i].action_playing_time -= deltatime;
                continue;
            } else {
                players[i].action_playing_time = 0;
                if (players[i].has_key && !players[i].had_key) {
                    wav64_play(&sfx_key, 31);
                    players[i].had_key = true;
                    key.hidden = false;
                }
            }
            if (i < playercount) {  // Human player
                joypad_port_t port = core_get_playercontroller(i);
                joypad_inputs_t joypad = joypad_get_inputs(port);
                // Exit minigame by pressing start
                if (joypad.btn.start) {
                    minigame_end();
                }
                // Player actions: rummage, open vault, grab other player
                if(joypad.btn.a || joypad.btn.b) {  // TODO use new key presses only ?
                    // If the player is close to a furniture, search for the key
                    for (int j=0; j<FURNITURES_COUNT; j++) {
                        rummage(i, j);
                    }
                    if (players[i].has_key) {
                        for (int j=0; j<VAULTS_COUNT; j++) {
                            open(i, j);
                        }
                    }
                }
                T3DVec3 newDir = {0};
                newDir.v[0] = (float)joypad.stick_x * 0.10f;
                newDir.v[2] = -(float)joypad.stick_y * 0.10f;
                float speed = sqrtf(t3d_vec3_len2(&newDir));
                // Smooth movements and stop
                if(speed > 0.15f) {
                    newDir.v[0] /= speed;
                    newDir.v[2] /= speed;
                    players[i].direction = newDir;
                    float newAngle = -atan2f(players[i].direction.v[0], players[i].direction.v[2]);
                    players[i].rotation.v[1] = t3d_lerp_angle(players[i].rotation.v[1], newAngle, 0.5f);
                    players[i].speed = t3d_lerp(players[i].speed, speed * 0.3f, 0.15f);
                } else {
                    players[i].speed *= 0.64f;
                }
                // Move player
                players[i].position.v[0] += players[i].direction.v[0] * players[i].speed;
                players[i].position.v[2] += players[i].direction.v[2] * players[i].speed;
            } else {    // API Player
                switch(players[i].state) {
                    case IDLE:
                    {
                        if (players[i].idle_delay > 0) {
                            players[i].idle_delay--;
                            break;
                        }

                        // Find a new target and path
                        debugf("AI Player #%d needs a new path\n", i);

                        // TODO Adjust difficulty
                        // - by decreasing reaction time
                        // - by increasing frequency of pathfinder (store fewer waypoints), especially when chasing player?

                        ai_state_t next_state = IDLE;
                        if (leader() != MAXPLAYERS) {
                            if (players[i].has_key) {
                                // We have the key, go to unvisited vault
                                int target_idx;
                                do {
                                    target_idx = rand() % VAULTS_COUNT;
                                } while (players[i].vaults[target_idx]);
                                players[i].target_idx = target_idx;
                                // Go to a point in front of the vault
                                // TODO Center of usable zone
                                t3d_vec3_scale(&players[i].target, &vaults[target_idx].direction, vaults[target_idx].h/2.0f + players[i].h/2.0f);
                                t3d_vec3_add(&players[i].target, &players[i].target, &vaults[target_idx].position);
                                debugf("Player #%d now targeting vault #%d at coords: %f %f\n", i, target_idx, players[i].target.v[0], players[i].target.v[2]);
                                next_state = MOVING_TO_VAULT;
                            } else {
                                // Another player has the key, go after them!
                                int target_idx = leader();
                                players[i].target_idx = target_idx;
                                // FIXME Go to a (REACHABLE) point in front of the player
                                //t3d_vec3_scale(&players[i].target, &players[target_idx].direction, players[target_idx].h/2.0f + players[i].h/2.0f);
                                //t3d_vec3_add(&players[i].target, &players[i].target, &players[target_idx].position);
                                players[i].target.v[0] = players[target_idx].position.v[0];
                                players[i].target.v[1] = players[target_idx].position.v[1];
                                players[i].target.v[2] = players[target_idx].position.v[2];
                                debugf("Player #%d now targeting player #%d at coords: %f %f\n", i, target_idx, players[i].target.v[0], players[i].target.v[2]);
                                next_state = MOVING_TO_PLAYER;
                            }
                        } else {
                            // The key has not been found, go to unvisited furniture
                            int target_idx;
                            do {
                                target_idx = rand() % FURNITURES_COUNT;
                            } while (players[i].furnitures[target_idx]);
                            players[i].target_idx = target_idx;
                            // Go to a point in front of the furniture
                            // TODO Center of usable zone
                            t3d_vec3_scale(&players[i].target, &furnitures[target_idx].direction, furnitures[target_idx].h/2.0f + players[i].h/2.0f);
                            t3d_vec3_add(&players[i].target, &players[i].target, &furnitures[target_idx].position);
                            debugf("Player #%d now targeting furniture #%d at coords: %f %f\n", i, target_idx, players[i].target.v[0], players[i].target.v[2]);
                            next_state = MOVING_TO_FURNITURE;
                        }

                        update_path(i);
                        if (has_waypoints(i)) {
                            reset_idle_delay(i);
                            players[i].state = next_state;
                        }
                        break;
                    }
                    case MOVING_TO_FURNITURE:
                    {
                        // FIXME Don't get stuck!
                        // Move towards next waypoint
                        if (!follow_path(i)) {
                            // No more waypoint
                            if (players[i].path_delay < 0) {
                                // TODO Abandon path ? move back?
                                //reset_idle_delay(i);
                                players[i].state = IDLE;
                            } else {
                                if (can_rummage(i, players[i].target_idx)) {
                                    // We have reached the target: search for key
                                    players[i].state = RUMMAGING;
                                } else {
                                    // We haven't reached the target, get more waypoints
                                    update_path(i);
                                }
                            }
                        }
                        break;
                    }
                    case RUMMAGING:
                    {
                        rummage(i, players[i].target_idx);
                        reset_idle_delay(i);
                        players[i].state = IDLE;
                        break;
                    }
                    case MOVING_TO_PLAYER:
                    {
                        // FIXME Don't get stuck!
                        // Move towards next waypoint
                        if (!follow_path(i)) {
                            // No more waypoint
                            if (players[i].path_delay < 0) {
                                // TODO Abandon path ? move back?
                                //reset_idle_delay(i);
                                players[i].state = IDLE;
                            } else {
                                if (t3d_vec3_distance(&players[i].position, &players[i].target) < ACTION_DISTANCE_THRESHOLD) {
                                    // FIXME should use a smaller waypoints buffer for a moving target
                                    // FIXME should also limit pathfinder to a certain depth for better performances
                                    int target_idx = players[i].target_idx;
                                    if (t3d_vec3_distance(&players[i].position, &players[target_idx].position) < ACTION_DISTANCE_THRESHOLD) {
                                        // We have reached the target: attack player
                                        players[i].state = ATTACKING;
                                    } else {
                                        // TODO Player has moved --> find new position and keep moving
                                        players[i].target.v[0] = players[target_idx].position.v[0];
                                        players[i].target.v[1] = players[target_idx].position.v[1];
                                        players[i].target.v[2] = players[target_idx].position.v[2];
                                        debugf("Player #%d now chasing player #%d at *new* coords: %f %f\n", i, target_idx, players[i].target.v[0], players[i].target.v[2]);
                                    }
                                } else {
                                    // We haven't reached the target, get more waypoints
                                    update_path(i);
                                }
                            }
                        }
                        break;
                    }
                    case ATTACKING:
                    {
                        // TODO If key successfully stolen, go to unvisited vault
                        // FIXME attack(i, players[i].target_idx);
                        // TODO attacked player should be stunned and get back to IDLE
                        reset_idle_delay(i);
                        players[i].state = IDLE;
                        break;
                    }
                    case MOVING_TO_VAULT:
                    {
                        // FIXME Don't get stuck!
                        // Move towards next waypoint
                        if (!follow_path(i)) {
                            // No more waypoint
                            if (players[i].path_delay < 0) {
                                // TODO Abandon path ? move back?
                                //reset_idle_delay(i);
                                players[i].state = IDLE;
                            } else {
                                if (can_open(i, players[i].target_idx)) {
                                    // We have reached the target: open vault
                                    players[i].state = OPENING_VAULT;
                                } else {
                                    // We haven't reached the target, get more waypoints
                                    update_path(i);
                                }
                            }
                        }
                        break;
                    }
                    case OPENING_VAULT:
                    {
                        open(i, players[i].target_idx);
                        reset_idle_delay(i);
                        players[i].state = IDLE;
                        break;
                    }
                    default:
                    {
                        reset_idle_delay(i);
                        players[i].state = IDLE;
                        break;
                    }
                }
            }
        }

        // Collision handling
        for (size_t i = 0; i < MAXPLAYERS; i++) {
            // Players cannot move outside the room
            if(players[i].position.v[0] < -(room.w/2.0f - players[i].w/2.0f))   players[i].position.v[0] = -(room.w/2.0f - players[i].w/2.0f);
            if(players[i].position.v[0] >  (room.w/2.0f - players[i].w/2.0f))   players[i].position.v[0] =  (room.w/2.0f - players[i].w/2.0f);
            if(players[i].position.v[2] < -(room.h/2.0f - players[i].h/2.0f))   players[i].position.v[2] = -(room.h/2.0f - players[i].h/2.0f);
            if(players[i].position.v[2] >  (room.h/2.0f - players[i].h/2.0f))   players[i].position.v[2] =  (room.h/2.0f - players[i].h/2.0f);
            // Static objects
            c2AABB player_i = actor_bounding_box((actor_t*)&players[i]);
            for (int j=0; j<FURNITURES_COUNT; j++) {
                c2Manifold m;
                c2AABBtoAABBManifold(player_i, furnitures[j].bbox, &m);
                if (m.count) {
                    players[i].position.v[0] -= m.n.x * COLLISION_CORRECTIVE_FACTOR;
                    players[i].position.v[2] -= m.n.y * COLLISION_CORRECTIVE_FACTOR;
                }
            }
            for (int j=0; j<VAULTS_COUNT; j++) {
                c2Manifold m;
                c2AABBtoAABBManifold(player_i, vaults[j].bbox, &m);
                if (m.count) {
                    players[i].position.v[0] -= m.n.x * COLLISION_CORRECTIVE_FACTOR;
                    players[i].position.v[2] -= m.n.y * COLLISION_CORRECTIVE_FACTOR;
                }
            }
            // Other players
            for (int j=0; j<MAXPLAYERS; j++) {
                if (i != j) {
                    c2AABB player_j = actor_bounding_box((actor_t*)&players[j]);
                    c2Manifold m;
                    c2AABBtoAABBManifold(player_i, player_j, &m);
                    if (m.count) {
                        players[i].position.v[0] -= m.n.x * COLLISION_CORRECTIVE_FACTOR;
                        players[i].position.v[2] -= m.n.y * COLLISION_CORRECTIVE_FACTOR;
                        players[j].position.v[0] += m.n.x * COLLISION_CORRECTIVE_FACTOR;
                        players[j].position.v[2] += m.n.y * COLLISION_CORRECTIVE_FACTOR;
                    }
                }
            }
        }
    }
}

void game_render(float deltatime)
{
    // Room
    rspq_block_run(room.dpl);

    // Furnitures
    for (int i=0; i<FURNITURES_COUNT; i++) {
        rspq_block_run(furnitures[i].dpl);
    }

    // Vaults
    for (int i=0; i<VAULTS_COUNT; i++) {
        rspq_block_run(vaults[i].dpl);
    }

    // Players
    for (int i=0; i<MAXPLAYERS; i++) {
        t3d_anim_update(&players[i].anim_idle, deltatime);
        t3d_anim_set_speed(&players[i].anim_walk, players[i].speed/4.8f + 0.15f);   // TODO Very animation depending on player's speed
        t3d_anim_update(&players[i].anim_walk, deltatime);
        // TODO only set anim when switching state ?
        if (players[i].speed > 0.0f) {
            t3d_anim_set_playing(&players[i].anim_walk, true);
        } else {
            t3d_anim_set_playing(&players[i].anim_idle, true);
        }
        t3d_skeleton_update(&players[i].skel);
        // FIXME Need sync??
        t3d_mat4fp_from_srt_euler(players[i].mat_fp, players[i].scale.v, players[i].rotation.v, players[i].position.v);
        rspq_block_run(players[i].dpl);
        // Display key
        if (players[i].has_key && !key.hidden) {
            t3d_mat4fp_from_srt_euler(key.mat_fp, key.scale.v, key.rotation.v, players[i].position.v);
            rspq_block_run(key.dpl);
        }
    }
}

#if ENABLE_WIREFRAME
void render_actor_aabb(actor_t* actor) {
    c2AABB bbox = actor_bounding_box(actor);
    draw_aabb(bbox.min.x, bbox.max.x, actor->position.v[1], actor->position.v[1] + actor->max_y, bbox.min.y, bbox.max.y, 0.2f, 0.2f, 0.2f);
}
void render_usable_actor_aabb(usable_actor_t* actor) {
    draw_aabb(actor->bbox.min.x, actor->bbox.max.x, actor->position.v[1], actor->position.v[1] + actor->max_y, actor->bbox.min.y, actor->bbox.max.y, 0.2f, 0.2f, 0.2f);
}
void render_usable_zone_aabb(usable_actor_t* actor) {
    draw_aabb(actor->zone_bbox.min.x, actor->zone_bbox.max.x, actor->position.v[1], actor->position.v[1], actor->zone_bbox.min.y, actor->zone_bbox.max.y, 0.2f, 0.5f, 0.2f);
}
#endif

void game_render_gl(float deltatime)
{
#if ENABLE_WIREFRAME
    // Furnitures
    for (int i=0; i<FURNITURES_COUNT; i++) {
        render_usable_actor_aabb((usable_actor_t*)&furnitures[i]);
        render_usable_zone_aabb((usable_actor_t*)&furnitures[i]);
    }

    // Vaults
    for (int i=0; i<VAULTS_COUNT; i++) {
        render_usable_actor_aabb((usable_actor_t*)&vaults[i]);
        render_usable_zone_aabb((usable_actor_t*)&vaults[i]);
    }

    // Players
    for (int i=0; i<MAXPLAYERS; i++) {
        render_actor_aabb((actor_t*)&players[i]);
    }
#endif
}


void game_cleanup()
{
    free(map);

    wav64_close(&sfx_rummage);
    wav64_close(&sfx_key);
    
    rspq_block_free(room.dpl);
    free_uncached(room.mat_fp);
    t3d_model_free(room.model);
    for (int i=0; i<FURNITURES_COUNT; i++) {
        rspq_block_free(furnitures[i].dpl);
        free_uncached(furnitures[i].mat_fp);
    }
    t3d_model_free(furnitures[0].model);
    for (int i=0; i<VAULTS_COUNT; i++) {
        rspq_block_free(vaults[i].dpl);
        free_uncached(vaults[i].mat_fp);
    }
    t3d_model_free(vaults[0].model);
    for (int i=0; i<MAXPLAYERS; i++) {
        rspq_block_free(players[i].dpl);
        t3d_skeleton_destroy(&players[i].skel);
        t3d_anim_destroy(&players[i].anim_idle);
        t3d_anim_destroy(&players[i].anim_walk);
        free_uncached(players[i].mat_fp);
    }
    t3d_model_free(players[0].model);
    rspq_block_free(key.dpl);
    free_uncached(key.mat_fp);
    t3d_model_free(key.model);
}


int game_key() {
    for (int i=0; i<FURNITURES_COUNT; i++) {
        if (furnitures[i].has_key) {
            return i;
        }
    }
    return -1;
}


int game_vault() {
    for (int i=0; i<VAULTS_COUNT; i++) {
        if (vaults[i].is_target) {
            return i;
        }
    }
    return -1;
}


void start_game() {
    playing = true;
}

void stop_game() {
    playing = false;
}

bool is_playing() {
    return playing;
}

bool has_winner() {
    for (int i=0; i<MAXPLAYERS; i++) {
        if (players[i].has_won) {
            return true;
        }
    }
    return false;
}

PlyNum winner() {
    for (int i=0; i<MAXPLAYERS; i++) {
        if (players[i].has_won) {
            return players[i].plynum;
        }
    }
    return MAXPLAYERS;
}
