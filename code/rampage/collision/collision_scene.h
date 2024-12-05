#ifndef __COLLISION_COLLISION_SCENE_H__
#define __COLLISION_COLLISION_SCENE_H__

#include "dynamic_object.h"
#include "../util/hash_map.h"
#include "contact.h"

typedef int collision_id;

#define MIN_DYNAMIC_OBJECTS 64
#define MAX_ACTIVE_CONTACTS 128

struct collision_scene_element {
    struct dynamic_object* object;
};

struct collision_scene {
    struct collision_scene_element* elements;
    struct contact* next_free_contact;
    struct contact* all_contacts;
    struct hash_map entity_mapping;
    uint16_t count;
    uint16_t capacity;
};

void collision_scene_init();
void collision_scene_add(struct dynamic_object* object);
void collision_scene_remove(struct dynamic_object* object);
void collision_scene_destroy();

struct dynamic_object* collision_scene_find_object(int id);

void collision_scene_collide(float fixed_time_step);

struct contact* collision_scene_new_contact();

typedef void (*collision_scene_query_callback)(void* data, struct dynamic_object* overlaps);

void collision_scene_query(struct dynamic_object_type* shape, struct Vector3* center, int collision_layers, collision_scene_query_callback callback, void* callback_data);

#endif