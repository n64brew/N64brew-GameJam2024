void sauna_do_light() {
  //uint8_t light_color[4] = {255, 182, 78, 255};
  uint8_t light_color[4] = {255, 255, 255, 255};
  T3DVec3 lights[] = {
    (T3DVec3) {{-40, 200, 330}},
    (T3DVec3) {{415, 200, 330}},
    (T3DVec3) {{125, 100, 0}},
  };
  t3d_light_set_point(0, light_color, &lights[0], 1, false);
  t3d_light_set_point(1, light_color, &lights[1], 1, false);
  t3d_light_set_point(2, light_color, &lights[2], 1, false);
  t3d_light_set_count(3);
}
struct scene sauna_scene = {
  .bg_path = "rom:/avanto/sauna.sprite",
  .z_path = "rom:/avanto/sauna-depth.sprite",
  .fov = T3D_DEG_TO_RAD(78.f),
  .starting_cam = {
    .pos = (T3DVec3) {{0, 100, 0}},
    .target = (T3DVec3) {{125, 125, 155}}
  },
  .do_light = sauna_do_light,
  .ground = {
    .num_changes = 3,
    .changes = {
      {-INFINITY, -30},
      {140, 41},
      {230, 110}
    }
  },
};
