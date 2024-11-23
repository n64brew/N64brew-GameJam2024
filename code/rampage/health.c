#include "./health.h"

#include "./util/hash_map.h"
#include <stdbool.h>
#include <stdio.h>

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
    health->is_dead = 0;

    hash_map_set(&g_health_callbacks, entity_id, health);
}

void health_unregister(int entity_id) {
    hash_map_delete(&g_health_callbacks, entity_id);
}

void health_apply_damage(int entity_id, int amount, struct Vector3* velocity, int source_id) {
    struct health* target = hash_map_get(&g_health_callbacks, entity_id);

    if (target) {
        target->callback(target->data, amount, velocity, source_id);
    }
}

bool health_contact_check_prev_contacts(int target_id, uint8_t* already_hit, int max_hit_count) {
    if (!already_hit) {
        return true;
    }

    for (int i = 0; i < max_hit_count; i += 1) {
        if (already_hit[i] == target_id) {
            return false;
        }

        if (already_hit[i] == 0) {
            already_hit[i] = target_id;
            return true;
        }
    }

    return false;
}

void health_contact_damage(struct contact* contact, int amount, struct Vector3* velocity, int source_id, uint8_t* already_hit, int max_hit_count) {
    while (contact) {
        if (health_contact_check_prev_contacts(contact->other_object, already_hit, max_hit_count)) {
            health_apply_damage(contact->other_object, amount, velocity, source_id);
        }

        contact = contact->next;
    }
}

enum HealthStatus health_status(int entity_id) {
    struct health* target = hash_map_get(&g_health_callbacks, entity_id);

    if (!target) {
        return HEALTH_STATUS_NONE;
    }

    if (target->is_dead) {
        return HEALTH_STATUS_DEAD;
    }

    return HEALTH_STATUS_ALIVE;
}