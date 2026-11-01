#pragma once
#include "../EntityManager.h"
#include "../GameState.h"

class CollisionSystem
{
public:
    CollisionSystem(SDL_Renderer* inRenderer) : renderer(inRenderer)
    {
    }
    void update(EntityManager& entityManager, GameState& gs, float deltaTime);

private:
    void CollisionResponse(
        EntityManager& entityManager,
        Entity entityA, Entity entityB,
        const SDL_FRect& rectA, const SDL_FRect& rectB, const SDL_FRect& overlap,
        std::vector<Entity>& entitiesToDestroy);

    bool checkAABB(const SDL_FRect& a, const SDL_FRect& b, SDL_FRect& outOverlap);
    bool hasAABBIntersection(const SDL_FRect& a, const SDL_FRect& b);
private:
    SDL_Renderer* renderer;
};