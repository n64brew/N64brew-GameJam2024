#ifndef __UI_HPP
#define __UI_HPP

#include <libdragon.h>
#include "./wrappers.hpp"
#include "./constants.hpp"
#include "./gamestate.hpp"

class UIRenderer
{
    private:
        RDPQFont mediumFont;
        RDPQFont bigFont;

    public:
        UIRenderer();
        void render(const GameState &state);
};

#endif // __UI_HPP