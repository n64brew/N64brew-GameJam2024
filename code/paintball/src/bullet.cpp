#include "bullet.hpp"

Bullet::Bullet() :
    pos {0},
    prevPos {0},
    velocity {0},
    team {PLAYER_1},
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

void BulletController::render(float deltaTime) {
    const color_t colors[] = {
        PLAYERCOLOR_1,
        PLAYERCOLOR_2,
        PLAYERCOLOR_3,
        PLAYERCOLOR_4,
    };

    double interpolate = core_get_subtick();

    for (auto& bullet : bullets)
    {
        assertf(bullet.matFP.get(), "Bullet matrix is null");
        assertf(block.get(), "Bullet dl is null");

        T3DVec3 currentPos {0};
        t3d_vec3_lerp(currentPos, bullet.prevPos, bullet.pos, interpolate);

        t3d_mat4fp_from_srt_euler(
            bullet.matFP.get(),
            (float[3]){0.1f, 0.1f, 0.1f},
            /// TODO: add some random rotation & size
            (float[3]){0.0f, 0.0f, 0.0f},
            currentPos.v
        );

        if (bullet.velocity.v[0] != 0 || bullet.velocity.v[1] != 0 || bullet.velocity.v[2] != 0) {
            t3d_matrix_push(bullet.matFP.get());
                rdpq_set_prim_color(colors[bullet.team]);
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

/**
 * Returns true if the player changed team
 */
bool BulletController::applyDamage(PlayerGameplayData &gameplayData, PlyNum team) {
    auto currentVal = gameplayData.health[team];

    // Already on same team
    if (gameplayData.team == team || currentVal == MaxHealth) return false;

    gameplayData.health[team] = 0;

    auto result = std::max_element(gameplayData.health.begin(), gameplayData.health.end(), [](int a, int b)
    {
        if (a == b) return (static_cast<float>(rand()) / RAND_MAX) > 0.5f;
        return a < b;
    });

    *result -= Damage;
    if (*result < 0) *result = 0;

    gameplayData.health[team] = currentVal + Damage;
    if (gameplayData.health[team] >= MaxHealth) {
        gameplayData.health[team] = MaxHealth;
        gameplayData.team = team;
        return true;
    }
    return false;
}

void BulletController::simulatePhysics(float deltaTime, Bullet &bullet) {
    bullet.prevPos = bullet.pos;

    // TODO: add some gravity
    T3DVec3 posDiff = {0};
    t3d_vec3_scale(posDiff, bullet.velocity, deltaTime);
    t3d_vec3_add(bullet.pos, bullet.pos, posDiff);

    if (bullet.pos.v[0] > 200.f || bullet.pos.v[0] < -200.f ||
        bullet.pos.v[2] > 200.f || bullet.pos.v[2] < -200.f) {
        killBullet(bullet);
    }
}

/**
 * Returns an array of players changed status this tick
 */
std::array<bool, PlayerCount> BulletController::fixedUpdate(float deltaTime, std::vector<PlayerGameplayData> &gameplayData) {
    std::array<bool, PlayerCount> playerHitStatus {0};

    for (auto& bullet : bullets)
    {
        // TODO: this will prevent firing once every slot is occupied
        // This is a free bullet slot, fill it if we have pending bullets
        if (bullet.velocity.v[0] == 0 && bullet.velocity.v[1] == 0 && bullet.velocity.v[2] == 0) {
            if (newBulletCount > 0) {
                int bulletIndex = --newBulletCount;
                bullet.pos = newBullets[bulletIndex].pos;
                bullet.prevPos = bullet.pos;
                bullet.velocity = newBullets[bulletIndex].velocity;
                bullet.team = newBullets[bulletIndex].team;
            } else {
                continue;
            }
        }

        simulatePhysics(deltaTime, bullet);

        int i = 0;
        for (auto& player : gameplayData)
        {
            if (t3d_vec3_distance2(player.pos, bullet.pos) < PlayerRadius * PlayerRadius) {
                playerHitStatus[i] = applyDamage(player, bullet.team);
                killBullet(bullet);
            }
            i++;
        }
    }

    return playerHitStatus;
}

void BulletController::fireBullet(const T3DVec3 &pos, const T3DVec3 &velocity, PlyNum team) {
    newBullets[newBulletCount].pos = pos;
    newBullets[newBulletCount].velocity = velocity;
    newBullets[newBulletCount].team = team;
    newBulletCount++;
}