#ifndef UI_COLORS_H
#define UI_COLORS_H

#ifdef __cplusplus
extern "C"
{
#endif

    enum COLOR_NAMES
    {
        // Standard ROYGBIV
        RED,
        ORANGE,
        YELLOW,
        GREEN,
        BLUE,
        INDIGO,
        VIOLET,
        // RGB 0 (full black) & 1 (full white)
        BLACK,
        WHITE,
        // RGB 1 * (n*.25f)
        LIGHT_GREY, // n = 3
        GREY,       // n = 2
        DARK_GREY,  // n = 1
                    // Transparent Colors
        TRANSPARENT,
        T_RED,
        T_ORANGE,
        T_YELLOW,
        T_GREEN,
        T_BLUE,
        T_INDIGO,
        T_VIOLET,
        T_BLACK,
        T_WHITE,
        T_GREY,
        // Darker Variants
        DARK_RED,
        DARK_GREEN,
        // N64 Logo Colors
        N_RED,
        N_YELLOW,
        N_GREEN,
        N_BLUE,
        COLOR_COUNT
    };

    // FMT_RGBA32, 32-bit packed RGBA (8888)
    const uint32_t COLORS[COLOR_COUNT] =
        {
            0xD90000FF, // RED
            0xFF6822FF, // ORANGE
            0xFFDA21FF, // YELLOW
            0x33DD00FF, // GREEN
            0x1133CCFF, // BLUE
            0x220066FF, // INDIGO
            0x330044FF, // VIOLET
            0x000000FF, // BLACK
            0xFFFFFFFF, // WHITE
            0xC0C0C0FF, // LIGHT_GREY
            0x808080FF, // GREY
            0x404040FF, // DARK_GREY
            0x0000007F, // TRANSPARENT
            0xD90000C8, // T_RED
            0xFF6822C8, // T_ORANGE
            0xFFDA21C8, // T_YELLOW
            0x33DD00C8, // T_GREEN
            0x1133CCC8, // T_BLUE
            0x220066C8, // T_INDIGO
            0x330044C8, // T_VIOLET
            0x1F1F1FC8, // T_BLACK
            0xFFFFFFC8, // T_WHITE
            0xC0C0C0C8, // T_GREY
            0x820000FF, // DARK_RED
            0x006400FF, // DARK_GREEN
            0xE10916FF, // N_RED
            0xF5B201FF, // N_YELLOW
            0x319900FF, // N_GREEN
            0x01009AFF, // N_BLUE
    };

    color_t ui_color(int colorIdx);
    uint32_t ui_colorSetAlpha(uint32_t color, uint8_t alpha);

    // Creates a color_t from one of the 32-bit packed COLORS.
    color_t ui_color(int colorIdx)
    {
        return color_from_packed32(COLORS[colorIdx]);
    }

    // Clears the alpha bits and sets them to the new value
    uint32_t ui_colorSetAlpha(uint32_t color, uint8_t alpha)
    {
        return (color & 0xFFFFFF00) | (alpha & 0xFF);
    }

#ifdef __cplusplus
}
#endif

#endif // UI_COLORS_H