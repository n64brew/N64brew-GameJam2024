#include "world.h"
#include "gamestatus.h"
#include "gfx.h"

WorldDef world;

float frandr( float min, float max )
{
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
}

color_t get_rainbow_color(float s) {
  float r = fm_sinf(s + 0.0f) * 127.0f + 128.0f;
  float g = fm_sinf(s + 2.0f) * 127.0f + 128.0f;
  float b = fm_sinf(s + 4.0f) * 127.0f + 128.0f;
  return RGBA32(r, g, b, 255);
}

inline int randm(int max){
  return (rand() % max);
}

inline int randr(int min, int max){
  int range = min - max;
  return (rand() % range) + min;
}

void world_reinit(){
  world.space.main = RGBA32(randm(180), randm(180),randm(180), 0xFF);
  world.space.back = RGBA32(randm(180), randm(180),randm(180), 0xFF);
  world.space.stars = RGBA32(randr(200, 255), randr(200, 255), randr(200, 255), 0xFF);
  world.space.fog  = RGBA32(randr(50, 128), randr(50, 128), randr(50, 128), 0xFF);
  world.space.galaxytype = rand() % 3;
  world.space.offsetstars = frandr(0.0f, 100.0f);
  world.space.offsetgalaxy = frandr(0.0f, 100.0f);
  world.space.offsettime = 0.0f;

  for(int i = 0; i < PLANETS_MAX; i++){
    world.planets[i].enabled = rand() % 2;
    world.planets[i].rings = rand() % 2;
    world.planets[i].polarpos = (T3DVec3){{frandr(0.0f, 360.0f), frandr(0.0f, 360.0f), frandr(0.0f, 1.0f)}};
    world.planets[i].main = RGBA32(randr(0, 110), randr(0, 110), randr(0, 110), 0x00);
    world.planets[i].back = RGBA32(randm(255), randm(255),randm(255), (world.planets[i].rings) && (rand() % 2)? 0x3F : 0x00);
    world.planets[i].city = RGBA32(randr(200, 255), randr(200, 255), randr(200, 255), 0xFF);
    world.planets[i].fog    = RGBA32(randr(128, 255), randr(128, 255), randr(128, 255), 0xFF);

    float pscale = frandr(0.1f, 1.0f);
    T3DVec3 worldpos = gfx_worldpos_from_polar(
                world.planets[i].polarpos.v[0], 
                world.planets[i].polarpos.v[1], 
                500.0f);
    t3d_mat4fp_from_srt_euler(world.planets[i].modelMatFP,
            (float[3]){pscale, pscale, pscale},
            (float[3]){0.0f,  world.planets[i].polarpos.v[1],  world.planets[i].polarpos.v[0]},
            (float[3]){worldpos.v[0],worldpos.v[1],worldpos.v[2]});
  }

  world.sun.color = RGBA32(randr(128, 255), randr(128, 255), randr(128, 255), 0xFF);
  world.sun.lensflareangles = (T3DVec3){{frandr(0.0f, 6.28f), frandr(0.0f, 6.28f), frandr(0.0f, 6.28f)}};
  world.sun.direction = gfx_t3d_dir_from_euler2((world.sun.lensflareangles.v[0]), (world.sun.lensflareangles.v[1]));
  world.sun.lensflarealpha = 0.0f;
  t3d_vec3_norm(&world.sun.direction);

}

void world_init(){
  for(int i = 0; i < PLANETS_MAX; i++){
    world.planets[i].modelMatFP = malloc_uncached(sizeof(T3DMat4FP));
  }
  world_reinit();
}

void world_close(){
  for(int i = 0; i < PLANETS_MAX; i++){
    if(world.planets[i].modelMatFP) free_uncached(world.planets[i].modelMatFP);
  }
  if(world.space.dlblock) rspq_block_free(world.space.dlblock);
  for(int p = 0; p < PLANETS_MAX; p++)
    if(world.planets[p].dlblock)
      rspq_block_free(world.planets[p].dlblock);
}

void world_draw(){
    int i = 0;

    // Draw the model, material settings (e.g. textures, color-combiner) are handled internally
    T3DMaterial* mat =  t3d_model_get_material(models[i], "f3d.space");
    T3DObject* obj = t3d_model_get_object_by_index(models[i], 0);
    t3d_model_draw_material(mat, NULL);

    t3d_fog_set_enabled(false);
    t3d_light_set_count(0);
    color_t amb = RGBA32(0xFF,0xFF,0xFF,0xFF);
    t3d_light_set_ambient((uint8_t*)&amb);
    t3d_state_set_drawflags(T3D_FLAG_SHADED | T3D_FLAG_TEXTURED);

    rdpq_tex_multi_begin();
    surface_t surf = sprite_get_pixels(sprites[world.space.galaxytype + 2]);
    rdpq_tex_upload(TILE0, &surf, &(rdpq_texparms_t){
      .s.scale_log = 2, .t.scale_log = 2,
      .s.mirror = true, .t.mirror = true, 
      .s.repeats = REPEAT_INFINITE, .t.repeats = REPEAT_INFINITE,
      .s.translate = fwrap(world.space.offsetgalaxy,0,256), .t.translate = fwrap(world.space.offsetgalaxy,0,256)});
    surf = sprite_get_pixels(sprites[10]);
    rdpq_tex_upload(TILE1, &surf, &(rdpq_texparms_t){
      .s.scale_log = -1, .t.scale_log = -1,
      .s.mirror = true, .t.mirror = true, 
      .s.repeats = REPEAT_INFINITE, .t.repeats = REPEAT_INFINITE,
      .s.translate = fwrap(world.space.offsetstars + world.space.offsettime, 0, 256), .t.translate = fwrap(world.space.offsetstars + world.space.offsettime, 0, 256)});
    rdpq_tex_multi_end();

    rdpq_mode_mipmap(MIPMAP_NONE,0);
    rdpq_mode_zbuf(false, false);
    rdpq_mode_antialias(AA_NONE);

    rdpq_mode_combiner(RDPQ_COMBINER2(
      (TEX0, 0, PRIM, 0), 		(0,0,0,0),
      (SHADE,  0, ENV,  COMBINED),	(0,0,0,TEX1)));
          
    rdpq_mode_blender(RDPQ_BLENDER2(
      (BLEND_RGB, IN_ALPHA, IN_RGB, INV_MUX_ALPHA),
      (CYCLE1_RGB, SHADE_ALPHA, FOG_RGB, INV_MUX_ALPHA)));

    rdpq_set_prim_color(world.space.main);
    rdpq_set_env_color(world.space.back);
    rdpq_set_blend_color(world.space.stars);
    rdpq_set_fog_color(world.space.fog);
        
    if(!world.space.dlblock){
        rspq_block_begin();    
        rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
        t3d_model_draw_object(obj, NULL);
        rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
        world.space.dlblock = rspq_block_end();
    } else rspq_block_run(world.space.dlblock);

    amb = RGBA32(0,0,0,0);
    t3d_light_set_ambient((uint8_t*)&amb);
    t3d_light_set_directional(0, (uint8_t*)&world.sun.color, &world.sun.direction);
    t3d_light_set_count(1);

    i = 1;

    mat = t3d_model_get_material(models[i], "f3d.planet");
    obj = t3d_model_get_object_by_index(models[i], 0);
    t3d_model_draw_material(mat, NULL);

    rdpq_mode_combiner(RDPQ_COMBINER2(
        (PRIM, ENV, TEX0, ENV), 		(1,SHADE,ENV,0),
        (TEX1,  COMBINED, SHADE,  0),	(ONE,TEX1,ENV,SHADE)));
                            
    rdpq_mode_blender(RDPQ_BLENDER(
        (FOG_RGB, IN_ALPHA, IN_RGB, INV_MUX_ALPHA)));

          
    for(int p = 0; p < PLANETS_MAX; p++)
        if(world.planets[p].enabled){
            rdpq_set_prim_color(world.planets[p].main);
            rdpq_set_env_color(world.planets[p].back);
            rdpq_set_blend_color(world.planets[p].city);
            rdpq_set_fog_color(world.planets[p].fog);
            rdpq_mode_antialias(AA_STANDARD);
            t3d_matrix_push(world.planets[p].modelMatFP);
            rdpq_set_tile_size(TILE1, fwrap(CURRENT_TIME, 0, 256), fwrap(CURRENT_TIME,0,256), 255,255);
            if(!world.planets[p].dlblock){
                rspq_block_begin();
                rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
                t3d_model_draw_object(obj, NULL);
rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
                if(rand() % 2){
                    mat = t3d_model_get_material(models[i], "f3d.planetrings");
                    obj = t3d_model_get_object_by_index(models[i], 1);
                    t3d_model_draw_material(mat, NULL);

                    rdpq_mode_combiner(RDPQ_COMBINER1(
                      (ENV, 0, SHADE, 0), 		(ENV,0,TEX0,0)));
                    rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
                    rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
                    t3d_model_draw_object(obj, NULL);
rdpq_sync_pipe(); // Hardware crashes otherwise
        rdpq_sync_tile(); // Hardware crashes otherwise
                    mat = t3d_model_get_material(models[i], "f3d.planet");
                    obj = t3d_model_get_object_by_index(models[i], 0);
                    t3d_model_draw_material(mat, NULL);

                    rdpq_mode_combiner(RDPQ_COMBINER2(
                      (PRIM, ENV, TEX0, ENV), 		(1,SHADE,ENV,0),
                      (TEX1,  COMBINED, SHADE,  0),	(ONE,TEX1,ENV,SHADE)));
                    rdpq_mode_antialias(AA_STANDARD);
                    rdpq_mode_blender(RDPQ_BLENDER(
                      (FOG_RGB, IN_ALPHA, IN_RGB, INV_MUX_ALPHA)));
                    rdpq_set_blend_color(world.planets[p].city);
                }
                world.planets[p].dlblock = rspq_block_end();
            } else rspq_block_run(world.planets[p].dlblock);
            t3d_matrix_pop(1);
        }
    world.space.offsettime += DELTA_TIME;
    t3d_light_set_ambient((uint8_t*)&world.sun.ambient);
}


void world_draw_lensflare(){
    T3DVec3 viewpos, worldpos;

    worldpos = gfx_worldpos_from_polar(world.sun.lensflareangles.v[0], world.sun.lensflareangles.v[1], 1000.0f);
    t3d_viewport_calc_viewspace_pos(&viewport, &viewpos, &worldpos);
    float xpos = viewpos.v[0];
    float ypos = viewpos.v[1];
    
    if(gfx_pos_within_viewport(xpos, ypos))
         world.sun.lensflarealpha = t3d_lerp(world.sun.lensflarealpha, 255.0f, 0.4f);
    else world.sun.lensflarealpha = t3d_lerp(world.sun.lensflarealpha, 0.0f, 0.4f);

    if(world.sun.lensflarealpha > 25.0f){
      //uint16_t* pixels = zbuffer->buffer;
      //uint16_t depth = pixels[(int)ypos * zbuffer->width + (int)xpos];
      //if(depth > 0x8000){
        rdpq_set_mode_standard();
        rdpq_mode_dithering(dither);
        rdpq_mode_filter(FILTER_BILINEAR);
        rdpq_set_prim_color(RGBA32(0xFF,0xFF,0xFF,(uint8_t)world.sun.lensflarealpha));
        rdpq_mode_combiner(RDPQ_COMBINER1((0,0,0,PRIM), (PRIM,0,TEX0,0)));
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);

          rdpq_sprite_blit(sprites[12], xpos - (sprites[12]->width / 2), ypos - (sprites[12]->height / 2), NULL);
          float xpos2 = (display_get_width() - xpos);
          float ypos2 = (display_get_height() - ypos);
          rdpq_sprite_blit(sprites[13], xpos2 - (sprites[13]->width / 2), ypos2 - (sprites[13]->height / 2), NULL);
          xpos = (xpos + xpos2) / 2;
          ypos = (ypos + ypos2) / 2;
          rdpq_sprite_blit(sprites[14], xpos - (sprites[14]->width / 2), ypos - (sprites[14]->height / 2), NULL);
          xpos = (xpos + xpos2) / 2;
          ypos = (ypos + ypos2) / 2;
          rdpq_sprite_blit(sprites[14], xpos - (sprites[14]->width / 2), ypos - (sprites[14]->height / 2), NULL);

        rdpq_mode_antialias(AA_STANDARD);
        rdpq_mode_dithering(dither);
        rdpq_mode_zbuf(false, false);
        rdpq_mode_filter(FILTER_BILINEAR);
        rdpq_mode_persp(true);
        rdpq_mode_mipmap(MIPMAP_NONE,0);
      //}
    }
}