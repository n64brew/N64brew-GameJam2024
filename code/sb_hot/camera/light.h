#ifndef LIGHT_H
#define LIGHT_H

typedef struct
{

    uint8_t ambient_color[4];
    uint8_t directional_color1[4];
    T3DVec3 direction1;

    uint8_t directional_color2[4];
    T3DVec3 direction2;

} LightData;

void light_set(LightData *light);
void light_setAmbient(LightData *light, uint8_t value);
void light_resetAmbient(LightData *light);

LightData light_create()
{
    LightData light = {
        .ambient_color = {0x32, 0x20, 0x06, 0xFF},
        .directional_color1 = {0xE3, 0x3B, 0x29, 0xFF},
        .direction1 = {{1.0f, 1.0f, 1.0f}},
        .directional_color2 = {0x32, 0x20, 0x06, 0xFF},
        .direction2 = {{0.0f, 100.0f, 1000.0f}},
    };

    t3d_vec3_norm(&light.direction1);

    return light;
}

/* set light
temporary function until i learn how the lights work  */
void light_set(LightData *light)
{
    t3d_light_set_ambient(light->ambient_color);
    t3d_light_set_directional(0, light->directional_color1, &light->direction1);
    t3d_light_set_directional(1, light->directional_color2, &light->direction2);
    t3d_light_set_count(2);
}

void light_setAmbient(LightData *light, uint8_t value)
{
    for (size_t i = 0; i < 3; i++)
    {
        light->ambient_color[i] = value;
    }
    light_set(light);
}

void light_resetAmbient(LightData *light)
{
    light->ambient_color[0] = 0x32;
    light->ambient_color[1] = 0x20;
    light->ambient_color[2] = 0x06;
    light_set(light);
}

#endif