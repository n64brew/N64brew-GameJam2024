#include "ai.hpp"

AI::AI() : aiActionTimer(0) {
    difficulty = core_get_aidifficulty();
}

Direction AI::calculateFireDirection(Player& player, float deltaTime, std::vector<Player> &players, GameState &state) {
    aiActionTimer += deltaTime;

    float actionRate = AIActionRateSecond;
    float tempControl = 1.f;
    if (difficulty == AiDiff::DIFF_EASY) {
        if (player.aiState != AIState::AI_ATTACK) {
            return Direction::NONE;
        }
        actionRate = AIActionRateSecond * 2.f;
    } else if (difficulty == AiDiff::DIFF_MEDIUM) {
        if (player.aiState == AIState::AI_RUN) {
            return Direction::NONE;
        }
        actionRate = AIActionRateSecond * 1.5f;
    } else if (difficulty == AiDiff::DIFF_HARD) {
        actionRate = AIActionRateSecond;
        // Hard can barely overheat
        tempControl = (player.aiState == AIState::AI_ATTACK) ? CooldownPerSecond : 0.4f;
    }

    if (aiActionTimer < actionRate) {
        return Direction::NONE;
    }
    aiActionTimer = 0;

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

        float random = static_cast<float>(rand()) / RAND_MAX;
        float missFactorSeconds = 0.f;
        bool shouldMiss = false;
        if (difficulty == AiDiff::DIFF_EASY) {
            missFactorSeconds = random * 0.5f;
            if (random < 0.7f && other.team != player.team) {
                shouldMiss = true;
            }
        } else if (difficulty == AiDiff::DIFF_MEDIUM) {
            missFactorSeconds = random * 0.25f;
            if (random < 0.45f && other.team != player.team) {
                shouldMiss = true;
            }
        }

        if (other.velocity.v[0] != 0.f) {
            float enemyXTime = (diff.v[0]/other.velocity.v[0]);
            float bulletYTime = std::abs(diff.v[2] / BulletVelocity);

            if (enemyXTime >= 0 && (enemyXTime - bulletYTime) < (PlayerRadius/BulletVelocity + missFactorSeconds)) {
                if (diff.v[2] > 0) {
                    if (shouldMiss) return Direction::DOWN;
                    return Direction::UP;
                } else {
                    if (shouldMiss) return Direction::UP;
                    return Direction::DOWN;
                }
            }
        }

        if (other.velocity.v[2] != 0.f) {
            float enemyYTime = (diff.v[2]/other.velocity.v[2]);
            float bulletXTime = std::abs(diff.v[0] / BulletVelocity);

            if (enemyYTime >= 0 && (enemyYTime - bulletXTime) < (PlayerRadius/BulletVelocity + missFactorSeconds)) {
                if (diff.v[0] > 0) {
                    if (shouldMiss) return Direction::RIGHT;
                    return Direction::LEFT;
                } else {
                    if (shouldMiss) return Direction::LEFT;
                    return Direction::RIGHT;
                }
            }
        }

        if (std::abs(diff.v[0]) < (PlayerRadius + missFactorSeconds*SpeedLimit) && t3d_vec3_len(diff) < AIFarRange) {
            if (diff.v[2] > 0) {
                if (shouldMiss) return Direction::DOWN;
                return Direction::UP;
            } else {
                if (shouldMiss) return Direction::UP;
                return Direction::DOWN;
            }
        }

        if (std::abs(diff.v[2]) < (PlayerRadius + missFactorSeconds*SpeedLimit) && t3d_vec3_len(diff) < AIFarRange) {
            if (diff.v[0] > 0) {
                if (shouldMiss) return Direction::RIGHT;
                return Direction::LEFT;
            } else {
                if (shouldMiss) return Direction::LEFT;
                return Direction::RIGHT;
            }
        }
    }

    return Direction::NONE;
}

void AI::tryChangeState(Player& player, AIState newState) {
    float random = static_cast<float>(rand()) / RAND_MAX;

    float unstability = AIUnstable;
    if (difficulty == AiDiff::DIFF_EASY) {
        // More difficult to change state to converge on the "better" strat
        unstability *= 0.5f;
    } else if (difficulty == AiDiff::DIFF_MEDIUM) {
        unstability *= 0.7f;
    }

    if (random > unstability) {
        return;
    }

    player.aiState = newState;
    player.multiplier = 1.f + AIRandomRange * (static_cast<float>(rand()) / RAND_MAX);
    player.multiplier2 = 1.f + AIRandomRange * (static_cast<float>(rand()) / RAND_MAX);
}

void AI::calculateMovement(Player& player, float deltaTime, std::vector<Player> &players, GameState &state, T3DVec3 &inputDirection) {
    float random = static_cast<float>(rand()) / RAND_MAX;

    // Defaults
    float escapeWeight = 100.f;

    float centerAttraction = 0.01f;
    float randomWeight = 0.1f;

    // TODO: add bias based on player teams
    float playerAttraction = 0.f;
    float playerRepulsion = 0.f;

    float alignment = 0.1f;

    if (difficulty == AiDiff::DIFF_EASY) {
        escapeWeight = 1.f;
        alignment *= 0.f;
    } else if (difficulty == AiDiff::DIFF_MEDIUM) {
        escapeWeight = 6.f;
        alignment *= 0.5f;
    }

    if (random < AITemperature) {
        int r = randomRange(1, 3);
        player.aiState = (AIState)r;
    }

    if (state.state == State::STATE_LAST_ONE_STANDING) {
        if (state.winner == player.team) {
            tryChangeState(player, AIState::AI_ATTACK);
        } else {
            tryChangeState(player, AIState::AI_DEFEND);
        }
    } else if (state.state == State::STATE_WAIT_FOR_NEW_ROUND || state.state == State::STATE_FINISHED) {
        tryChangeState(player, AIState::AI_RUN);
    } else {
        if (player.temperature > 1.f || player.firstHit != player.team) {
            tryChangeState(player, AIState::AI_DEFEND);
        } else {
            tryChangeState(player, AIState::AI_ATTACK);
        }
    }

    if (player.aiState == AIState::AI_ATTACK) {
        centerAttraction = 0.1f;

        playerAttraction = 0.5f * player.multiplier;
        playerRepulsion = 0.3f * player.multiplier2;

        alignment *= player.multiplier;
    } else if (player.aiState == AIState::AI_DEFEND) {
        centerAttraction = 0.4f;

        playerAttraction = 0.2f * player.multiplier2;
        playerRepulsion = 0.5f * player.multiplier;

        alignment *= -player.multiplier;
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
        if (t3d_vec3_len(diff) > AICloseRange) {
            T3DVec3 myDiff = diff;
            float scale = t3d_vec3_len(diff) / AICloseRange;
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
        if (t3d_vec3_len(diff) > 0.f && t3d_vec3_len(diff) < AIFarRange) {
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