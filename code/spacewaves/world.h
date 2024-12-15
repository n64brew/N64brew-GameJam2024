#ifndef WORLD_H
#define WORLD_H

#include <libdragon.h>
#include <time.h>
#include <unistd.h>
#include <display.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include "gfx.h"

#define PLANETS_MAX       8

typedef struct worlddef_t{
  struct {
    color_t main, back, stars, fog;
    int galaxytype;
    float offsetstars;
    float offsettime;
    float offsetgalaxy;
    rspq_block_t* dlblock;
  } space;

  struct {
    bool enabled;
    bool rings;
    color_t main, back, city, fog;
    T3DVec3 polarpos;
    T3DMat4FP* modelMatFP;
    rspq_block_t* dlblock;
  } planets[PLANETS_MAX];

  struct{
    color_t color;
    color_t ambient;
    T3DVec3 direction;
    T3DVec3 lensflareangles;
    float lensflarealpha;
  } sun;

  T3DVec3 currcamangles;

} WorldDef;
extern WorldDef world;


float frandr( float min, float max );

color_t get_rainbow_color(float s);

extern int randm(int max);

extern int randr(int min, int max);

void world_reinit();

void world_init();

void world_draw();

void world_draw_lensflare();

void world_close();

#endif