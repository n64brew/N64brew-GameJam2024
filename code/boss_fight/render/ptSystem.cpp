/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "ptSystem.h"

PTSystem::PTSystem(uint32_t countMax)
  : countMax{countMax}, count{0}
{
  assert(sizeof(countMax) % 2 == 0);
  if(countMax > 0) {
    mat = static_cast<T3DMat4FP*>(malloc_uncached(sizeof(T3DMat4FP)));
    particles = static_cast<TPXParticle*>(malloc_uncached(countMax * sizeof(TPXParticle) / 2));
  }
}

PTSystem::~PTSystem() {
  if(countMax > 0) {
    free_uncached(mat);
    free_uncached(particles);
  }
}

void PTSystem::draw() const {
  if(count == 0)return;
  tpx_matrix_push(mat);
  uint32_t safeCount = count & ~1;
  tpx_particle_draw(particles, safeCount);
  tpx_matrix_pop(1);
}

void PTSystem::drawTextured() const {
  if(count == 0)return;
  tpx_matrix_push(mat);
  uint32_t safeCount = count & ~1;
  tpx_particle_draw_tex(particles, safeCount);
  tpx_matrix_pop(1);
}
