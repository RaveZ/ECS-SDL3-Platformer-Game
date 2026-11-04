#pragma once
#include "../ECS.h"
#include "../GameState.h"
#include <SDL3/SDL.h>

class CollisionSystem : public System {
public:

    void Update(GameState& gs, SDL_Renderer* renderer, float deltaTime);


    void CollisionResponse(
        Entity entityA, Entity entityB,
        const SDL_FRect& rectA, const SDL_FRect& rectB, const SDL_FRect& overlap,
        std::vector<Entity>& entitiesToDestroy);

};