#include "./ui.hpp"

UIRenderer::UIRenderer() :
    font("rom:/paintball/FingerPaint-Regular.font64", BigFont)
{
    auto fnt = font.font.get();
    assertf(fnt, "BigFont is null");

    rdpq_fontstyle_t style = { .color = RGBA32(255, 0, 0, 255) };

    rdpq_font_style(fnt, 0, &style);
}

void UIRenderer::render(const GameState &state)
{
    rdpq_sync_tile();
    rdpq_sync_pipe(); // Hardware crashes otherwise

    if (state.isCountdown) {
        rdpq_textparms_t textparms = { .width = ScreenWidth, .height = ScreenHeight, .align = ALIGN_CENTER, .valign = VALIGN_CENTER };
        rdpq_text_printf(&textparms, BigFont, 0, 0, "%d", (int)ceilf(3.f - state.gameTime));

        rdpq_textparms_t textparms2 = { .width = ScreenWidth, .height = ScreenHeight / 2, .align = ALIGN_CENTER, .valign = VALIGN_CENTER };
        rdpq_text_printf(&textparms2, MainFont, 0, ScreenHeight / 2, "Start moving");
    }
}