#include "game.h"
#include "../../core.h"
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

// TODO Players
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
    for (int i=0; i<FURNITURES_COUNT; i++) {
        furnitures[i].w = 0.92f * T3D_MODEL_SCALE;
        furnitures[i].h = 0.42f * T3D_MODEL_SCALE;
        furnitures[i].scale = (T3DVec3){{1, 1, 1}};
        furnitures[i].rotation = (T3DVec3){{0, ((float)rand()/(float)(RAND_MAX)) * 2 * M_PI, 0}};  // TODO randomize rotation?
        furnitures[i].position = (T3DVec3){{ ((i%3)-1)*((room.w-furnitures[i].w-50)/2.0f), 0, (((i/3)%3)-1)*((room.h-furnitures[i].h-50)/2.0f) }};
        furnitures[i].model = t3d_model_load("rom:/rummage/furniture.t3dm");
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
    for (int i=0; i<VAULTS_COUNT; i++) {
        vaults[i].w = 1.09f * T3D_MODEL_SCALE;
        vaults[i].h = 0.11f * T3D_MODEL_SCALE;
        vaults[i].scale = (T3DVec3){{1, 1, 1}};
        vaults[i].rotation = (T3DVec3){{0, (i-1)*M_PI/2, 0}};
        vaults[i].position = (T3DVec3){{ (i-1)*(room.w-vaults[i].h)/2.0f, 0, -1*(room.h-vaults[i].h)/2.0f*(i%2) }};
        vaults[i].model = t3d_model_load("rom:/rummage/vault.t3dm");
        vaults[i].mat_fp = malloc_uncached(sizeof(T3DMat4FP));
        t3d_mat4fp_from_srt_euler(vaults[i].mat_fp, vaults[i].scale.v, vaults[i].rotation.v, vaults[i].position.v);
        rspq_block_begin();
            t3d_matrix_push(vaults[i].mat_fp);
            t3d_model_draw(vaults[i].model);
            t3d_matrix_pop(1);
        vaults[i].dpl = rspq_block_end();
        vaults[i].is_target = (i == target);
    }

    // TODO Place players
}


void game_logic(float deltatime)
{
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

    // TODO Players
}


void game_cleanup()
{
    rspq_block_free(room.dpl);
    for (int i=0; i<FURNITURES_COUNT; i++) {
        rspq_block_free(furnitures[i].dpl);
    }
    for (int i=0; i<VAULTS_COUNT; i++) {
        rspq_block_free(vaults[i].dpl);
    }
    free_uncached(room.mat_fp);
    t3d_model_free(room.model);
}
