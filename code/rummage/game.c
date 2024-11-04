#include "game.h"
#include "../../core.h"
#include "../../minigame.h"
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>


#define T3D_MODEL_SCALE 64

typedef struct actor_t {
    T3DModel* model;
    T3DMat4FP* mat_fp;
    rspq_block_t* dpl;
    T3DVec3 scale;
    T3DVec3 rotation;
    T3DVec3 position;
    float w;
    float h;
} actor_t;

typedef struct {
    actor_t;
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
    T3DModel* furniture_model = t3d_model_load("rom:/rummage/furniture.t3dm");
    for (int i=0; i<FURNITURES_COUNT; i++) {
        furnitures[i].w = 0.92f * T3D_MODEL_SCALE;
        furnitures[i].h = 0.42f * T3D_MODEL_SCALE;
        furnitures[i].scale = (T3DVec3){{1, 1, 1}};
        furnitures[i].rotation = (T3DVec3){{0, ((float)rand()/(float)(RAND_MAX)) * 2 * M_PI, 0}};  // TODO randomize rotation?
        furnitures[i].position = (T3DVec3){{ ((i%3)-1)*((room.w-furnitures[i].w-50)/2.0f), 0, (((i/3)%3)-1)*((room.h-furnitures[i].h-50)/2.0f) }};
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
    T3DModel* vault_model = t3d_model_load("rom:/rummage/vault.t3dm");
    for (int i=0; i<VAULTS_COUNT; i++) {
        vaults[i].w = 1.09f * T3D_MODEL_SCALE;
        vaults[i].h = 0.11f * T3D_MODEL_SCALE;
        vaults[i].scale = (T3DVec3){{1, 1, 1}};
        vaults[i].rotation = (T3DVec3){{0, (i-1)*M_PI/2, 0}};
        vaults[i].position = (T3DVec3){{ (i-1)*(room.w-vaults[i].h)/2.0f, 0, -1*(room.h-vaults[i].h)/2.0f*(i%2) }};
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
        players[i].w = 0.5f * T3D_MODEL_SCALE;
        players[i].h = 0.5f * T3D_MODEL_SCALE;
        players[i].scale = (T3DVec3){{1, 1, 1}};
        players[i].rotation = (T3DVec3){{0, (i-1)*M_PI/2, 0}};
        players[i].position = (T3DVec3){{ ((i-1)%2)*(room.w-players[i].h)/3.0f, 0, ((i-2)%2)*(room.h-players[i].h)/3.0f }};
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
        players[i].has_key = false;
    }
}


void game_logic(float deltatime)
{
    uint32_t playercount = core_get_playercount();
    for (size_t i = 0; i < core_get_playercount(); i++) {
        if (i < playercount) {  // Human player
            joypad_port_t port = core_get_playercontroller(i);
            joypad_buttons_t btn = joypad_get_buttons_pressed(port);
            if (btn.start) {
                minigame_end();
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
