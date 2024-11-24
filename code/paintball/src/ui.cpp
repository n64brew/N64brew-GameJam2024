#include "./ui.hpp"

UIRenderer::UIRenderer() :
    mediumFont("rom:/paintball/FingerPaint-Regular-Medium.font64", MediumFont),
    bigFont("rom:/paintball/FingerPaint-Regular-Big.font64", BigFont),
    hitSprite {sprite_load("rom:/paintball/marker.ia4.sprite"), sprite_free}
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

void UIRenderer::render(const GameState &state, T3DViewport &viewport, float deltaTime)
{
    renderHitMarks(viewport, deltaTime);

    rdpq_sync_tile();
    rdpq_sync_pipe(); // Hardware crashes otherwise
    rdpq_set_mode_standard();

    rdpq_textparms_t centerparms = {
        .style_id = 4,
        .width = ScreenWidth,
        .height = ScreenHeight,
        .align = ALIGN_CENTER,
        .valign = VALIGN_CENTER,
        .disable_aa_fix = true
    };

    if (state.state == STATE_COUNTDOWN) {
        if (state.currentRound == (RoundCount-1)) {
            rdpq_text_printf(&centerparms, MediumFont, 0, - ScreenHeight / 4, "Final Round");
        } else {
            rdpq_text_printf(&centerparms, MediumFont, 0, - ScreenHeight / 4, "Round %d", state.currentRound + 1);
        }
        rdpq_text_printf(&centerparms, BigFont, 0, 0, "%d", (int)ceilf(3.f - state.gameTime));

        rdpq_text_printf(&centerparms, SmallFont, 0, - ScreenHeight / 8, "^04Prepare to paint!");
    } else if(state.state == STATE_GAME && state.gameTime < 0.6f){
        rdpq_text_printf(&centerparms, BigFont, 0, 0, "Go!");
    } else if(state.state == STATE_LAST_ONE_STANDING){
        if(state.gameTime < 3.f) {
            rdpq_text_printf(&centerparms, MediumFont, 0, - ScreenHeight / 4, "Run to score!");
        }

        if(state.state == STATE_LAST_ONE_STANDING && state.gameTime < LastOneStandingTime){
            rdpq_textparms_t textparms3 = {
                .style_id = 4,
                .width = (int16_t)(ScreenWidth * 0.8),
                .height = (int16_t)(ScreenHeight * 0.8),
                .align = ALIGN_RIGHT,
                .valign = VALIGN_BOTTOM,
                .disable_aa_fix = true
            };

            rdpq_text_printf(&textparms3, MediumFont, ScreenWidth * 0.1, ScreenHeight * 0.1, "%d", (int)ceilf(LastOneStandingTime - state.gameTime));
        }
    } else if (state.state == STATE_WAIT_FOR_NEW_ROUND || state.state == STATE_FINISHED) {
        centerparms.style_id = state.winner;
        if (state.state == STATE_FINISHED) {
            rdpq_text_printf(&centerparms, BigFont, 0, - ScreenHeight / 3, "Game Over!");
        } else {
            rdpq_text_printf(&centerparms, BigFont, 0, - ScreenHeight / 3, "Player %d wins!", state.winner + 1);
        }

        for (int i = 0; i < MAXPLAYERS; i++) {
            centerparms.style_id = i;
            rdpq_text_printf(&centerparms, MediumFont, 0, (i-1) * 30, "Player %d: %d", i + 1, state.scores[i]);
        }
    }
}

void UIRenderer::renderHitMarks(T3DViewport &viewport, float deltaTime) {
    for (auto hit = hits.begin(); hit < hits.end(); ++hit) {
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

        const color_t colors[] = {
            PLAYERCOLOR_1,
            PLAYERCOLOR_2,
            PLAYERCOLOR_3,
            PLAYERCOLOR_4,
        };

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