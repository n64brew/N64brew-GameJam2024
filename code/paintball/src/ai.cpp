#include "ai.hpp"

int randomRange(int min, int max){
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

AI::AI() : aiActionTimer(0) {
    difficulty = core_get_aidifficulty();
}

Direction AI::calculateFireDirection(Player& player, float deltaTime, std::vector<Player> &players) {
    aiActionTimer += deltaTime;

    float actionRate = AIActionRateSecond;
    float tempControl = 1.f;
    if (difficulty == AiDiff::DIFF_EASY) {
        actionRate = AIActionRateSecond * 6;
    } else if (difficulty == AiDiff::DIFF_MEDIUM) {
        actionRate = AIActionRateSecond * 3;
    } else if (difficulty == AiDiff::DIFF_HARD) {
        actionRate = AIActionRateSecond;
        tempControl = (player.aiState == AIState::AI_ATTACK) ? 0.8f : 0.2f;
    }

    if (aiActionTimer < actionRate) {
        return Direction::NONE;
    }
    aiActionTimer = 0;

    // Random fire & skip fire
    float random = static_cast<float>(rand()) / RAND_MAX;
    if (difficulty == AiDiff::DIFF_EASY) {
        if (random < 0.6f) {
            return Direction::NONE;
        }
        if (random < 0.7) {
            return (Direction)randomRange(0, 3);
        }
    } else if (difficulty == AiDiff::DIFF_MEDIUM) {
        if (random < 0.25f) {
            return Direction::NONE;
        }
        if (random < 0.3f) {
            return (Direction)randomRange(0, 3);
        }
    }

    for (auto& other : players) {
        if (&other == &player) {
            continue;
        }

        if (player.temperature > tempControl) {
            continue;
        }

        // Already at full health
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

        if (std::abs(diff.v[0]) < PlayerRadius && t3d_vec3_len(diff) < AICloseRange) {
            if (diff.v[2] > 0) {
                return Direction::UP;
            } else {
                return Direction::DOWN;
            }
        }

        if (std::abs(diff.v[2]) < PlayerRadius && t3d_vec3_len(diff) < AICloseRange) {
            if (diff.v[0] > 0) {
                return Direction::LEFT;
            } else {
                return Direction::RIGHT;
            }
        }
    }

    return Direction::NONE;
}

void AI::tryChangeState(Player& player, AIState newState) {
    float random = static_cast<float>(rand()) / RAND_MAX;

    if (random < AIStability) {
        return;
    }

    player.aiState = newState;
    player.multiplier = 1.f + AIRandomRange * (static_cast<float>(rand()) / RAND_MAX);
    player.multiplier2 = 1.f + AIRandomRange * (static_cast<float>(rand()) / RAND_MAX);
}

void AI::calculateMovement(Player& player, float deltaTime, std::vector<Player> &players, T3DVec3 &inputDirection) {
    float random = static_cast<float>(rand()) / RAND_MAX;

    // Defaults
    float escapeWeight = 100.f;

    float centerAttraction = 0.f;
    float randomWeight = 0.1f;

    // TODO: add bias based on player teams
    float playerAttraction = 0.f;
    float playerRepulsion = 0.f;

    float alignment = 0.f;

    if (difficulty == AiDiff::DIFF_EASY) {
        escapeWeight *= 0.05f;
    } else if (difficulty == AiDiff::DIFF_MEDIUM) {
        escapeWeight *= 0.2f;
    }

    float temperature = AITemperature;
    // Easy mode is more random
    if (difficulty == AiDiff::DIFF_EASY) {
        temperature = AITemperature * 3.f;
    } else if (difficulty == AiDiff::DIFF_MEDIUM) {
        temperature = AITemperature * 1.5f;
    }
    if (random < temperature) {
        // TODO: different probabilities
        int r = randomRange(1, 3);
        player.aiState = (AIState)r;
    }

    if (player.aiState == AIState::AI_ATTACK) {
        if (player.temperature > 1.f || player.firstHit != player.team) {
            tryChangeState(player, AIState::AI_DEFEND);
        }
        centerAttraction = 0.5f;

        playerAttraction = 0.4f * player.multiplier;
        playerRepulsion = 0.3f * player.multiplier2;

        alignment = 0.1f * player.multiplier;
    } else if (player.aiState == AIState::AI_DEFEND) {
        if (player.temperature < 1.f && player.firstHit == player.team) {
            tryChangeState(player, AIState::AI_ATTACK);
        }
        centerAttraction = 0.8f;

        playerAttraction = 0.2f * player.multiplier2;
        playerRepulsion = 0.5f * player.multiplier;

        alignment = -0.1f * player.multiplier;
    } else if (player.aiState == AIState::AI_RUN) {
        centerAttraction = 0.2f;
        randomWeight = 1.f;

        playerAttraction = 0.001f * player.multiplier2;
        playerRepulsion = 0.001f * player.multiplier;

        alignment = 0.f;
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

    // random walk
    if (randomWeight > 0) {
        T3DVec3 force = {player.multiplier -1.f -(AIRandomRange/2.f), 0.f, player.multiplier2-1.f -(AIRandomRange/2.f)};
        t3d_vec3_norm(force);
        t3d_vec3_scale(force, force, randomWeight);
        t3d_vec3_add(inputDirection, inputDirection, force);
    }

    for (auto& other : players) {
        if (&other == &player) {
            continue;
        }

        T3DVec3 diff = {0};
        t3d_vec3_diff(diff, player.pos, other.pos);

        // Player attraction
        if (t3d_vec3_len(diff) > AIAttractRange) {
            T3DVec3 myDiff = diff;
            float scale = t3d_vec3_len(diff) / AIAttractRange;
            scale = std::min(1.2f, scale);
            t3d_vec3_scale(myDiff, myDiff, scale);
            if (player.team == other.team) {
                t3d_vec3_scale(myDiff, myDiff, -0.1 * playerAttraction);
            } else {
                t3d_vec3_scale(myDiff, myDiff, -playerAttraction);
            }
            t3d_vec3_add(inputDirection, inputDirection, myDiff);
        }

        // Player repulsion
        if (t3d_vec3_len(diff) < AICloseRange) {
            T3DVec3 myDiff = diff;
            float scale = AICloseRange / t3d_vec3_len(diff);
            scale = std::min(1.2f, scale);
            t3d_vec3_scale(myDiff, myDiff, scale);
            if (player.team == other.team) {
                t3d_vec3_scale(myDiff, myDiff, 0.2 * playerRepulsion);
            } else {
                t3d_vec3_scale(myDiff, myDiff, playerRepulsion);
            }
            t3d_vec3_add(inputDirection, inputDirection, myDiff);
        }

        // Player alignment
        if (t3d_vec3_len(diff) < AICloseRange) {
            T3DVec3 myDiff = diff;
            t3d_vec3_norm(myDiff);
            if (std::abs(diff.v[0]) < std::abs(diff.v[2])) {
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
    }

    // TODO: lerp inputDirection
    inputDirection.v[1] = 0.f;
    t3d_vec3_norm(inputDirection);
    t3d_vec3_scale(inputDirection, inputDirection, ForceLimit);
}