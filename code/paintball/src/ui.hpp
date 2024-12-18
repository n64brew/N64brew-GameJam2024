#ifndef __UI_HPP
#define __UI_HPP

#include <libdragon.h>

#include "./wrappers.hpp"
#include "./constants.hpp"
#include "./gamestate.hpp"
#include "./list.hpp"

#include "../../../minigame.h"

struct HitMark {
    T3DVec3 pos;
    PlyNum team;
    float lifetime;
};

enum SelectedMenuItem {
    MENU_PLAY,
    MENU_EXIT
};

class UIRenderer
{
    private:
        RDPQFont mediumFont;
        RDPQFont bigFont;

        U::Sprite hitSprite;
        U::Sprite splash1;
        U::Sprite splash2;

        List<HitMark, PlayerCount * 4> hits;

        Wav64 sfxCountdown;
        int prevCountdown;

        void renderHitMarks(T3DViewport &viewport, float deltaTime);

        State renderMenu(const State &state);
        SelectedMenuItem selectedMenuItem;
        State pausedState;

    public:
        UIRenderer();
        void render(GameState &state, T3DViewport &viewport, float deltaTime);

        void registerHit(const HitMark &hit);
};

#endif // __UI_HPP