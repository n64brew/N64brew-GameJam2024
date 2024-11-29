#include "ai.hpp"

Direction AI::calculateFireDirection(Player& player, float deltaTime) {
    return Direction::NONE;
}

void AI::calculateMovement(Player& player, float deltaTime, T3DVec3 &inputDirection) {
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
        // t3d_vec3_scale(diffPerp, diffPerp, ForceLimit);

        if (t3d_vec3_len(diffPerp) == 0.f) [[unlikely]] {
            // rotate clockwise
            inputDirection.v[0] = -bulletVelocityDir.v[2];
            inputDirection.v[2] = bulletVelocityDir.v[0];
        } else {
            // TODO: increase strength if close by
            t3d_vec3_add(inputDirection, inputDirection, diffPerp);
        }
    }
    player.incomingBullets.clear();
    inputDirection.v[1] = 0.f;
    t3d_vec3_scale(inputDirection, inputDirection, ForceLimit);
}