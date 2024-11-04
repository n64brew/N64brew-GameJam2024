#include "game.h"
#include "../../core.h"
#include "../../minigame.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wunused-function"
#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.h"
#pragma GCC diagnostic pop


#define T3D_MODEL_SCALE 64
#define COLLISION_CORRECTIVE_FACTOR 1.3f

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
} actor_t;

typedef struct {
    actor_t;
    bool rotated;
    bool has_key;
} furniture_t;

typedef struct {
    actor_t;
    bool is_target;
} vault_t;

typedef struct {
    actor_t;
    // TODO rotation, speed, skeleton, animations, ...
    PlyNum plynum;
    color_t color;
    float speed;
    bool has_key;
} player_t;


// Room
actor_t room;

// Furnitures
#define FURNITURES_COUNT 9
furniture_t furnitures[FURNITURES_COUNT];

// Vaults
#define VAULTS_COUNT 3
vault_t vaults[VAULTS_COUNT];

// Players
player_t players[MAXPLAYERS];


void game_init()
{
    // Init room
    room.scale = (T3DVec3){{1, 1, 1}};
    room.rotation = (T3DVec3){{0, 0, 0}};
    room.position = (T3DVec3){{0, 0, 0}};
    room.w = 4.8 * T3D_MODEL_SCALE;
    room.h = 4.8 * T3D_MODEL_SCALE;
    room.model = t3d_model_load("rom:/rummage/room.t3dm");
    room.mat_fp = malloc_uncached(sizeof(T3DMat4FP));
    t3d_mat4fp_from_srt_euler(room.mat_fp, room.scale.v, room.rotation.v, room.position.v);
    rspq_block_begin();
        t3d_matrix_push(room.mat_fp);
        t3d_model_draw(room.model);
        t3d_matrix_pop(1);
    room.dpl = rspq_block_end();

    // Place furnitures
    int key = rand() % FURNITURES_COUNT;
    debugf("Key is in furniture #%d!\n", key);
    T3DModel* furniture_model = t3d_model_load("rom:/rummage/furniture.t3dm");
    for (int i=0; i<FURNITURES_COUNT; i++) {
        furnitures[i].w = 0.92f * T3D_MODEL_SCALE;
        furnitures[i].h = 0.42f * T3D_MODEL_SCALE;
        furnitures[i].scale = (T3DVec3){{1, 1, 1}};
        int rotated = rand() % 3;
        furnitures[i].rotated = rotated % 2;
        furnitures[i].rotation = (T3DVec3){{0, rotated * M_PI/2, 0}};
        furnitures[i].position = (T3DVec3){{ ((i%3)-1)*((room.w-furnitures[i].w-50)/2.0f), 0, (((i/3)%3)-1)*((room.h-furnitures[i].h-50)/2.0f) }};
        furnitures[i].direction = (T3DVec3){{furnitures[i].rotated ? rotated-2 : 0, 0, furnitures[i].rotated ? 0 : 1-rotated}};
        furnitures[i].model = furniture_model;
        furnitures[i].mat_fp = malloc_uncached(sizeof(T3DMat4FP));
        t3d_mat4fp_from_srt_euler(furnitures[i].mat_fp, furnitures[i].scale.v, furnitures[i].rotation.v, furnitures[i].position.v);
        rspq_block_begin();
            t3d_matrix_push(furnitures[i].mat_fp);
            t3d_model_draw(furnitures[i].model);
            t3d_matrix_pop(1);
        furnitures[i].dpl = rspq_block_end();
        furnitures[i].has_key = (i == key);
    }

    // Place vaults
    int target = rand() % VAULTS_COUNT;
    debugf("Target is vault #%d!\n", target);
    T3DModel* vault_model = t3d_model_load("rom:/rummage/vault.t3dm");
    for (int i=0; i<VAULTS_COUNT; i++) {
        vaults[i].w = 1.09f * T3D_MODEL_SCALE;
        vaults[i].h = 0.11f * T3D_MODEL_SCALE;
        vaults[i].scale = (T3DVec3){{1, 1, 1}};
        vaults[i].rotation = (T3DVec3){{0, (i-1)*M_PI/2, 0}};
        vaults[i].position = (T3DVec3){{ (i-1)*(room.w-vaults[i].h)/2.0f, 0, -1*(room.h-vaults[i].h)/2.0f*(i%2) }};
        vaults[i].direction = (T3DVec3){{1-i, 0, i%2}};
        vaults[i].model = vault_model;
        vaults[i].mat_fp = malloc_uncached(sizeof(T3DMat4FP));
        t3d_mat4fp_from_srt_euler(vaults[i].mat_fp, vaults[i].scale.v, vaults[i].rotation.v, vaults[i].position.v);
        rspq_block_begin();
            t3d_matrix_push(vaults[i].mat_fp);
            t3d_model_draw(vaults[i].model);
            t3d_matrix_pop(1);
        vaults[i].dpl = rspq_block_end();
        vaults[i].is_target = (i == target);
    }

    // Place players
    const color_t colors[] = {
        PLAYERCOLOR_1,
        PLAYERCOLOR_2,
        PLAYERCOLOR_3,
        PLAYERCOLOR_4
    };
    T3DModel* player_model = t3d_model_load("rom:/rummage/player.t3dm");
    for (int i=0; i<MAXPLAYERS; i++) {
        players[i].w = 0.3f * T3D_MODEL_SCALE;
        players[i].h = 0.3f * T3D_MODEL_SCALE;
        players[i].scale = (T3DVec3){{1, 1, 1}};
        players[i].rotation = (T3DVec3){{0, (i-1)*M_PI/2, 0}};
        players[i].position = (T3DVec3){{ ((i-1)%2)*50, 0, ((i-2)%2)*50 }};
        players[i].direction = (T3DVec3){{0, 0, 0}};
        players[i].model = player_model;
        players[i].mat_fp = malloc_uncached(sizeof(T3DMat4FP));
        players[i].color = colors[i];
        t3d_mat4fp_from_srt_euler(players[i].mat_fp, players[i].scale.v, players[i].rotation.v, players[i].position.v);
        rspq_block_begin();
            t3d_matrix_push(players[i].mat_fp);
            rdpq_set_prim_color(players[i].color);
            t3d_model_draw(players[i].model);
            t3d_matrix_pop(1);
        players[i].dpl = rspq_block_end();
        players[i].plynum = i;
        players[i].speed = 0.0f;
        players[i].has_key = false;
    }
}


void game_logic(float deltatime)
{
    // Player controls
    uint32_t playercount = core_get_playercount();
    for (size_t i = 0; i < MAXPLAYERS; i++) {
        if (i < playercount) {  // Human player
            joypad_port_t port = core_get_playercontroller(i);
            joypad_inputs_t joypad = joypad_get_inputs(port);
            // Exit minigame by pressing start
            if (joypad.btn.start) {
                minigame_end();
            }
            // Player actions: rummage, open vault, grab other player
            // FIXME Limit the rate at which a player can perform an action
            if(joypad.btn.a || joypad.btn.b) {
                // If the player is close to a furniture, search for the key
                for (int j=0; j<FURNITURES_COUNT; j++) {
                    // Is player on the front side of furniture?
                    T3DVec3 diff;
                    t3d_vec3_diff(&diff, &players[i].position, &furnitures[j].position);
                    float dot = t3d_vec3_dot(&furnitures[j].direction, &diff);
                    if (dot > 0) {
                        // Is player close enough?
                        float center_distance = t3d_vec3_distance(&players[i].position, &furnitures[j].position);
                        float distance = center_distance - players[i].h/2.0f - furnitures[j].h/2.0f;
                        if (distance <= 8.0f) {
                            debugf("Rummaging through furniture #%d!\n", j);
                            if (furnitures[j].has_key) {
                                debugf("Player #%d found key in furniture #%d!\n", i, j);
                                players[i].has_key = true;
                                furnitures[j].has_key = false;
                            }
                        }
                    }
                }
                if (players[i].has_key) {
                    for (int j=0; j<VAULTS_COUNT; j++) {
                        // Is player on the front side of vault?
                        T3DVec3 diff;
                        t3d_vec3_diff(&diff, &players[i].position, &vaults[j].position);
                        float dot = t3d_vec3_dot(&vaults[j].direction, &diff);
                        if (dot > 0) {
                            // Is player close enough?
                            float center_distance = t3d_vec3_distance(&players[i].position, &vaults[j].position);
                            float distance = center_distance - players[i].h/2.0f - vaults[j].h/2.0f;
                            if (distance <= 8.0f) {
                                debugf("Trying open vault #%d!\n", j);
                                if (vaults[j].is_target) {
                                    debugf("Player #%d opened the vault #%d!\n", i, j);
                                    core_set_winner(i);
                                    minigame_end(); // FIXME Don't exit minigame immediately
                                }
                            }
                        }
                    }
                }
            }
            T3DVec3 newDir = {0};
            newDir.v[0] = (float)joypad.stick_x * 0.05f;
            newDir.v[2] = -(float)joypad.stick_y * 0.05f;
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
        c2AABB player_i;
        player_i.min = c2V(players[i].position.v[0] - players[i].w/2.0f, players[i].position.v[2] - players[i].h/2.0f);
        player_i.max = c2V(players[i].position.v[0] + players[i].w/2.0f, players[i].position.v[2] + players[i].h/2.0f);
        for (int j=0; j<FURNITURES_COUNT; j++) {
            c2AABB furniture_j;
            float fw = furnitures[j].rotated ? furnitures[j].h : furnitures[j].w;
            float fh = furnitures[j].rotated ? furnitures[j].w : furnitures[j].h;
            furniture_j.min = c2V(furnitures[j].position.v[0] - fw/2.0f, furnitures[j].position.v[2] - fh/2.0f);
            furniture_j.max = c2V(furnitures[j].position.v[0] + fw/2.0f, furnitures[j].position.v[2] + fh/2.0f);
            c2Manifold m;
            c2AABBtoAABBManifold(player_i, furniture_j, &m);
            if (m.count) {
                players[i].position.v[0] -= m.n.x * COLLISION_CORRECTIVE_FACTOR;
                players[i].position.v[2] -= m.n.y * COLLISION_CORRECTIVE_FACTOR;
            }
        }
        for (int j=0; j<VAULTS_COUNT; j++) {
            c2AABB vault_j;
            vault_j.min = c2V(vaults[j].position.v[0] - vaults[j].w/2.0f, vaults[j].position.v[2] - vaults[j].h/2.0f);
            vault_j.max = c2V(vaults[j].position.v[0] + vaults[j].w/2.0f, vaults[j].position.v[2] + vaults[j].h/2.0f);
            c2Manifold m;
            c2AABBtoAABBManifold(player_i, vault_j, &m);
            if (m.count) {
                players[i].position.v[0] -= m.n.x * COLLISION_CORRECTIVE_FACTOR;
                players[i].position.v[2] -= m.n.y * COLLISION_CORRECTIVE_FACTOR;
            }
        }
        // Other players
        for (int j=0; j<MAXPLAYERS; j++) {
            if (i != j) {
                c2AABB player_j;
                player_j.min = c2V(players[j].position.v[0] - players[j].w/2.0f, players[j].position.v[2] - players[j].h/2.0f);
                player_j.max = c2V(players[j].position.v[0] + players[j].w/2.0f, players[j].position.v[2] + players[j].h/2.0f);
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
        t3d_mat4fp_from_srt_euler(players[i].mat_fp, players[i].scale.v, players[i].rotation.v, players[i].position.v);
        rspq_block_run(players[i].dpl);
    }
}


void game_cleanup()
{
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
        free_uncached(players[i].mat_fp);
    }
    t3d_model_free(players[0].model);
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
