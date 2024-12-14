/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#pragma once
#include <t3d/t3d.h>
#include <t3d/tpx.h>

#include "ptSystem.h"

class PTSprites
{
  private:
    PTSystem systems[4]{
      PTSystem(128), PTSystem(128),
      PTSystem(128), PTSystem(128),
    };

    sprite_t *sprite{};
    rspq_block_t *setupDPL{};
    float animTimer = 0.0f;
    float simTimer = 0.0f;
    uint16_t mirrorPt = 32;
    color_t color;

    PTSystem* getBySection(float sectionX);
  public:
    explicit PTSprites(const char* spritePath, bool isRotating = false);
    ~PTSprites();

    void setColor(color_t newColor) { this->color = newColor; }
    [[nodiscard]] color_t getColor() const { return color; }

    void add(const T3DVec3 &pos, uint32_t seed, color_t col, float scale = 1.0f);
    void add(const T3DVec3 &pos, uint32_t seed, float scale = 1.0f) {
      add(pos, seed, color, scale);
    }

    void draw(float deltaTime);
    void clear();

    void simulateDust(float deltaTime);
};