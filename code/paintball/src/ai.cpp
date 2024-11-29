#include "ai.hpp"

Direction AI::calculateFireDirection(Player& player, float deltaTime, std::vector<Player> &players) {
    for (auto& other : players) {
        // TODO: difficulty parameter
        if (&other == &player || player.temperature > 0.1f) {
            continue;
        }

        if (other.team == player.team && other.firstHit == player.team) {
            continue;
        }

        T3DVec3 diff = {0};
        t3d_vec3_diff(diff, player.pos, other.pos);

        if (other.velocity.v[0] != 0.f) {
            float enemyXTime = (diff.v[0]/other.velocity.v[0]);
            float bulletYTime = std::abs(diff.v[2] / BulletVelocity);

            if (enemyXTime >= 0 && (enemyXTime - bulletYTime) < PlayerRadius/BulletVelocity) {
                if (diff.v[2] > 0) {
                    return Direction::UP;
                } else {
                    return Direction::DOWN;
                }
            }
        }

        if (other.velocity.v[2] != 0.f) {
            float enemyYTime = (diff.v[2]/other.velocity.v[2]);
            float bulletXTime = std::abs(diff.v[0] / BulletVelocity);

            if (enemyYTime >= 0 && (enemyYTime - bulletXTime) < PlayerRadius/BulletVelocity) {
                if (diff.v[0] > 0) {
                    return Direction::LEFT;
                } else {
                    return Direction::RIGHT;
                }
            }
        }

        if (std::abs(diff.v[0]) < PlayerRadius && t3d_vec3_len(diff) < AIBulletRange*2) {
            if (diff.v[2] > 0) {
                return Direction::UP;
            } else {
                return Direction::DOWN;
            }
        }

        if (std::abs(diff.v[2]) < PlayerRadius && t3d_vec3_len(diff) < AIBulletRange*2) {
            if (diff.v[0] > 0) {
                return Direction::LEFT;
            } else {
                return Direction::RIGHT;
            }
        }
    }

    return Direction::NONE;
}

void AI::calculateMovement(Player& player, float deltaTime, std::vector<Player> &players, T3DVec3 &inputDirection) {
    // Idle defaults
    float escapeWeight = 1.f;

    float centerAttraction = 0.2f;

    // TODO: add bias based on player teams
    float playerAttraction = -0.2f;
    float teamPlayerAttraction = 0.4f;

    float playerRepulsion = 0.5f;
    float teamPlayerRepulsion = 0.2f;

    float alignment = 0.f;

    if (player.aiState == AIState::AI_ATTACK) {
        if (player.temperature > 1.f || player.firstHit != player.team) {
            player.aiState = AIState::AI_DEFEND;
        }
        escapeWeight = 0.2f;

        centerAttraction = 0.f;
        playerAttraction = 0.2f;
        teamPlayerAttraction = 0.5f;

        playerRepulsion = 1.f;
        teamPlayerRepulsion = 0.3f;

        alignment = 0.7f;
    } else if (player.aiState == AIState::AI_DEFEND) {
        if (player.temperature < 1.f && player.firstHit == player.team) {
            player.aiState = AIState::AI_ATTACK;
        }
        escapeWeight = 0.8f;

        centerAttraction = 0.2f;
        playerAttraction = 0.2f;
        teamPlayerAttraction = 0.8f;

        playerRepulsion = 0.1f;
        teamPlayerRepulsion = 0.1f;

        alignment = 0.2f;
    } else {
        if (player.firstHit == player.team) {
            player.aiState = AIState::AI_ATTACK;
        } else {
            player.aiState = AIState::AI_DEFEND;
        }
    }

    // Bullet escape
    for (auto bullet = player.incomingBullets.begin(); bullet != player.incomingBullets.end(); ++bullet) {
        if (bullet->team == player.team) {
            continue;
        }

        T3DVec3 diff = {0};
        t3d_vec3_diff(diff, player.pos, bullet->pos);
        diff.v[1] = 0.f;

        T3DVec3 bulletVelocityDir = bullet->velocity;
        bulletVelocityDir.v[1] = 0.f;
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
        // TODO: adjust with distance
        if (t3d_vec3_len2(diff) > AIBulletRange) {
            T3DVec3 myDiff = diff;
            t3d_vec3_norm(myDiff);
            if (player.team == other.team) {
                t3d_vec3_scale(myDiff, myDiff, -teamPlayerAttraction);
            } else {
                t3d_vec3_scale(myDiff, myDiff, -playerAttraction);
            }
            t3d_vec3_add(inputDirection, inputDirection, myDiff);
        }

        // Player repulsion
        // TODO: adjust with distance
        if (t3d_vec3_len2(diff) < AIBulletRange) {
            T3DVec3 myDiff = diff;
            t3d_vec3_norm(myDiff);
            if (player.team == other.team) {
                t3d_vec3_scale(myDiff, myDiff, teamPlayerRepulsion);
            } else {
                t3d_vec3_scale(myDiff, myDiff, playerRepulsion);
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