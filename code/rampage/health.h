#ifndef __RAMPAGE_HEALTH_H__
#define __RAMPAGE_HEALTH_H__

#include "./collision/contact.h"

typedef void (*DamageCallback)(void* data, int amount);

enum HealthStatus {
    HEALTH_STATUS_NONE,
    HEALTH_STATUS_ALIVE,
    HEALTH_STATUS_DEAD,
};

struct health {
    void* data;
    DamageCallback callback;
    int is_dead:1;
};

// global setup
void health_init();
void health_destroy();

void health_register(int entity_id, struct health* health, DamageCallback callback, void* data);
void health_unregister(int entity_id);

void health_apply_damage(int entity_id, int amount);
void health_contact_damage(struct contact* contact, int amount);

enum HealthStatus health_status(int entity_id);

#endif