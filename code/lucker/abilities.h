#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "lucker.h"

void sword(player* p);

void heart(player* p);

void bomb(player* src, player* dest);

void swap_hp(player* src, player* dest);

void evade_remove(player* p);

void burst(player* p);

void crit(player* p);

void vampirism(player* p);

void lightning(player* p);

void evade(player* p);

static inline void activate_ability(wheel selection, player* targ, player* other)
{
    switch (selection)
    {
        case SWORD:
            sword(targ);
            break;
        case HEART:
            heart(targ);
            break;
        case BOMB:
            bomb(targ, other);
            break;
        case SWAP_HP:
            swap_hp(targ, other);
            break;
        case EVADE_REMOVE:
            evade_remove(targ);
            break;
        case BURST:
            burst(targ);
            break;
        case CRIT:
            crit(targ);
            break;
        case VAMPIRISM:
            vampirism(targ);
            break;
        case LIGHTNING:
            lightning(targ);
            break;
        case EVADE:
            evade(targ);
            break;
        default:
            return;
    }
}