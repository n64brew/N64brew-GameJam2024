#include "./gameplay.hpp"

GameplayController::GameplayController(std::shared_ptr<MapRenderer> map, std::shared_ptr<UIRenderer> ui) :
    bulletController(map, ui),
    model({
        t3d_model_load("rom:/paintball/char.t3dm"),
        t3d_model_free
    }),
    shadowModel({
        t3d_model_load("rom:/paintball/shadow.t3dm"),
        t3d_model_free
    }),
    arrowSprite {sprite_load("rom:/paintball/arrow.ia4.sprite"), sprite_free},
    map(map)
    {
        assertf(model.get(), "Player model is null");

        playerOtherData.reserve(PlayerCount);

        playerOtherData.emplace_back(Player::OtherData {model.get(), shadowModel.get()});
        playerOtherData.emplace_back(Player::OtherData {model.get(), shadowModel.get()});
        playerOtherData.emplace_back(Player::OtherData {model.get(), shadowModel.get()});
        playerOtherData.emplace_back(Player::OtherData {model.get(), shadowModel.get()});

        playerGameplayData.reserve(PlayerCount);
        playerGameplayData.emplace_back(Player::GameplayData {{-100,0,0}, PLAYER_1});
        playerGameplayData.emplace_back(Player::GameplayData {{0,0,-100}, PLAYER_2});
        playerGameplayData.emplace_back(Player::GameplayData {{100,0,0}, PLAYER_3});
        playerGameplayData.emplace_back(Player::GameplayData {{0,0,100}, PLAYER_4});

        newRound();
    }

void GameplayController::simulatePhysics(
    Player::GameplayData &gameplay,
    Player::OtherData &otherData,
    uint32_t id,
    float deltaTime,
    T3DVec3 &inputDirection
)
{
    gameplay.prevPos = gameplay.pos;

    if (id < MAXPLAYERS && id < core_get_playercount()) {
        // Temperature
        gameplay.temperature -= deltaTime * CooldownPerSecond;
        if (gameplay.temperature < 0) gameplay.temperature = 0;


        float strength = sqrtf(t3d_vec3_len2(inputDirection));
        if (strength > ForceLimit) {
            strength = ForceLimit;
        }

        t3d_vec3_norm(inputDirection);

        T3DVec3 force = {0};
        t3d_vec3_scale(force, inputDirection, strength);

        // TODO: move into player.cpp?
        // Deadzone
        if (strength < 10.0f) {
            // Physics
            T3DVec3 force = otherData.velocity;
            t3d_vec3_norm(force);
            t3d_vec3_scale(force, force, -ForceLimit);

            T3DVec3 newAccel = {0};
            // a = F/m
            t3d_vec3_scale(newAccel, force, PlayerInvMass);
            t3d_vec3_add(newAccel, otherData.accel, newAccel);

            T3DVec3 velocityTarget = {0};
            t3d_vec3_scale(velocityTarget, newAccel, deltaTime);
            t3d_vec3_add(velocityTarget, otherData.velocity, velocityTarget);

            if (t3d_vec3_dot(velocityTarget, otherData.velocity) < 0) {
                otherData.velocity = {0};
                otherData.accel = {0};
            } else {
                otherData.accel = newAccel;
                otherData.velocity = velocityTarget;
            }

            // Animation
            t3d_anim_set_playing(otherData.animWalk.get(), false);
        } else  {
            // Physics
            otherData.direction = t3d_lerp_angle(otherData.direction, -atan2f(inputDirection.v[0], inputDirection.v[2]), 0.5f);

            T3DVec3 newAccel = {0};
            // a = F/m
            t3d_vec3_scale(newAccel, force, PlayerInvMass);
            t3d_vec3_add(newAccel, otherData.accel, newAccel);

            T3DVec3 velocityTarget = {0};
            t3d_vec3_scale(velocityTarget, newAccel, deltaTime);
            t3d_vec3_add(velocityTarget, otherData.velocity, velocityTarget);

            float speedLimit = strength * SpeedLimit / ForceLimit;
            if (t3d_vec3_len(velocityTarget) > speedLimit) {
                T3DVec3 accelDiff = {0};
                t3d_vec3_diff(accelDiff, velocityTarget, otherData.velocity);
                t3d_vec3_scale(accelDiff, accelDiff, 1.0f/deltaTime);
                t3d_vec3_add(newAccel, otherData.accel, accelDiff);

                t3d_vec3_norm(velocityTarget);
                t3d_vec3_scale(velocityTarget, velocityTarget, speedLimit);
            }
            otherData.accel = newAccel;
            otherData.velocity = velocityTarget;

            // Animation
            t3d_anim_set_playing(otherData.animWalk.get(), true);
            t3d_anim_set_speed(otherData.animWalk.get(), 2.f * t3d_vec3_len(velocityTarget) / SpeedLimit);
        }

        T3DVec3 posDiff = {0};
        t3d_vec3_scale(posDiff, otherData.velocity, deltaTime);

        t3d_vec3_add(gameplay.pos, gameplay.pos, posDiff);

        if (gameplay.pos.v[0] > map->getHalfSize()) gameplay.pos.v[0] = map->getHalfSize();
        if (gameplay.pos.v[0] < -map->getHalfSize()) gameplay.pos.v[0] = -map->getHalfSize();
        if (gameplay.pos.v[2] > map->getHalfSize()) gameplay.pos.v[2] = map->getHalfSize();
        if (gameplay.pos.v[2] < -map->getHalfSize()) gameplay.pos.v[2] = -map->getHalfSize();

        otherData.accel = {0};
    } else {
        // AI
        t3d_anim_set_playing(otherData.animWalk.get(), false);
    }
}

void GameplayController::handleFire(Player::GameplayData &player, uint32_t id, Direction direction) {
    if (player.temperature > 1.f) return;

    auto position = T3DVec3{player.pos.v[0], BulletHeight, player.pos.v[2]};

    bool fired = false;
    if (direction == UP) {
        t3d_vec3_add(position, position, T3DVec3 {0, 0, -BulletOffset});
        bulletController.fireBullet(position, T3DVec3{0, 0, -BulletVelocity}, (PlyNum)id, player.team);
        fired = true;
    } else if (direction == DOWN) {
        t3d_vec3_add(position, position, T3DVec3 {0, 0, BulletOffset});
        bulletController.fireBullet(position, T3DVec3 {0, 0, BulletVelocity}, (PlyNum)id, player.team);
        fired = true;
    } else if (direction == LEFT) {
        t3d_vec3_add(position, position, T3DVec3 {-BulletOffset, 0, 0});
        bulletController.fireBullet(position, T3DVec3 {-BulletVelocity, 0, 0}, (PlyNum)id, player.team);
        fired = true;
    } else if (direction == RIGHT) {
        t3d_vec3_add(position, position, T3DVec3 {BulletOffset, 0, 0});
        bulletController.fireBullet(position, T3DVec3 {BulletVelocity, 0, 0}, (PlyNum)id, player.team);
        fired = true;
    }

    if (fired) {
        player.temperature += TempPerBullet;
        // Penalize if player is overheating
        if (player.temperature > 1.f) player.temperature = 1.f + OverheatPenalty;
    }
}


// TODO: stop passing state around
// TODO: remove viewport from here, move to UI
void GameplayController::render(float deltaTime, T3DViewport &viewport, GameState &state)
{
    state.avPos = {0};
    int id = 0;
    for (auto& playerOther : playerOtherData)
    {
        auto& playerGameplay = playerGameplayData[id];

        Direction dir = NONE;
        if (id < (int)core_get_playercount()) {
            joypad_buttons_t pressed = joypad_get_buttons_pressed(core_get_playercontroller((PlyNum)id));

            // TODO: implement basic pause menu
            if (pressed.start) minigame_end();

            if (state.state != STATE_COUNTDOWN) {
                if (pressed.c_up || pressed.d_up) {
                    dir = UP;
                } else if (pressed.c_down || pressed.d_down) {
                    dir = DOWN;
                } else if (pressed.c_left || pressed.d_left) {
                    dir = LEFT;
                } else if (pressed.c_right || pressed.d_right) {
                    dir = RIGHT;
                }
            }
        } else {
            dir = ai.calculateFireDirection(playerGameplay, playerOther, deltaTime);
        }

        handleFire(playerGameplay, id, dir);
        Player::render(playerGameplay, playerOther, id, viewport, deltaTime);

        t3d_vec3_add(state.avPos, state.avPos, playerGameplay.pos);

        id++;
    }
    t3d_vec3_scale(state.avPos, state.avPos, 0.25);
    bulletController.render(deltaTime);
}

void GameplayController::renderUI()
{
    int i = 0;
    for (auto& playerOther : playerOtherData)
    {
        auto& playerGameplay = playerGameplayData[i];
        Player::renderUI(playerGameplay, playerOther, i, arrowSprite.get());
        i++;
    }
}

void GameplayController::fixedUpdate(float deltaTime, GameState &state)
{
    uint32_t id = 0;
    for (auto& player : playerOtherData)
    {
        auto& gameplayData = playerGameplayData[id];

        T3DVec3 direction = {0};
        if (id < core_get_playercount()) {
            joypad_inputs_t joypad = joypad_get_inputs(core_get_playercontroller((PlyNum)id));
            direction.v[0] = (float)joypad.stick_x;
            direction.v[2] = -(float)joypad.stick_y;
        } else {
            ai.calculateMovement(gameplayData, player, deltaTime, direction);
        }

        simulatePhysics(gameplayData, player, id, deltaTime, direction);
        id++;
    }

    bulletController.fixedUpdate(deltaTime, playerGameplayData);
}

void GameplayController::newRound()
{
    PlyNum ply = PLAYER_1;
    for (Player::GameplayData &player : playerGameplayData)
    {
        auto& playerOther = playerOtherData[ply];

        // TODO: fix this
        playerOther.direction = 0;
        playerOther.velocity = {0};
        playerOther.accel = {0};
        playerOther.displayTemperature = 0;

        player.team = ply;
        player.firstHit = ply;
        player.fragCount = 0;
        player.temperature = 0;

        ply = (PlyNum)(ply + 1);
    }

    playerGameplayData[0].pos = {-100, 0, 0};
    playerGameplayData[0].prevPos = {-100, 0, 0};

    playerGameplayData[1].pos = {0, 0, -100};
    playerGameplayData[1].prevPos = {0, 0, -100};

    playerGameplayData[2].pos = {100, 0, 0};
    playerGameplayData[2].prevPos = {100, 0, 0};

    playerGameplayData[3].pos = {0, 0, 100};
    playerGameplayData[3].prevPos = {0, 0, 100};
}

const std::vector<Player::GameplayData> &GameplayController::getPlayerGameplayData() const {
    return playerGameplayData;
}
