#pragma once

#include "SDL3/SDL.h"
#include "SDLState.h"
#include "SDL3_mixer/SDL_mixer.h"
#include <array>


const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_CHARACTERS = 1;


struct GameState
{
    int playerIndex;
    SDL_FRect mapViewport;
    float bg2Scroll, bg3Scroll, bg4Scroll;
    bool debugMode;

    GameState()
    {

    }

    GameState(const SDLState& state)
    {
        playerIndex = -1;
        mapViewport = SDL_FRect{
            .x = 0, .y = 0,
            .w = static_cast<float>(state.logW), .h = static_cast<float>(state.logH)
        };
        bg2Scroll = bg3Scroll = bg4Scroll = 0.f;
        debugMode = false;
    }

};
