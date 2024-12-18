#ifndef PLAYER_H
#define PLAYER_H

typedef struct
{

    Vector3 position;
    uint8_t id;
    uint8_t actor_id;
    uint8_t score;
    bool died;
    bool isHuman;
    bool deathCounted;
    ControllerData control;

} Player;

void player_init(Player *player, uint8_t id, uint8_t actor_id);

void player_init(Player *player, uint8_t id, uint8_t actor_id)
{
    player->position = (Vector3){0, 0, 0};
    player->id = id;
    player->actor_id = actor_id;
    player->died = false;
    player->isHuman = true;
    player->deathCounted = false;
}

void player_setControlData(Player *player)
{
    joypad_poll();

    for (int i = 0; i < PLAYER_COUNT; i++)
    {
        if (i != 0 && player[i].died)
            continue; // Allow player one to still pause when eliminated
        controllerData_getInputs(&player[i].control, i);
    }
}

// Modified from `player_draw_billboard` in Snake3D example
Vector3 player_getBillboard(Player *player, T3DViewport *viewport)
{
    Vector3 result = (Vector3){0, 0, 0};
    if (player->died)
        return result;

    Vector3 billboardPos = (Vector3){
        player->position.x,
        player->position.y,
        player->position.z + 200.0f};

    T3DVec3 billboardPosConvert = Vector3_to_T3DVec3(billboardPos);

    T3DVec3 billboardScreenPos;
    t3d_viewport_calc_viewspace_pos(viewport, &billboardScreenPos, &billboardPosConvert);

    int x = fm_floorf(billboardScreenPos.v[0]);
    int y = fm_floorf(billboardScreenPos.v[2]);
    int z = fm_floorf(billboardScreenPos.v[1]);

    result = (Vector3){x, y, z};
    return result;
}

void player_drawShadow(Vector3 position, T3DViewport *viewport)
{

    Vector3 shadowPos = (Vector3){
        position.x,
        position.y,
        325 // @TODO: in a bigger game, I'd probably just raycasst for the floor Z
    };

    T3DVec3 playerPosConvert = Vector3_to_T3DVec3(position);
    T3DVec3 shadowPosConvert = Vector3_to_T3DVec3(shadowPos);

    T3DVec3 playerScreenPos;
    T3DVec3 shadowScreenPos;
    t3d_viewport_calc_viewspace_pos(viewport, &playerScreenPos, &playerPosConvert);
    t3d_viewport_calc_viewspace_pos(viewport, &shadowScreenPos, &shadowPosConvert);

    int offset = 3;
    float v1[] = {playerScreenPos.x, playerScreenPos.y - offset};
    float v2[] = {shadowScreenPos.x - offset, shadowScreenPos.y};
    float v3[] = {shadowScreenPos.x + offset, shadowScreenPos.y};
    float v4[] = {shadowScreenPos.x, shadowScreenPos.y + offset};

    rdpq_sync_pipe();
    rdpq_set_mode_standard();
    rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
    rdpq_set_prim_color(RGBA32(0, 0, 0, 127));

    // Draw rhombus shape with 2 tris
    rdpq_triangle(&TRIFMT_FILL, v1, v2, v3);
    rdpq_triangle(&TRIFMT_FILL, v3, v4, v2);
}
#endif