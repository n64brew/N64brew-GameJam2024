#include <libdragon.h>
#include "../../core.h"
#include "../../minigame.h"
#include "lucker.h"
#include "abilities.h"
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dskeleton.h>
#include <t3d/t3danim.h>
#include <t3d/t3ddebug.h>

//this will be called first so that
void ability_init()
{
    //!!!! bomb model?
}

void sword(player* p) 
{
    p->fighter.abilityActive[SWORD] = true;
    p->fighter.abilityTimers[SWORD] = 0;
}

void heart(player* p)
{
    p->fighter.hp += 50;
}

void bomb(player* src, player* dest)
{
    //!!!! do this later, need to spawn bomb on src
    dest->fighter.hp = 0;
    dest->fighter.color = RGBA32(0,0,0,1);
    dest->fighter.boom = true;
}

void swap_hp(player* src, player* dest)
{
    float tempHP = src->fighter.hp;
    src->fighter.hp = dest->fighter.hp;
    dest->fighter.hp = tempHP;
}

void evade_remove(player* p)
{
    p->fighter.abilityActive[EVADE_REMOVE] = true;
    p->fighter.abilityTimers[EVADE_REMOVE] = 0;
}

void burst(player* p)
{
    p->fighter.abilityActive[BURST] = true;
    p->fighter.abilityTimers[BURST] = 0;
}

void crit(player* p)
{
    p->fighter.abilityActive[CRIT] = true;   
    p->fighter.abilityTimers[CRIT] = 0;
}

void vampirism(player* p)
{
    p->fighter.abilityActive[VAMPIRISM] = true;  
    p->fighter.abilityTimers[VAMPIRISM] = 0;
}

void lightning(player* p)
{
    p->fighter.abilityActive[LIGHTNING] = true;  
    p->fighter.abilityTimers[LIGHTNING] = 0;
}

void evade(player* p)
{
    p->fighter.abilityActive[EVADE] = true;  
    p->fighter.abilityTimers[EVADE] = 0;
}