#include "entity_id.h"

static int g_next_entity_id = 1;

int entity_id_next() {
    return g_next_entity_id++;
}