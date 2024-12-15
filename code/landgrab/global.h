#ifndef GAMEJAM2024_LANDGRAB_GLOBAL_H
#define GAMEJAM2024_LANDGRAB_GLOBAL_H

#include <libdragon.h>

#include "../../core.h"

// Prefer constants for display size so that the compiler can optimize
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240

// clang-format off

/**
 * @brief Iterate over all player numbers (0-3), including AI players.
 */
#define PLAYER_FOREACH(iterator_token) \
    for (\
        PlyNum iterator_token = PLAYER_1; \
        iterator_token < MAXPLAYERS; \
        iterator_token += 1 \
    )

/**
 * @brief Top-left and bottom-right corners of an axis-aligned rectangle.
 */
typedef struct { int x0; int y0; int x1; int y1; } Rect;
// clang-format on

#define plynum_is_ai(p) ((p) >= core_get_playercount ())

/**
 * Get the number of elements in a constant-size array.
 */
#define ARRAY_SIZE(a) (sizeof (a) / sizeof (a[0]))

/**
 * @brief Check if a joypad 8-way is any direction of up.
 *
 * (This should probably be included in LibDragon's joypad subsystem.)
 */
#define JOYPAD_8WAY_ANY_UP(d)                                                  \
  (d == JOYPAD_8WAY_UP || d == JOYPAD_8WAY_UP_LEFT                            \
   || d == JOYPAD_8WAY_UP_RIGHT)

/**
 * @brief Check if a joypad 8-way is any direction of down.
 *
 * (This should probably be included in LibDragon's joypad subsystem.)
 */
#define JOYPAD_8WAY_ANY_DOWN(d)                                                \
  (d == JOYPAD_8WAY_DOWN || d == JOYPAD_8WAY_DOWN_LEFT                        \
   || d == JOYPAD_8WAY_DOWN_RIGHT)

/**
 * @brief Check if a joypad 8-way is any direction of left.
 *
 * (This should probably be included in LibDragon's joypad subsystem.)
 */
#define JOYPAD_8WAY_ANY_LEFT(d)                                                \
  (d == JOYPAD_8WAY_LEFT || d == JOYPAD_8WAY_UP_LEFT                          \
   || d == JOYPAD_8WAY_DOWN_LEFT)

/**
 * @brief Check if a joypad 8-way is any direction of right.
 *
 * (This should probably be included in LibDragon's joypad subsystem.)
 */
#define JOYPAD_8WAY_ANY_RIGHT(d)                                               \
  (d == JOYPAD_8WAY_RIGHT || d == JOYPAD_8WAY_UP_RIGHT                        \
   || d == JOYPAD_8WAY_DOWN_RIGHT)

#endif // GAMEJAM2024_LANDGRAB_GLOBAL_H
