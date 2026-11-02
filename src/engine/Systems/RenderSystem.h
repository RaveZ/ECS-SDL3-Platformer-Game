#pragma once

#include <SDL3/SDL.h>
#include "../ECS.h"
#include "../GameState.h"

class RenderSystem : public System {
public:

    void Update(GameState& gs, Entity player, SDL_Renderer* renderer, float deltaTime);

};