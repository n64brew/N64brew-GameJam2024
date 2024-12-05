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

        playerData.reserve(PlayerCount);
        playerData.emplace_back(Player {{-100,0,0}, PLAYER_1, model.get(), shadowModel.get()});
        playerData.emplace_back(Player {{0,0,-100}, PLAYER_2, model.get(), shadowModel.get()});
        playerData.emplace_back(Player {{100,0,0}, PLAYER_3, model.get(), shadowModel.get()});
        playerData.emplace_back(Player {{0,0,100}, PLAYER_4, model.get(), shadowModel.get()});

        newRound();
    }

void GameplayController::simulatePhysics(
    Player &player,
    uint32_t id,
    float deltaTime,
    T3DVec3 &inputDirection
)
{
    player.prevPos = player.pos;

    // Temperature
    player.temperature -= deltaTime * CooldownPerSecond;
    if (player.temperature < 0) player.temperature = 0;


    float strength = t3d_vec3_len(inputDirection);
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
        T3DVec3 force = player.velocity;
        t3d_vec3_norm(force);
        t3d_vec3_scale(force, force, -ForceLimit);

        T3DVec3 newAccel = {0};
        // a = F/m
        t3d_vec3_scale(newAccel, force, PlayerInvMass);
        t3d_vec3_add(newAccel, player.accel, newAccel);

        T3DVec3 velocityTarget = {0};
        t3d_vec3_scale(velocityTarget, newAccel, deltaTime);
        t3d_vec3_add(velocityTarget, player.velocity, velocityTarget);

        if (t3d_vec3_dot(velocityTarget, player.velocity) < 0) {
            player.velocity = {0};
            player.accel = {0};
        } else {
            player.accel = newAccel;
            player.velocity = velocityTarget;
        }

        // Animation
        t3d_anim_set_playing(player.animWalk.get(), false);
    } else  {
        // Physics
        player.direction = t3d_lerp_angle(player.direction, -atan2f(inputDirection.v[0], inputDirection.v[2]), 0.5f);

        T3DVec3 newAccel = {0};
        // a = F/m
        t3d_vec3_scale(newAccel, force, PlayerInvMass);
        t3d_vec3_add(newAccel, player.accel, newAccel);

        T3DVec3 velocityTarget = {0};
        t3d_vec3_scale(velocityTarget, newAccel, deltaTime);
        t3d_vec3_add(velocityTarget, player.velocity, velocityTarget);

        float speedLimit = strength * SpeedLimit / ForceLimit;
        if (t3d_vec3_len(velocityTarget) > speedLimit) {
            T3DVec3 accelDiff = {0};
            t3d_vec3_diff(accelDiff, velocityTarget, player.velocity);
            t3d_vec3_scale(accelDiff, accelDiff, 1.0f/deltaTime);
            t3d_vec3_add(newAccel, player.accel, accelDiff);

            t3d_vec3_norm(velocityTarget);
            t3d_vec3_scale(velocityTarget, velocityTarget, speedLimit);
        }
        player.accel = newAccel;
        player.velocity = velocityTarget;

        // Animation
        t3d_anim_set_playing(player.animWalk.get(), true);
        t3d_anim_set_speed(player.animWalk.get(), 2.f * t3d_vec3_len(velocityTarget) / SpeedLimit);
    }

    T3DVec3 posDiff = {0};
    t3d_vec3_scale(posDiff, player.velocity, deltaTime);

    t3d_vec3_add(player.pos, player.pos, posDiff);

    if (player.pos.v[0] > map->getHalfSize()) player.pos.v[0] = map->getHalfSize();
    if (player.pos.v[0] < -map->getHalfSize()) player.pos.v[0] = -map->getHalfSize();
    if (player.pos.v[2] > map->getHalfSize()) player.pos.v[2] = map->getHalfSize();
    if (player.pos.v[2] < -map->getHalfSize()) player.pos.v[2] = -map->getHalfSize();

    player.accel = {0};
}

void GameplayController::handleFire(Player &player, uint32_t id, Direction direction) {
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
    if (state.state == STATE_PAUSED || state.state == STATE_INTRO) {
        return;
    }
    state.avPos = {0};
    int id = 0;
    for (auto& player : playerData)
    {
        Direction dir = NONE;
        if (id < (int)core_get_playercount()) {
            joypad_buttons_t pressed = joypad_get_buttons_pressed(core_get_playercontroller((PlyNum)id));

            if (pressed.c_up || pressed.d_up) {
                dir = UP;
            } else if (pressed.c_down || pressed.d_down) {
                dir = DOWN;
            } else if (pressed.c_left || pressed.d_left) {
                dir = LEFT;
            } else if (pressed.c_right || pressed.d_right) {
                dir = RIGHT;
            }
        } else {
            dir = ai.calculateFireDirection(player, deltaTime, playerData, state);
        }

        if (state.state == STATE_GAME || state.state == STATE_LAST_ONE_STANDING) handleFire(player, id, dir);
        player.render(id, viewport, deltaTime, *map);

        t3d_vec3_add(state.avPos, state.avPos, player.pos);

        id++;
    }
    t3d_vec3_scale(state.avPos, state.avPos, 0.25);

    if (state.state == STATE_GAME || state.state == STATE_LAST_ONE_STANDING) {
        bulletController.render(deltaTime);
    }
}

void GameplayController::renderUI()
{
    int i = 0;
    for (auto& player : playerData)
    {
        player.renderUI(i, arrowSprite.get());
        i++;
    }
}

void GameplayController::fixedUpdate(float deltaTime, GameState &state)
{
    uint32_t id = 0;
    for (auto& player : playerData)
    {
        T3DVec3 direction = {0};
        if (id < core_get_playercount()) {
            joypad_inputs_t joypad = joypad_get_inputs(core_get_playercontroller((PlyNum)id));
            direction.v[0] = (float)joypad.stick_x;
            direction.v[2] = -(float)joypad.stick_y;
        } else {
            ai.calculateMovement(player, deltaTime, playerData, state, direction);
        }
        simulatePhysics(player, id, deltaTime, direction);
        id++;
    }

    if (state.state == STATE_GAME || state.state == STATE_LAST_ONE_STANDING) {
        bulletController.fixedUpdate(deltaTime, playerData);
    }
}

void GameplayController::newRound()
{
    std::array<T3DVec3, PlayerCount> playerPositions {
        T3DVec3{-100, 0, 0},
        T3DVec3{0, 0, -100},
        T3DVec3{100, 0, 0},
        T3DVec3{0, 0, 100}
    };

    auto rd = std::random_device {}; 
    auto rng = std::default_random_engine { rd() };
    std::shuffle(std::begin(playerPositions), std::end(playerPositions), rng);

    PlyNum ply = PLAYER_1;
    for (Player &player : playerData)
    {
        // TODO: fix this
        player.direction = 0;
        player.velocity = {0};
        player.accel = {0};
        player.displayTemperature = 0;

        player.team = ply;
        player.firstHit = ply;
        player.temperature = 0;

        player.pos = playerPositions[ply];
        player.prevPos = playerPositions[ply];

        player.capturer = -1;

        ply = (PlyNum)(ply + 1);
    }
}

const std::vector<Player> &GameplayController::getPlayerData() const {
    return playerData;
}
