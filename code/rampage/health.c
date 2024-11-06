#include "./health.h"

#include "./util/hash_map.h"

static struct hash_map g_health_callbacks;

void health_init() {
    hash_map_init(&g_health_callbacks, 32);
}

void health_destroy() {
    hash_map_destroy(&g_health_callbacks);
}

void health_register(int entity_id, struct health* health, DamageCallback callback, void* data) {
    health->callback = callback;
    health->data = data;

    hash_map_set(&g_health_callbacks, entity_id, health);
}

void health_unregister(int entity_id) {
    hash_map_delete(&g_health_callbacks, entity_id);
}

void health_apply_damage(int entity_id) {
    struct health* target = hash_map_get(&g_health_callbacks, entity_id);

    if (target) {
        target->callback(target->data);
    }
}

void health_contact_damage(struct contact* contact) {
    while (contact) {
        health_apply_damage(contact->other_object);
        contact = contact->next;
    }
}