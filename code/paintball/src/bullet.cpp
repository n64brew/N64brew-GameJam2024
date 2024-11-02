#include "bullet.hpp"

Bullet::Bullet() :
    pos {0},
    velocity {0},
    color {0},
    matFP({(T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP)), free_uncached}) {}

BulletController::BulletController() :
    newBulletCount(0),
    model({
        t3d_model_load("rom:/paintball/snake.t3dm"),
        t3d_model_free
    }),
    block({nullptr, rspq_block_free}) {
        assertf(model.get(), "Bullet model is null");

        rspq_block_begin();
            t3d_model_draw(model.get());
        block = U::RSPQBlock(rspq_block_end(), rspq_block_free);
    }

void BulletController::update(float deltaTime) {
    // assertf(newBulletCount == 0, "Bullets were not processed before render");
    for (auto& bullet : bullets)
    {
        assertf(bullet.matFP.get(), "Bullet matrix is null");
        assertf(block.get(), "Bullet dl is null");

        t3d_mat4fp_from_srt_euler(
            bullet.matFP.get(),
            (float[3]){0.1f, 0.1f, 0.1f},
            /// TODO: add some random rotation & size
            (float[3]){0.0f, 0.0f, 0.0f},
            bullet.pos.v
        );

        if (bullet.velocity.v[0] != 0 || bullet.velocity.v[1] != 0 || bullet.velocity.v[2] != 0) {
            t3d_matrix_push(bullet.matFP.get());
                rdpq_set_prim_color(bullet.color);
                rspq_block_run(block.get());
            t3d_matrix_pop(1);
        }
    }
}

// TODO: shift everything to left to fill in holes & gain performance
void BulletController::killBullet(Bullet &bullet) {
    bullet.velocity.v[0] = 0;
    bullet.velocity.v[1] = 0;
    bullet.velocity.v[2] = 0;
}

void BulletController::fixed_update(float deltaTime, const std::vector<PlayerGameplayData> &gameplayData) {
    for (auto& bullet : bullets)
    {
        // TODO: this will prevent firing once every slot is occupied
        // This is a free bullet slot, fill it if we have pending bullets
        if (bullet.velocity.v[0] == 0 && bullet.velocity.v[1] == 0 && bullet.velocity.v[2] == 0) {
            if (newBulletCount > 0) {
                debugf("spawn bullet\n");

                int bulletIndex = --newBulletCount;
                bullet.pos = newBullets[bulletIndex].pos;
                bullet.velocity = newBullets[bulletIndex].velocity;
                bullet.color = newBullets[bulletIndex].color;
            } else {
                continue;
            }
        }

        // TODO: add some gravity
        T3DVec3 posDiff = {0};
        t3d_vec3_scale(posDiff, bullet.velocity, deltaTime);
        t3d_vec3_add(bullet.pos, bullet.pos, posDiff);

        // Kill bullet
        if (bullet.pos.v[0] > 200.f || bullet.pos.v[0] < -200.f ||
            bullet.pos.v[2] > 200.f || bullet.pos.v[2] < -200.f) {
            killBullet(bullet);
        }

        for (auto& player : gameplayData)
        {
            if (t3d_vec3_distance2(player.pos, bullet.pos) < PlayerRadius * PlayerRadius) {
                killBullet(bullet);
            }
        }
    }
}

void BulletController::fireBullet(const T3DVec3 &pos, const T3DVec3 &velocity, color_t color) {
    debugf("Firing bullet\n");
    newBullets[newBulletCount].pos = pos;
    newBullets[newBulletCount].velocity = velocity;
    newBullets[newBulletCount].color = color;
    newBulletCount++;
}