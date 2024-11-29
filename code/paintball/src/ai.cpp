#include "ai.hpp"

Direction AI::calculateFireDirection(Player& player, float deltaTime, std::vector<Player> &players) {
    for (auto& other : players) {
        if (&other == &player) {
            continue;
        }
        T3DVec3 diff = {0};
        t3d_vec3_diff(diff, player.pos, other.pos);
        if (t3d_vec3_len2(diff) > AIBulletRange) {
            continue;
        }

    }

    return Direction::NONE;
}

void AI::calculateMovement(Player& player, float deltaTime, std::vector<Player> &players, T3DVec3 &inputDirection) {
    float escapeWeight = 1.f;

    float centerAttraction = 0.01f;
    float playerAttraction = 0.5f;
    float teamPlayerAttraction = 0.7f;
    float alignment = 0.01f;

    // Bullet escape
    for (auto bullet = player.incomingBullets.begin(); bullet != player.incomingBullets.end(); ++bullet) {
        if (bullet->team == player.team) {
            continue;
        }

        T3DVec3 diff = {0};
        t3d_vec3_diff(diff, player.pos, bullet->pos);

        T3DVec3 bulletVelocityDir = bullet->velocity;
        t3d_vec3_norm(bulletVelocityDir);

        float diffProjLen = t3d_vec3_dot(bulletVelocityDir, diff);

        T3DVec3 diffProj = {0};
        t3d_vec3_scale(diffProj, bulletVelocityDir, diffProjLen);

        T3DVec3 diffPerp = {0};
        t3d_vec3_diff(diffPerp, diff, diffProj);
        t3d_vec3_norm(diffPerp);
        t3d_vec3_scale(diffPerp, diffPerp, escapeWeight);

        if (t3d_vec3_len(diffPerp) == 0.f) [[unlikely]] {
            // rotate clockwise
            diffPerp.v[0] = -bulletVelocityDir.v[2];
            diffPerp.v[2] = bulletVelocityDir.v[0];
        } else {
            // TODO: increase strength if close by
            t3d_vec3_add(inputDirection, inputDirection, diffPerp);
        }
    }
    player.incomingBullets.clear();

    // center attraction
    T3DVec3 diff = {0};
    t3d_vec3_diff(diff, T3DVec3 {0}, player.pos);
    t3d_vec3_norm(diff);
    t3d_vec3_scale(diff, diff, centerAttraction);
    t3d_vec3_add(inputDirection, inputDirection, diff);

    for (auto& other : players) {
        if (&other == &player) {
            continue;
        }

        T3DVec3 diff = {0};
        t3d_vec3_diff(diff, player.pos, other.pos);

        // Player attraction
        if (t3d_vec3_len2(diff) > AIBulletRange * 2.f) {
            T3DVec3 myDiff = diff;
            t3d_vec3_norm(myDiff);
            if (player.team == other.team) {
                t3d_vec3_scale(myDiff, myDiff, -teamPlayerAttraction);
            } else {
                t3d_vec3_scale(myDiff, myDiff, -playerAttraction);
            }
            t3d_vec3_add(inputDirection, inputDirection, myDiff);
        }

        // Player alignment
        T3DVec3 myDiff = diff;
        t3d_vec3_norm(myDiff);
        if (diff.v[0] < diff.v[2]) {
            myDiff.v[2] = 0;
        } else {
            myDiff.v[0] = 0;
        }

        if (player.team == other.team) {
            t3d_vec3_scale(myDiff, myDiff, -alignment * 0.5);
        } else {
            t3d_vec3_scale(myDiff, myDiff, -alignment);
        }

        t3d_vec3_add(inputDirection, inputDirection, myDiff);
    }


    inputDirection.v[1] = 0.f;
    t3d_vec3_norm(inputDirection);
    t3d_vec3_scale(inputDirection, inputDirection, ForceLimit);
}