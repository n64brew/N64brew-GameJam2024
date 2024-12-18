#include "game.h"
#include "astar.h"
#include "../../core.h"
#include "../../minigame.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>

#define ENABLE_TEXT 1
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
#define MAP_REDUCTION_FACTOR 4
#define MAX_PATH_VISIT 500
#define PATH_8_WAYS 1
#define PATH_LOOKUP 30
#define PATH_LENGTH 10
#define NO_PATH 9999
#define WAYPOINT_DELAY 60
#define WAYPOINT_DISTANCE_THRESHOLD 5.0f
#define ACTION_DISTANCE_THRESHOLD 10.0f
#define ACTION_TIME (50.0f/60.0f)
#define ATTACK_TIME (50.0f/60.0f)
#define ATTACK_START (35.0f/60.0f)
#define ATTACK_END (10.0f/60.0f)
#define ATTACK_OFFSET 15.0f
#define ATTACK_RADIUS 10.0f
#define HURT_TIME (50.0f/60.0f)
#define ROOM_SCALE 1.25f
#define FURNITURE_KEEPOUT 65
#define FURNITURES_ROWS 4
#define FURNITURES_COLS 4
#define FURNITURES_COUNT (FURNITURES_ROWS * FURNITURES_COLS)
#define FURNITURE_SCALE 0.75f
#define VAULTS_COUNT (2*(FURNITURES_ROWS-1) + (FURNITURES_COLS-1))
#define PLAYER_SCALE 0.2f
#define FONT_BILLBOARD 3
#define BILLBOARD_YOFFSET 15.0f

bool playing = false;
bool paused = false;

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
    T3DVec3 zone_target;
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
    OPENING_VAULT,
    DEADFISH = 99
} ai_state_t;

typedef struct {
    actor_t;
    PlyNum plynum;
    bool is_human;
    color_t color;
    T3DSkeleton skel;
    T3DAnim anim_idle;
    T3DAnim anim_walk;
    T3DAnim anim_act;
    T3DAnim anim_attack;
    T3DAnim anim_hurt;
    T3DAnim anim_win;
    T3DAnim anim_lose;
    wav64_t sfx_rummage;
    wav64_t sfx_open;
    wav64_t sfx_attack;
    wav64_t sfx_hurt;
    int sfx_channel;
    float speed;
    bool had_key;
    bool has_key;
    bool had_won;
    bool has_won;
    float action_playing_time;
    float attack_playing_time;
    float hurt_playing_time;
    c2AABB bbox;
    c2Circle attack_range;
    // AI players
    int idle_delay;
    ai_state_t state;
    bool furnitures[FURNITURES_COUNT];
    bool vaults[VAULTS_COUNT];
    int target_idx;
    T3DVec3 target;
    T3DVec3 path[PATH_LENGTH];  // Next points in path
    int path_pos;
    int path_lookup;
    int path_keep;
    int path_keep_chase;
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


// Sound FX
wav64_t sfx_key;

rdpq_font_t *fontbill;


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
    // Margin around walls and obstacles to account for players width
    float margin = ((players[0].w/2.0f) / MAP_REDUCTION_FACTOR) + 2;
    // Walls
    for (int x=0; x<margin; x++) {
        for (int y=0; y<room.h; y++) {
            T3DVec3 coords = (T3DVec3){{-room.w/2+x, 0, -room.h/2+y}};
            to_pathmap_coords(&coords, &coords);
            *(map+((int)coords.v[2])*map_width+((int)coords.v[0])) = 1;
        }
    }
    for (int x=room.w; x>room.w-margin; x--) {
        for (int y=0; y<room.h; y++) {
            T3DVec3 coords = (T3DVec3){{-room.w/2+x, 0, -room.h/2+y}};
            to_pathmap_coords(&coords, &coords);
            *(map+((int)coords.v[2])*map_width+((int)coords.v[0])) = 1;
        }
    }
    for (int y=0; y<margin; y++) {
        for (int x=0; x<room.w; x++) {
            T3DVec3 coords = (T3DVec3){{-room.w/2+x, 0, -room.h/2+y}};
            to_pathmap_coords(&coords, &coords);
            *(map+((int)coords.v[2])*map_width+((int)coords.v[0])) = 1;
        }
    }
    for (int y=room.h; y>room.h-margin; y--) {
        for (int x=0; x<room.w; x++) {
            T3DVec3 coords = (T3DVec3){{-room.w/2+x, 0, -room.h/2+y}};
            to_pathmap_coords(&coords, &coords);
            *(map+((int)coords.v[2])*map_width+((int)coords.v[0])) = 1;
        }
    }
    // Furnitures
    for (int i=0; i<FURNITURES_COUNT; i++) {
        T3DVec3 furniture_min = (T3DVec3){{furnitures[i].bbox.min.x-margin, 0, furnitures[i].bbox.min.y-margin}};
        to_pathmap_coords(&furniture_min, &furniture_min);
        T3DVec3 furniture_max = (T3DVec3){{furnitures[i].bbox.max.x+margin, 0, furnitures[i].bbox.max.y+margin}};
        to_pathmap_coords(&furniture_max, &furniture_max);
        for (int x=furniture_min.v[0]+1; x<furniture_max.v[0]; x++) {
            for (int y=furniture_min.v[2]+1; y<furniture_max.v[2]; y++) {
                if (x >= 0 && x < map_width && y >= 0 && y < map_height) {
                    *(map+y*map_width+x) = 1;
                }
            }
        }
    }
    // Vaults
    for (int i=0; i<VAULTS_COUNT; i++) {
        T3DVec3 vault_min = (T3DVec3){{vaults[i].bbox.min.x-margin, 0, vaults[i].bbox.min.y-margin}};
        to_pathmap_coords(&vault_min, &vault_min);
        T3DVec3 vault_max = (T3DVec3){{vaults[i].bbox.max.x+margin, 0, vaults[i].bbox.max.y+margin}};
        to_pathmap_coords(&vault_max, &vault_max);
        for (int x=vault_min.v[0]+1; x<vault_max.v[0]; x++) {
            for (int y=vault_min.v[2]+1; y<vault_max.v[2]; y++) {
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

void add_neighbours(node_list_t* list, cell_t cell) {
    for (int x=cell.x-1; x<=cell.x+1; x++) {
        for (int y=cell.y-1; y<=cell.y+1; y++) {
            if ((x != cell.x || y != cell.y) && is_walkable((cell_t){x, y})) {
#if PATH_8_WAYS
                float cost = (x == cell.x || y == cell.y) ? 1 : 1.414;
                add_neighbour(list, (cell_t){x, y}, cost);
#else
                if (x == cell.x || y == cell.y) {
                    add_neighbour(list, (cell_t){x, y}, 1);
                }
#endif
            }
        }
    }
}

float heuristic(cell_t from, cell_t to) {
    // Manhattan distance
    return (fabs(from.x - to.x) + fabs(from.y - to.y));
}

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
    path_t* path = find_path(start_node, target_node, players[i].path_lookup, MAX_PATH_VISIT);
    if (get_path_count(path) > 1) {
        // Keep fewer waypoints when chasing a player
        int keep = players[i].state == MOVING_TO_PLAYER ? players[i].path_keep_chase : players[i].path_keep;
        for (int j=0; j<get_path_count(path); j++) {
            if (players[i].path_pos >= keep)   break;
            cell_t* node = get_path_cell(path, j);
            players[i].path[players[i].path_pos].v[0] = node->x;
            players[i].path[players[i].path_pos].v[2] = node->y;
            players[i].path_pos++;
        }
        players[i].path_pos = 0;
    } else {
        debugf("No path from %d %d to %d %d\n", start_node.x, start_node.y, target_node.x, target_node.y);
    }
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
        T3DVec3 next_point = (T3DVec3){{next->v[0], 0, next->v[2]}};
        from_pathmap_coords(&next_point, &next_point);
        T3DVec3 newDir;
        t3d_vec3_diff(&newDir, &next_point, &players[i].position);
        float speed = sqrtf(t3d_vec3_len2(&newDir));
        float max_speed = 7.66f + core_get_aidifficulty();
        if (speed > max_speed) {
            speed = max_speed;
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

        // Reached waypoint
        if (t3d_vec3_distance(&players[i].position, &next_point) < WAYPOINT_DISTANCE_THRESHOLD) {
            players[i].path_pos++;
            players[i].path_delay = WAYPOINT_DELAY;
        }

        if (players[i].path_delay < 0) {
            return false;
        }
    }
    
    return has_waypoints(i);
}

void abort_path(int i) {
    players[i].state = IDLE;
}


// Gameplay

void start_player_action(int i) {
    players[i].action_playing_time = ACTION_TIME;
    t3d_anim_set_playing(&players[i].anim_act, true);
    t3d_anim_set_time(&players[i].anim_act, 0.0f);
}

void start_player_attack(int i) {
    float s, c;
    fm_sincosf(players[i].rotation.v[1] + M_PI/2, &s, &c);
    players[i].attack_range.p = c2V(players[i].position.v[0] + c * ATTACK_OFFSET, players[i].position.v[2] + s * ATTACK_OFFSET);
    players[i].attack_range.r = ATTACK_RADIUS;
    players[i].attack_playing_time = ATTACK_TIME;
    t3d_anim_set_playing(&players[i].anim_attack, true);
    t3d_anim_set_time(&players[i].anim_attack, 0.0f);
}

void start_player_hurt(int i) {
    players[i].hurt_playing_time = HURT_TIME;
    t3d_anim_set_playing(&players[i].anim_hurt, true);
    t3d_anim_set_time(&players[i].anim_hurt, 0.0f);
}

bool close_to_furniture(int i, int j) {
    return c2AABBtoAABB(players[i].bbox, furnitures[j].zone_bbox);
}

bool can_rummage(int i, int j) {
    if (close_to_furniture(i, j)) {
        // Is player looking at the furniture?
        T3DVec3 diff;
        t3d_vec3_diff(&diff, &furnitures[j].position, &players[i].position);
        float s, c;
        fm_sincosf(players[i].rotation.v[1] + M_PI/2, &s, &c);
        T3DVec3 dir = (T3DVec3){{c, 0, s}};
        float dot = t3d_vec3_dot(&dir, &diff);
        return dot > 0;
    } else {
        return false;
    }
}

bool rummage(int i, int j) {
    if (can_rummage(i, j)) {
        debugf("Player #%d rummaging through furniture #%d!\n", i, j);
        wav64_play(&players[i].sfx_rummage, players[i].sfx_channel);
        players[i].furnitures[players[i].target_idx] = true;
        start_player_action(i);
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

bool close_to_vault(int i, int j) {
    return c2AABBtoAABB(players[i].bbox, vaults[j].zone_bbox);
}

bool can_open(int i, int j) {
    if (close_to_vault(i, j)) {
        // Is player looking at the vault?
        T3DVec3 diff;
        t3d_vec3_diff(&diff, &vaults[j].position, &players[i].position);
        float s, c;
        fm_sincosf(players[i].rotation.v[1] + M_PI/2, &s, &c);
        T3DVec3 dir = (T3DVec3){{c, 0, s}};
        float dot = t3d_vec3_dot(&dir, &diff);
        return dot > 0;
    } else {
        return false;
    }
}

bool open(int i, int j) {
    if (players[i].has_key && can_open(i, j)) {
        debugf("Player #%d trying open vault #%d!\n", i, j);
        wav64_play(&players[i].sfx_open, players[i].sfx_channel);
        players[i].vaults[players[i].target_idx] = true;
        start_player_action(i);
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
    room.scale = (T3DVec3){{ROOM_SCALE, ROOM_SCALE, ROOM_SCALE}};
    room.rotation = (T3DVec3){{0, 0, 0}};
    room.position = (T3DVec3){{0, 0, 0}};
    room.w = 4.8 * ROOM_SCALE * T3D_MODEL_SCALE;
    room.h = 4.8 * ROOM_SCALE * T3D_MODEL_SCALE;
    room.max_y = 1.81f * ROOM_SCALE * T3D_MODEL_SCALE;
    room.model = t3d_model_load("rom:/tohubohu/room.t3dm");
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
    T3DModel* furniture_model = t3d_model_load("rom:/tohubohu/furniture.t3dm");
    for (int i=0; i<FURNITURES_COUNT; i++) {
        furnitures[i].w = 0.92f * FURNITURE_SCALE * T3D_MODEL_SCALE;
        furnitures[i].h = 0.42f * FURNITURE_SCALE * T3D_MODEL_SCALE;
        furnitures[i].max_y = 0.70f * FURNITURE_SCALE * T3D_MODEL_SCALE;
        furnitures[i].zone_w = furnitures[i].w/2.0f;
        furnitures[i].zone_h = 15.0f;
        furnitures[i].scale = (T3DVec3){{FURNITURE_SCALE, FURNITURE_SCALE, FURNITURE_SCALE}};
        int rotated = rand() % 4;
        furnitures[i].rotated = rotated % 2;
        furnitures[i].rotation = (T3DVec3){{0, rotated * M_PI/2, 0}};
        float slot_w = (room.w-2*FURNITURE_KEEPOUT)/(FURNITURES_COLS-1);
        float slot_h = (room.h-2*FURNITURE_KEEPOUT)/(FURNITURES_ROWS-1);
        furnitures[i].position = (T3DVec3){{
            -room.w/2.0f + FURNITURE_KEEPOUT + slot_w*(i%FURNITURES_COLS),
            0,
            -room.h/2.0f + FURNITURE_KEEPOUT + slot_h*(i/FURNITURES_COLS)
        }};
        furnitures[i].direction = (T3DVec3){{furnitures[i].rotated ? rotated-2 : 0, 0, furnitures[i].rotated ? 0 : 1-rotated}};
        furnitures[i].bbox = usable_actor_bounding_box((usable_actor_t*)&furnitures[i]);
        furnitures[i].zone_bbox = usable_zone_bounding_box((usable_actor_t*)&furnitures[i]);
        t3d_vec3_scale(&furnitures[i].zone_target, &furnitures[i].direction, furnitures[i].h/2.0f + furnitures[i].zone_h);
        t3d_vec3_add(&furnitures[i].zone_target, &furnitures[i].zone_target, &furnitures[i].position);
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
    T3DModel* vault_model = t3d_model_load("rom:/tohubohu/vault.t3dm");
    for (int i=0; i<VAULTS_COUNT; i++) {
        vaults[i].w = 1.09f * T3D_MODEL_SCALE;
        vaults[i].h = 0.11f * T3D_MODEL_SCALE;
        vaults[i].max_y = 1.50f * T3D_MODEL_SCALE;
        vaults[i].zone_w = vaults[i].w/1.5f;
        vaults[i].zone_h = 15.0f;
        vaults[i].scale = (T3DVec3){{1, 1, 1}};
        // Vaults are place in-between furniture rows
        float slot_w = (room.w-2*FURNITURE_KEEPOUT)/(FURNITURES_COLS-1);
        float slot_h = (room.h-2*FURNITURE_KEEPOUT)/(FURNITURES_ROWS-1);
        if (i < FURNITURES_ROWS-1) {
            // (rows-1) vaults on the left
            int y = FURNITURES_ROWS - 2 - i;
            vaults[i].rotated = true;
            vaults[i].rotation = (T3DVec3){{0, -1*M_PI/2, 0}};
            vaults[i].position = (T3DVec3){{ -1*(room.w-vaults[i].h)/2.0f, 0, -room.h/2.0f + FURNITURE_KEEPOUT + (slot_h/2.0f) * (1+2*y) }};
            vaults[i].direction = (T3DVec3){{1, 0, 0}};
        } else if (i < FURNITURES_ROWS-1+FURNITURES_COLS-1) {
            // (cols-1) vaults on top
            int x = i - (FURNITURES_ROWS-1);
            vaults[i].rotated = false;
            vaults[i].rotation = (T3DVec3){{0, 0, 0}};
            vaults[i].position = (T3DVec3){{ -room.w/2.0f + FURNITURE_KEEPOUT + (slot_w/2.0f) * (1+2*x), 0, -1*(room.h-vaults[i].h)/2.0f }};
            vaults[i].direction = (T3DVec3){{0, 0, 1}};
        } else {
            // (rows-1) vaults on the right
            int y = VAULTS_COUNT - 1 - i;
            vaults[i].rotated = true;
            vaults[i].rotation = (T3DVec3){{0, M_PI/2, 0}};
            vaults[i].position = (T3DVec3){{ (room.w-vaults[i].h)/2.0f, 0, -room.h/2.0f + FURNITURE_KEEPOUT + (slot_h/2.0f) * (1+2*(2-y)) }};
            vaults[i].direction = (T3DVec3){{-1, 0, 0}};
        }
        vaults[i].bbox = usable_actor_bounding_box((usable_actor_t*)&vaults[i]);
        vaults[i].zone_bbox = usable_zone_bounding_box((usable_actor_t*)&vaults[i]);
        t3d_vec3_scale(&vaults[i].zone_target, &vaults[i].direction, vaults[i].h/2.0f + vaults[i].zone_h);
        t3d_vec3_add(&vaults[i].zone_target, &vaults[i].zone_target, &vaults[i].position);
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
    T3DModel* player_model = t3d_model_load("rom:/tohubohu/player.t3dm");
    for (int i=0; i<MAXPLAYERS; i++) {
        players[i].w = 1.42f * PLAYER_SCALE * T3D_MODEL_SCALE;
        players[i].h = players[i].w;    // Player is expected to be a square for collisions and pathfinding
        players[i].max_y = 3.60f * PLAYER_SCALE * T3D_MODEL_SCALE;
        players[i].scale = (T3DVec3){{PLAYER_SCALE, PLAYER_SCALE, PLAYER_SCALE}};
        players[i].rotation = (T3DVec3){{0, ((i%2==0) ? -1 : 1) * ((i/2==0) ? 1 : 3) * M_PI/4, 0}};
        float slot_w = (room.w-2*FURNITURE_KEEPOUT)/(FURNITURES_COLS-1);
        float slot_h = (room.h-2*FURNITURE_KEEPOUT)/(FURNITURES_ROWS-1);
        players[i].position = (T3DVec3){{
            ((i%2==0) ? -1 : 1) * (room.w/2.0f - FURNITURE_KEEPOUT - slot_w/2.0f),
            0,
            ((i/2==0) ? -1 : 1) * (room.h/2.0f - FURNITURE_KEEPOUT - slot_h/2.0f)
        }};
        players[i].direction = (T3DVec3){{0, 0, 0}};
        players[i].bbox = actor_bounding_box((actor_t*)&players[i]);
        float s, c;
        fm_sincosf(players[i].rotation.v[1] + M_PI/2, &s, &c);
        players[i].attack_range.p = c2V(players[i].position.v[0] + c * ATTACK_OFFSET, players[i].position.v[2] + s * ATTACK_OFFSET);
        players[i].attack_range.r = ATTACK_RADIUS;
        players[i].model = player_model;
        players[i].mat_fp = malloc_uncached(sizeof(T3DMat4FP));
        players[i].skel = t3d_skeleton_create(players[i].model);
        players[i].anim_idle = t3d_anim_create(players[i].model, "Action_Base");
        t3d_anim_attach(&players[i].anim_idle, &players[i].skel);
        players[i].anim_walk = t3d_anim_create(players[i].model, "Action_Marche");
        t3d_anim_attach(&players[i].anim_walk, &players[i].skel);
        t3d_anim_set_playing(&players[i].anim_walk, false);
        players[i].anim_act = t3d_anim_create(players[i].model, "Action_Act");
        t3d_anim_attach(&players[i].anim_act, &players[i].skel);
        t3d_anim_set_looping(&players[i].anim_act, false);
        t3d_anim_set_playing(&players[i].anim_act, false);
        players[i].anim_attack = t3d_anim_create(players[i].model, "Action_Attack");
        t3d_anim_attach(&players[i].anim_attack, &players[i].skel);
        t3d_anim_set_looping(&players[i].anim_attack, false);
        t3d_anim_set_playing(&players[i].anim_attack, false);
        players[i].anim_hurt = t3d_anim_create(players[i].model, "Action_Hurt");
        t3d_anim_attach(&players[i].anim_hurt, &players[i].skel);
        t3d_anim_set_looping(&players[i].anim_hurt, false);
        t3d_anim_set_playing(&players[i].anim_hurt, false);
        players[i].anim_win = t3d_anim_create(players[i].model, "Action_Win");
        t3d_anim_attach(&players[i].anim_win, &players[i].skel);
        players[i].anim_lose = t3d_anim_create(players[i].model, "Action_Lose");
        t3d_anim_attach(&players[i].anim_lose, &players[i].skel);
        players[i].color = colors[i];
        t3d_mat4fp_from_srt_euler(players[i].mat_fp, players[i].scale.v, players[i].rotation.v, players[i].position.v);
        rspq_block_begin();
            t3d_matrix_push(players[i].mat_fp);
            rdpq_set_prim_color(players[i].color);
            t3d_model_draw_skinned(players[i].model, &players[i].skel);
            t3d_matrix_pop(1);
        players[i].dpl = rspq_block_end();
        wav64_open(&players[i].sfx_rummage, "rom:/tohubohu/rummage.wav64");
        wav64_open(&players[i].sfx_open, "rom:/tohubohu/open.wav64");
        wav64_open(&players[i].sfx_attack, "rom:/tohubohu/attack.wav64");
        wav64_open(&players[i].sfx_hurt, "rom:/tohubohu/hurt.wav64");
        players[i].sfx_channel = 20 + i;
        players[i].plynum = i;
        players[i].is_human = (i < playercount);
        players[i].speed = 0.0f;
        players[i].had_key = false;
        players[i].has_key = false;
        players[i].had_won = false;
        players[i].has_won = false;
        players[i].action_playing_time = 0;
        players[i].attack_playing_time = 0;
        players[i].hurt_playing_time = 0;
        players[i].hidden = false;
        // AI player
        if (!players[i].is_human) {
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
            players[i].path_lookup = PATH_LOOKUP * (1+core_get_aidifficulty());
            players[i].path_keep = (PATH_LENGTH-1) - 2*core_get_aidifficulty();
            players[i].path_keep_chase = (PATH_LENGTH/2) - core_get_aidifficulty();
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
    key.model = t3d_model_load("rom:/tohubohu/key.t3dm");
    key.mat_fp = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_from_srt_euler(key.mat_fp, key.scale.v, key.rotation.v, key.position.v);
    rspq_block_begin();
        t3d_matrix_push(key.mat_fp);
        t3d_model_draw(key.model);
        t3d_matrix_pop(1);
    key.dpl = rspq_block_end();
    key.hidden = true;

    // Sound FX
    wav64_open(&sfx_key, "rom:/tohubohu/key.wav64");

#if ENABLE_TEXT
    // Init fonts
    fontbill = rdpq_font_load("rom:/squarewave.font64");
    rdpq_text_register_font(FONT_BILLBOARD, fontbill);
    for (size_t i = 0; i < MAXPLAYERS; i++) {
        rdpq_font_style(fontbill, i, &(rdpq_fontstyle_t){ .color = colors[i] });
    }
#endif

    // Init pathfinder
    map_width = (room.w/MAP_REDUCTION_FACTOR) + 1;
    map_height = (room.h/MAP_REDUCTION_FACTOR) + 1;
    origin = (T3DVec3){{-map_width/2.0f, 0, -map_height/2.0f}};
    map = calloc(1, sizeof(char) * map_width * map_height);
    update_obstacles();
}


void game_logic(float deltatime)
{
    if (is_playing() && !is_paused()) {
        // Player controls
        for (size_t i = 0; i < MAXPLAYERS; i++) {
            if (players[i].hurt_playing_time > 0) {
                players[i].hurt_playing_time -= deltatime;
                continue;
            } else {
                players[i].hurt_playing_time = 0;
            }
            if (players[i].action_playing_time > 0) {
                players[i].action_playing_time -= deltatime;
                continue;
            } else {
                players[i].action_playing_time = 0;
                if (players[i].has_key && !players[i].had_key) {
                    wav64_play(&sfx_key, 31);
                    players[i].had_key = true;
                    key.hidden = false;
                }
                if (players[i].has_won && !players[i].had_won) {
                    players[i].had_won = true;
                }
            }
            if (players[i].attack_playing_time > 0) {
                players[i].attack_playing_time -= deltatime;
                if (players[i].attack_playing_time <= ATTACK_START && players[i].attack_playing_time >= ATTACK_END) {
                    for (int j=0; j<MAXPLAYERS; j++) {
                        if (i != j && players[j].hurt_playing_time == 0) {
                            if (c2CircletoAABB(players[i].attack_range, players[j].bbox)) {
                                debugf("player #%d hurting player #%d\n", i, j);
                                wav64_play(&players[j].sfx_hurt, players[j].sfx_channel);
                                start_player_hurt(j);
                                // Steal key, if any
                                if (players[j].has_key && !players[j].has_won) {
                                    players[j].has_key = false;
                                    if (!players[j].is_human) {
                                        // Stop moving towards vault if the key is lost
                                        players[j].state = IDLE;
                                    }
                                    players[j].had_key = false;
                                    players[i].has_key = true;
                                }
                            }
                        }
                    }
                }
                continue;
            } else {
                players[i].attack_playing_time = 0;
            }
            if (players[i].is_human) {  // Human player
                joypad_port_t port = core_get_playercontroller(i);
                joypad_inputs_t joypad = joypad_get_inputs(port);
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
                    players[i].speed = 0.0f;
                }
                // Move player
                players[i].position.v[0] += players[i].direction.v[0] * players[i].speed;
                players[i].position.v[2] += players[i].direction.v[2] * players[i].speed;
            } else {    // AI Player
                switch(players[i].state) {
                    case IDLE:
                    {
                        if (players[i].idle_delay > 0) {
                            players[i].idle_delay--;
                            break;
                        }

                        // Find a new target and path
                        //debugf("AI Player #%d needs a new path\n", i);

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
                                players[i].target = vaults[target_idx].zone_target;
                                //debugf("Player #%d now targeting vault #%d at coords: %f %f\n", i, target_idx, players[i].target.v[0], players[i].target.v[2]);
                                next_state = MOVING_TO_VAULT;
                            } else {
                                // Another player has the key, go after them!
                                int target_idx = leader();
                                players[i].target_idx = target_idx;
                                players[i].target.v[0] = players[target_idx].position.v[0];
                                players[i].target.v[1] = players[target_idx].position.v[1];
                                players[i].target.v[2] = players[target_idx].position.v[2];
                                //debugf("Player #%d now targeting player #%d at coords: %f %f\n", i, target_idx, players[i].target.v[0], players[i].target.v[2]);
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
                            players[i].target = furnitures[target_idx].zone_target;
                            //debugf("Player #%d now targeting furniture #%d at coords: %f %f\n", i, target_idx, players[i].target.v[0], players[i].target.v[2]);
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
                        // Move towards next waypoint
                        if (!follow_path(i)) {
                            // No more waypoint
                            if (players[i].path_delay < 0) {
                                abort_path(i);
                            } else {
                                if (close_to_furniture(i, players[i].target_idx)) {
                                    // Rotate towards furniture
                                    T3DVec3 diff;
                                    t3d_vec3_diff(&diff, &furnitures[players[i].target_idx].position, &players[i].position);
                                    float newAngle = -atan2f(diff.v[0], diff.v[2]);
                                    players[i].rotation.v[1] = newAngle;
                                    if (can_rummage(i, players[i].target_idx)) {
                                        // We have reached the target: search for key
                                        players[i].state = RUMMAGING;
                                    } else {
                                        // We have reached the target but something's wrong, find another target
                                        abort_path(i);
                                    }
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
                        // Move towards next waypoint
                        bool has_more_waypoints = follow_path(i);
                        // Attack player if in range, even if the path is not empty
                        int target_idx = players[i].target_idx;
                        if (t3d_vec3_distance(&players[i].position, &players[target_idx].position) - players[i].w < ATTACK_OFFSET+ATTACK_RADIUS) {
                            // Rotate towards player
                            T3DVec3 diff;
                            t3d_vec3_diff(&diff, &players[target_idx].position, &players[i].position);
                            float newAngle = -atan2f(diff.v[0], diff.v[2]);
                            players[i].rotation.v[1] = newAngle;
                            // We have reached the target: attack player
                            players[i].state = ATTACKING;
                            // TODO Need to abort the path ?
                        } else if (!has_more_waypoints) {
                            // No more waypoint
                            if (players[i].path_delay < 0) {
                                abort_path(i);
                            } else {
                                // No need to find remaining waypoints for our original target: find path to the player's new position
                                players[i].target.v[0] = players[target_idx].position.v[0];
                                players[i].target.v[1] = players[target_idx].position.v[1];
                                players[i].target.v[2] = players[target_idx].position.v[2];
                                //debugf("Player #%d now chasing player #%d at *new* coords: %f %f\n", i, target_idx, players[i].target.v[0], players[i].target.v[2]);
                                update_path(i);
                            }
                        }
                        break;
                    }
                    case ATTACKING:
                    {
                        wav64_play(&players[i].sfx_attack, players[i].sfx_channel);
                        start_player_attack(i);
                        reset_idle_delay(i);
                        players[i].state = IDLE;
                        break;
                    }
                    case MOVING_TO_VAULT:
                    {
                        // Move towards next waypoint
                        if (!follow_path(i)) {
                            // No more waypoint
                            if (players[i].path_delay < 0) {
                                abort_path(i);
                            } else {
                                if (close_to_vault(i, players[i].target_idx)) {
                                    // Rotate towards vault
                                    T3DVec3 diff;
                                    t3d_vec3_diff(&diff, &vaults[players[i].target_idx].position, &players[i].position);
                                    float newAngle = -atan2f(diff.v[0], diff.v[2]);
                                    players[i].rotation.v[1] = newAngle;
                                    if (can_open(i, players[i].target_idx)) {
                                        // We have reached the target: open vault
                                        players[i].state = OPENING_VAULT;
                                    } else {
                                        // We have reached the target but something's wrong, find another target
                                        abort_path(i);
                                    }
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
                    case DEADFISH:
                    {
                        // Do nothing, player out
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
            for (int j=0; j<FURNITURES_COUNT; j++) {
                c2Manifold m;
                c2AABBtoAABBManifold(players[i].bbox, furnitures[j].bbox, &m);
                if (m.count && m.depths[0] > 0) {
                    players[i].position.v[0] -= m.n.x * m.depths[0];
                    players[i].position.v[2] -= m.n.y * m.depths[0];
                    players[i].bbox = actor_bounding_box((actor_t*)&players[i]);
                }
            }
            for (int j=0; j<VAULTS_COUNT; j++) {
                c2Manifold m;
                c2AABBtoAABBManifold(players[i].bbox, vaults[j].bbox, &m);
                if (m.count && m.depths[0] > 0) {
                    players[i].position.v[0] -= m.n.x * m.depths[0];
                    players[i].position.v[2] -= m.n.y * m.depths[0];
                    players[i].bbox = actor_bounding_box((actor_t*)&players[i]);
                }
            }
            // Other players
            for (int j=0; j<MAXPLAYERS; j++) {
                if (i != j) {
                    c2Manifold m;
                    c2AABBtoAABBManifold(players[i].bbox, players[j].bbox, &m);
                    if (m.count && m.depths[0] > 0) {
                        players[i].position.v[0] -= m.n.x * m.depths[0]/2.0f;
                        players[i].position.v[2] -= m.n.y * m.depths[0]/2.0f;
                        players[j].position.v[0] += m.n.x * m.depths[0]/2.0f;
                        players[j].position.v[2] += m.n.y * m.depths[0]/2.0f;
                        players[i].bbox = actor_bounding_box((actor_t*)&players[i]);
                        players[j].bbox = actor_bounding_box((actor_t*)&players[j]);
                    }
                }
            }
        }

        // Update bounding boxes
        for (size_t i = 0; i < MAXPLAYERS; i++) {
            players[i].bbox = actor_bounding_box((actor_t*)&players[i]);
        }
    }
}

void game_render(float deltatime, T3DViewport viewport)
{
    if (is_playing()) {
        // Player controls
        for (size_t i = 0; i < MAXPLAYERS; i++) {
            if (players[i].is_human && !is_paused()) {  // Human player
                if (players[i].hurt_playing_time > 0 || players[i].action_playing_time > 0 || players[i].attack_playing_time > 0) {
                    continue;
                }
                joypad_port_t port = core_get_playercontroller(i);
                joypad_buttons_t pressed = joypad_get_buttons_pressed(port);
                // Player actions: rummage, open vault, grab other player
                if(pressed.a) {
                    // If the player is close to a furniture, search for the key
                    for (int j=0; j<FURNITURES_COUNT; j++) {
                        rummage(i, j);
                    }
                    if (players[i].has_key) {
                        for (int j=0; j<VAULTS_COUNT; j++) {
                            open(i, j);
                        }
                    }
                } else if (pressed.b) {
                    wav64_play(&players[i].sfx_attack, players[i].sfx_channel);
                    start_player_attack(i);
                }
            }
        }
    }

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
        if (!is_paused()) {
            if (has_winner()) {
                if (players[i].has_won) {
                    t3d_anim_set_playing(&players[i].anim_walk, false);
                    t3d_anim_set_playing(&players[i].anim_idle, false);
                    t3d_anim_set_playing(&players[i].anim_win, true);
                    t3d_anim_update(&players[i].anim_win, deltatime);
                } else {
                    t3d_anim_set_playing(&players[i].anim_walk, false);
                    t3d_anim_set_playing(&players[i].anim_idle, false);
                    t3d_anim_set_playing(&players[i].anim_lose, true);
                    t3d_anim_update(&players[i].anim_lose, deltatime);
                }
            } else {
                if (players[i].action_playing_time > 0) {
                    t3d_anim_update(&players[i].anim_act, deltatime);
                } else if (players[i].attack_playing_time > 0) {
                    t3d_anim_update(&players[i].anim_attack, deltatime);
                } else if (players[i].hurt_playing_time > 0) {
                    t3d_anim_update(&players[i].anim_hurt, deltatime);
                } else if (players[i].speed > 0.0f) {
                    // TODO only set anim when switching state ?
                    t3d_anim_set_playing(&players[i].anim_walk, true);
                    t3d_anim_set_playing(&players[i].anim_idle, false);
                    t3d_anim_set_speed(&players[i].anim_walk, players[i].speed/3.0f + 0.15f);
                    t3d_anim_update(&players[i].anim_walk, deltatime);
                } else {
                    // TODO only set anim when switching state ?
                    t3d_anim_set_playing(&players[i].anim_idle, true);
                    t3d_anim_set_playing(&players[i].anim_walk, false);
                    t3d_anim_update(&players[i].anim_idle, deltatime);
                }
            }
            t3d_skeleton_update(&players[i].skel);
            t3d_mat4fp_from_srt_euler(players[i].mat_fp, players[i].scale.v, players[i].rotation.v, players[i].position.v);
            if (players[i].has_key && !key.hidden) {
                // Rotate key
                key.rotation.v[1] += deltatime * 4.0f;
                t3d_mat4fp_from_srt_euler(key.mat_fp, key.scale.v, key.rotation.v, players[i].position.v);
            }
        }
        rspq_block_run(players[i].dpl);
        // Display key
        if (players[i].has_key && !key.hidden) {
            rspq_block_run(key.dpl);
        }
    }

    // Billboards
    for (int i=0; i<MAXPLAYERS; i++) {
        // Display billboard
        T3DVec3 billboardPos = (T3DVec3){{
            players[i].position.v[0],
            players[i].position.v[1] + BILLBOARD_YOFFSET,
            players[i].position.v[2]
        }};
        T3DVec3 billboardScreenPos;
        t3d_viewport_calc_viewspace_pos(&viewport, &billboardScreenPos, &billboardPos);
        int x = floorf(billboardScreenPos.v[0]);
        int y = floorf(billboardScreenPos.v[1]);
        rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
        if (players[i].is_human) {
            rdpq_text_printf(&(rdpq_textparms_t){ .style_id = i }, FONT_BILLBOARD, x-5, y-16, "P%d", i+1);
        } else {
            rdpq_text_printf(&(rdpq_textparms_t){ .style_id = i }, FONT_BILLBOARD, x-5, y-16, "CPU");
        }
    }
}

#if ENABLE_WIREFRAME
void render_player_aabb(player_t* actor) {
    draw_aabb(actor->bbox.min.x, actor->bbox.max.x, actor->position.v[1], actor->position.v[1] + actor->max_y, actor->bbox.min.y, actor->bbox.max.y, 0.2f, 0.2f, 0.2f);
}
void render_usable_actor_aabb(usable_actor_t* actor) {
    draw_aabb(actor->bbox.min.x, actor->bbox.max.x, actor->position.v[1], actor->position.v[1] + actor->max_y, actor->bbox.min.y, actor->bbox.max.y, 0.2f, 0.2f, 0.5f);
}
void render_usable_zone_aabb(usable_actor_t* actor) {
    draw_aabb(actor->zone_bbox.min.x, actor->zone_bbox.max.x, actor->position.v[1], actor->position.v[1], actor->zone_bbox.min.y, actor->zone_bbox.max.y, 0.2f, 0.5f, 0.2f);
}
#endif

void game_render_gl(float deltatime)
{
#if ENABLE_WIREFRAME
    // Map
    /*
    for (int x=0; x<map_width; x++) {
        for (int y=0; y<map_height; y++) {
            char walkable = *(map+y*map_width+x) == 0;
            T3DVec3 point = (T3DVec3){{x, 0, y}};
            from_pathmap_coords(&point, &point);
            float r = 1.0f;
            draw_aabb(
                point.v[0]-r, point.v[0]+r,
                point.v[1], point.v[1],
                point.v[2]-r, point.v[2]+r,
                walkable ? 0.0f : 1.0f, 0.5f, 0.5f
            );
        }
    }
    */

    // Furnitures
    for (int i=0; i<FURNITURES_COUNT; i++) {
        render_usable_actor_aabb((usable_actor_t*)&furnitures[i]);
        render_usable_zone_aabb((usable_actor_t*)&furnitures[i]);
        if (furnitures[i].has_key) {
            float r = 3.0f;
            draw_aabb(
                furnitures[i].position.v[0]-r, furnitures[i].position.v[0]+r,
                furnitures[i].max_y, furnitures[i].max_y,
                furnitures[i].position.v[2]-r, furnitures[i].position.v[2]+r,
                0.8f, 0.2f, 0.2f
            );
        }
    }

    // Vaults
    for (int i=0; i<VAULTS_COUNT; i++) {
        render_usable_actor_aabb((usable_actor_t*)&vaults[i]);
        render_usable_zone_aabb((usable_actor_t*)&vaults[i]);
        if (vaults[i].is_target) {
            float r = 3.0f;
            draw_aabb(
                vaults[i].position.v[0]-r, vaults[i].position.v[0]+r,
                vaults[i].max_y, vaults[i].max_y,
                vaults[i].position.v[2]-r, vaults[i].position.v[2]+r,
                0.8f, 0.2f, 0.2f
            );
        }
    }

    // Players
    for (int i=0; i<MAXPLAYERS; i++) {
        render_player_aabb(&players[i]);
        // Draw direction/speed
        if (players[i].speed > 0.0f) {
            float factor = 10.0f;
            draw_line(
                players[i].position.v[0], players[i].position.v[0] + players[i].direction.v[0] * players[i].speed * factor,
                players[i].position.v[1], players[i].position.v[1],
                players[i].position.v[2], players[i].position.v[2] + players[i].direction.v[2] * players[i].speed * factor,
                0.5f, 0.5f, 0.5f
            );
        }
        // Draw path
        if (!players[i].is_human && has_waypoints(i)) {
            int j = 0;
            T3DVec3 curr = players[i].position;
            T3DVec3 next = players[i].path[players[i].path_pos+j];
            while (next.v[0] != NO_PATH) {
                T3DVec3 next_point = (T3DVec3){{next.v[0], 0, next.v[2]}};
                from_pathmap_coords(&next_point, &next_point);
                draw_line(curr.v[0], next_point.v[0], curr.v[1], next_point.v[1], curr.v[2], next_point.v[2], 0.5f, 0.5f, 0.5f);
                // Draw waypoints
                float r = 2.0f;
                draw_aabb(
                    next_point.v[0]-r, next_point.v[0]+r,
                    next_point.v[1], next_point.v[1],
                    next_point.v[2]-r, next_point.v[2]+r,
                    0.5f, 0.8f, 0.5f
                );
                curr = next_point;
                next = players[i].path[players[i].path_pos+(++j)];
            }
            // Target
            float r = 4.0f;
            draw_aabb(
                players[i].target.v[0]-r, players[i].target.v[0]+r,
                players[i].target.v[1], players[i].target.v[1],
                players[i].target.v[2]-r, players[i].target.v[2]+r,
                players[i].color.r/255.0f, players[i].color.g/255.0f, players[i].color.b/255.0f
            );
        }
        // Draw attack range
        if (players[i].attack_playing_time <= ATTACK_START && players[i].attack_playing_time >= ATTACK_END) {
            float x = players[i].attack_range.p.x;
            float y = players[i].position.v[1];
            float z = players[i].attack_range.p.y;
            float r = players[i].attack_range.r;
            draw_aabb(x-r, x+r, y, y, z-r, z+r, 0.8f, 0.5f, 0.5f);
        }
    }
#endif
}


void game_cleanup()
{
    free(map);

#if ENABLE_TEXT
    rdpq_text_unregister_font(FONT_BILLBOARD);
    rdpq_font_free(fontbill);
#endif

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
        t3d_anim_destroy(&players[i].anim_act);
        t3d_anim_destroy(&players[i].anim_attack);
        t3d_anim_destroy(&players[i].anim_hurt);
        t3d_anim_destroy(&players[i].anim_win);
        t3d_anim_destroy(&players[i].anim_lose);
        free_uncached(players[i].mat_fp);
        wav64_close(&players[i].sfx_rummage);
        wav64_close(&players[i].sfx_open);
        wav64_close(&players[i].sfx_attack);
        wav64_close(&players[i].sfx_hurt);
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
    // TODO Update player's animation
    for (int i=0; i<MAXPLAYERS; i++) {
        players[i].speed = 0;
    }
    playing = false;
}

bool is_playing() {
    return playing;
}

bool is_paused() {
    return paused;
}

void toggle_pause() {
    paused = !paused;
}

bool has_winner() {
    for (int i=0; i<MAXPLAYERS; i++) {
        if (players[i].has_won && players[i].had_won) {
            return true;
        }
    }
    return false;
}

PlyNum winner() {
    for (int i=0; i<MAXPLAYERS; i++) {
        if (players[i].has_won && players[i].had_won) {
            return players[i].plynum;
        }
    }
    return MAXPLAYERS;
}
