#include "./ui.hpp"

UIRenderer::UIRenderer() :
    mediumFont("rom:/paintball/FingerPaint-Regular-Medium.font64", MediumFont),
    bigFont("rom:/paintball/FingerPaint-Regular-Big.font64", BigFont),
    hitSprite {sprite_load("rom:/paintball/marker.ia4.sprite"), sprite_free},
    // TODO: re-use existing one, this is a waste
    splash1 {sprite_load("rom:/paintball/splash1.ia4.sprite"), sprite_free},
    splash2 {sprite_load("rom:/paintball/splash2.ia4.sprite"), sprite_free},
    sfxCountdown("rom:/core/Countdown.wav64"),
    prevCountdown(0)
{
    rdpq_fontstyle_t p1Style = { .color = PLAYERCOLOR_1 };
    rdpq_fontstyle_t p2Style = { .color = PLAYERCOLOR_2 };
    rdpq_fontstyle_t p3Style = { .color = PLAYERCOLOR_3 };
    rdpq_fontstyle_t p4Style = { .color = PLAYERCOLOR_4 };

    auto fnt = mediumFont.font.get();
    assertf(fnt, "MediumFont is null");

    rdpq_fontstyle_t style = { .color = RGBA32(255, 255, 180, 255) };

    rdpq_font_style(fnt, 4, &style);
    rdpq_font_style(fnt, 0, &p1Style);
    rdpq_font_style(fnt, 1, &p2Style);
    rdpq_font_style(fnt, 2, &p3Style);
    rdpq_font_style(fnt, 3, &p4Style);

    fnt = bigFont.font.get();
    assertf(fnt, "BigFont is null");

    rdpq_font_style(fnt, 4, &style);
    rdpq_font_style(fnt, 0, &p1Style);
    rdpq_font_style(fnt, 1, &p2Style);
    rdpq_font_style(fnt, 2, &p3Style);
    rdpq_font_style(fnt, 3, &p4Style);
}

State UIRenderer::renderMenu(const State &state) {
    rdpq_textparms_t centerparms = {
        .style_id = 4,
        .width = ScreenWidth,
        .height = ScreenHeight,
        .align = ALIGN_CENTER,
        .valign = VALIGN_CENTER,
        .wrap = WRAP_WORD,
    };

    joypad_buttons_t pressed = joypad_get_buttons_pressed(core_get_playercontroller(PLAYER_1));

    if (state == STATE_INTRO) {
        rdpq_sync_pipe();
        rdpq_sync_tile();
        rdpq_set_mode_standard();

        rdpq_mode_zbuf(false, false);
        rdpq_mode_alphacompare(1);
        rdpq_mode_combiner(RDPQ_COMBINER1((ZERO, ZERO, ZERO, PRIM), (ZERO, ZERO, ZERO, TEX0)));

        rdpq_blitparms_t params {
            .cx = 16,
            .cy = 16,
            .scale_x = 5,
            .scale_y = 5,
        };

        rdpq_set_prim_color(RGBA32(255, 0, 0, 255));
        rdpq_sprite_blit(splash2.get(), ScreenWidth / 4.f, ScreenWidth / 4.f, &params);

        rdpq_set_prim_color(RGBA32(255, 255, 0, 255));
        rdpq_sprite_blit(splash1.get(), ScreenWidth / 4.f, ScreenWidth / 4.f, &params);

        centerparms.width = ScreenWidth * 9.f/10.f;
        rdpq_text_printf(&centerparms, BigFont, 4 + ScreenHeight / 20.f, - ScreenHeight / 4.f, "Paint^00b^01a^03l^02l");
        rdpq_text_printf(&centerparms, SmallFont, 4 + ScreenHeight / 20.f, ScreenHeight / 10.f,
            "- Analog stick to move & C/D pad to shoot"
            "\n- Capture enemies by painting them!"
            "\n- A capture for the winning team = 1 point"
            "\n- Winning color gets +1 and last one standing can escape to prevent this for +2"
            "\n\nPress ^00START^04");

        if (pressed.start) {
            return STATE_COUNTDOWN;
        }

        return STATE_INTRO;
    } else if (state == STATE_PAUSED) {
        rdpq_text_printf(&centerparms, BigFont, 0, - ScreenHeight / 4, "Paused");
        rdpq_text_printf(&centerparms, selectedMenuItem == MENU_PLAY ? MediumFont : SmallFont, 0, 0, "Continue");
        rdpq_text_printf(&centerparms, selectedMenuItem == MENU_EXIT ? MediumFont : SmallFont, 0, ScreenHeight / 8, "Abandon");

        if (pressed.a || pressed.b) {
            switch(selectedMenuItem) {
                case MENU_PLAY:
                    return pausedState;
                case MENU_EXIT:
                    minigame_end();
            }
        } else if (pressed.c_up || pressed.d_up || pressed.c_down || pressed.d_down) {
            selectedMenuItem == MENU_PLAY ? selectedMenuItem = MENU_EXIT : selectedMenuItem = MENU_PLAY;
            return state;
        }
    } else if (pressed.start) {
        pausedState = state;
        selectedMenuItem = MENU_PLAY;
        return STATE_PAUSED;
    }
    return state;
}

void UIRenderer::render(GameState &state, T3DViewport &viewport, float deltaTime)
{
    rdpq_textparms_t centerparms = {
        .style_id = 4,
        .width = ScreenWidth,
        .height = ScreenHeight,
        .align = ALIGN_CENTER,
        .valign = VALIGN_CENTER,
    };

    renderHitMarks(viewport, deltaTime);

    rdpq_sync_tile();
    rdpq_sync_pipe(); // Hardware crashes otherwise
    rdpq_set_mode_standard();

    state.state = renderMenu(state.state);

    if (state.state == STATE_COUNTDOWN) {
        if (state.currentRound == (RoundCount-1)) {
            rdpq_text_printf(&centerparms, MediumFont, 0, - ScreenHeight / 4, "Final Round");
        } else {
            rdpq_text_printf(&centerparms, MediumFont, 0, - ScreenHeight / 4, "Round %d", state.currentRound + 1);
        }
        int countdown = (int)ceilf(3.f - state.timeInState);
        if (countdown != prevCountdown) {
            wav64_play(sfxCountdown.get(), GeneralPurposeAudioChannel);
            prevCountdown = countdown;
        }
        rdpq_text_printf(&centerparms, BigFont, 0, 0, "%d", (int)ceilf(3.f - state.timeInState));

        rdpq_text_printf(&centerparms, SmallFont, 0, - ScreenHeight / 8, "^04Prepare to paint!");
    } else if(state.state == STATE_GAME && state.timeInState < 0.6f){
        rdpq_text_printf(&centerparms, BigFont, 0, 0, "Go!");
    } else if(state.state == STATE_LAST_ONE_STANDING){
        if(state.timeInState < 3.f) {
            rdpq_text_printf(&centerparms, MediumFont, 0, - ScreenHeight / 4, "Final stand!");
            rdpq_text_printf(&centerparms, SmallFont, 0, - ScreenHeight / 8, "^04Don't let them escape!");
        }

        if(state.state == STATE_LAST_ONE_STANDING && state.timeInState < LastOneStandingTime){
            rdpq_textparms_t textparms3 = {
                .style_id = 4,
                .width = (int16_t)(ScreenWidth * 0.8),
                .height = (int16_t)(ScreenHeight * 0.8),
                .align = ALIGN_RIGHT,
                .valign = VALIGN_BOTTOM,
            };

            rdpq_text_printf(&textparms3, MediumFont, ScreenWidth * 0.1, ScreenHeight * 0.1, "%d", (int)ceilf(LastOneStandingTime - state.timeInState));
        }
    } else if (state.state == STATE_FINISHED) {
        centerparms.style_id = state.winnerCount > 1 ? 4 : state.winner;
        rdpq_text_printf(&centerparms, BigFont, 0, - ScreenHeight / 3, state.winnerCount > 1 ? "Draw!" : "Winner!");

        for (int i = 0; i < MAXPLAYERS; i++) {
            centerparms.style_id = i;
            rdpq_text_printf(&centerparms, MediumFont, 0, (i-1) * 30, "Player %d: %d", i + 1, state.scores[i]);
        }
    } else if (state.state == STATE_WAIT_FOR_NEW_ROUND) {
        rdpq_text_printf(&centerparms, BigFont, 0, - ScreenHeight / 3, "Player %d wins!", state.winner + 1);

        for (int i = 0; i < MAXPLAYERS; i++) {
            centerparms.style_id = i;
            rdpq_text_printf(&centerparms, MediumFont, 0, (i-1) * 30, "Player %d: %d", i + 1, state.scores[i]);
        }
    }

    if(state.state != STATE_WAIT_FOR_NEW_ROUND && state.state != STATE_FINISHED && state.state != STATE_INTRO){
        rdpq_text_printf(&centerparms, SmallFont, -5 * ScreenWidth/16, 3 * ScreenHeight / 8, "%d/%d", state.currentRound + 1, RoundCount);
        for (int i = 0; i < MAXPLAYERS; i++) {
            centerparms.style_id = i;
            rdpq_text_printf(&centerparms, SmallFont, ((i-1) * 2 - 1) * ScreenWidth/16, 3 * ScreenHeight / 8, "P%d: %d", i + 1, state.scores[i]);
        }
    }
}

void UIRenderer::renderHitMarks(T3DViewport &viewport, float deltaTime) {
    // TODO: shouldn't need colors here
    const color_t colors[] = {
        PLAYERCOLOR_1,
        PLAYERCOLOR_2,
        PLAYERCOLOR_3,
        PLAYERCOLOR_4,
    };

    for (auto hit = hits.begin(); hit != hits.end(); ++hit) {
        if (hit->lifetime <= 0.) {
            hits.remove(hit);
            continue;
        }

        hit->lifetime -= deltaTime;

        T3DVec3 screenPos;
        t3d_viewport_calc_viewspace_pos(viewport, screenPos, hit->pos);

        rdpq_sync_pipe();
        rdpq_sync_tile();
        rdpq_set_mode_standard();

        rdpq_mode_zbuf(false, false);
        rdpq_mode_alphacompare(1);
        rdpq_mode_combiner(RDPQ_COMBINER1((ZERO, ZERO, ZERO, PRIM), (ZERO, ZERO, ZERO, TEX0)));

        rdpq_set_prim_color(colors[hit->team]);

        rdpq_blitparms_t params {
            .width = 32,
            .height = 32,
            .cx = 16,
            .cy = 16,
        };
        rdpq_sprite_blit(hitSprite.get(), screenPos.v[0], screenPos.v[1], &params);
    }
}

void UIRenderer::registerHit(const HitMark &hit) {
    hits.add(HitMark {hit.pos, hit.team, 0.1f});
}