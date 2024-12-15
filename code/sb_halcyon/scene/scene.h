#ifndef SCENE_H
#define SCENE_H

typedef struct
{
    Camera camera;
    LightData light;

} Scene;

void scene_init(Scene *scene)
{
    scene->camera = camera_create();
    scene->light = light_create();
}

#endif