#ifndef SCREEN_H
#define SCREEN_H

extern T3DViewport viewport[MAXPLAYERS];

void viewport_create(T3DViewport *vp);
void viewport_set(T3DViewport *vp, int playercount, camera_data *cam);
void viewport_drawScissor(void);

// Creates up to 4 viewports depending on playercount
void viewport_create(T3DViewport *vp)
{
  switch (core_get_playercount())
  {
  case 1:
    vp[0] = t3d_viewport_create();
    t3d_viewport_set_area(&vp[0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    break;
  case 2:
    vp[0] = t3d_viewport_create();
    vp[1] = t3d_viewport_create();
    t3d_viewport_set_area(&vp[0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2);
    t3d_viewport_set_area(&vp[1], 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2);
    break;
  case 3:
    vp[0] = t3d_viewport_create();
    vp[1] = t3d_viewport_create();
    vp[2] = t3d_viewport_create();
    t3d_viewport_set_area(&vp[0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2);
    t3d_viewport_set_area(&vp[1], 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 2);
    t3d_viewport_set_area(&vp[2], SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2 - 2, SCREEN_HEIGHT / 2 - 2);
    break;
  case 4:
    vp[0] = t3d_viewport_create();
    vp[1] = t3d_viewport_create();
    vp[2] = t3d_viewport_create();
    vp[3] = t3d_viewport_create();
    t3d_viewport_set_area(&vp[0], 0, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    t3d_viewport_set_area(&vp[1], SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2 - 2, SCREEN_HEIGHT / 2);
    t3d_viewport_set_area(&vp[2], 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 2);
    t3d_viewport_set_area(&vp[3], SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2 - 2, SCREEN_HEIGHT / 2 - 2);
    break;
  }
}

// Sets projection matrix and look at for each viewport
void viewport_set(T3DViewport *vp, int playercount, camera_data *cam)
{
  for (size_t i = 0; i < playercount; i++)
  {
    switch (playercount)
    {
    case 1:
    case 4:
      t3d_viewport_set_projection(&vp[i], T3D_DEG_TO_RAD(30.0f), 5.0f, 300.0f);
      break;
    case 2:
      t3d_viewport_set_projection(&vp[i], T3D_DEG_TO_RAD(20.0f), 5.0f, 300.0f);
      break;
    case 3:
      if (i > 0)
      {
        t3d_viewport_set_projection(&vp[i], T3D_DEG_TO_RAD(30.0f), 5.0f, 300.0f);
      }
      else
      {
        t3d_viewport_set_projection(&vp[i], T3D_DEG_TO_RAD(20.0f), 5.0f, 300.0f);
      }
      break;
    }

    t3d_viewport_look_at(&vp[i], &cam[i].position, &cam[i].target, &(T3DVec3){{0, 1, 0}});
  }
}

// Draws the lines for splitscreen
void viewport_drawScissor(void)
{
  int sizeX = SCREEN_WIDTH;
  int sizeY = SCREEN_HEIGHT;
  rdpq_set_scissor(0, 0, sizeX, sizeY);
  rdpq_set_mode_standard();
  rdpq_set_mode_fill(RGBA32(0, 0, 0, 255));

  // draw thick lines between the screens
  switch (core_get_playercount())
  {
  case 1:
    break;
  case 2:
    rdpq_fill_rectangle(0, sizeY / 2 - 1, sizeX, sizeY / 2 + 1);
    break;
  case 3:
    rdpq_fill_rectangle(0, sizeY / 2 - 1, sizeX, sizeY / 2 + 1);
    rdpq_fill_rectangle(sizeX / 2 - 1, sizeY / 2, sizeX / 2 + 1, sizeY);
    break;
  case 4:
    rdpq_fill_rectangle(0, sizeY / 2 - 1, sizeX, sizeY / 2 + 1);
    rdpq_fill_rectangle(sizeX / 2 - 1, 0, sizeX / 2 + 1, sizeY);
    break;
  }
}

#endif // SCREEN_H