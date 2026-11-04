#include "CollisionSystem.h"
#include "../Components/TransformComponent.h"
#include "../Components/CollisionComponent.h"
#include "../Components/PhysicsComponent.h"
#include "../Components/TagComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/RenderComponent.h"
#include "../Components/LifetimeComponent.h"
#include "../MathUtils.h"
#include <cmath>
#include <vector>

extern Coordinator gCoordinator;

void CollisionSystem::Update(GameState& gs, SDL_Renderer* renderer, float deltaTime)
{
    // Reset grounded flags for all entities with a CollisionComponent
    for (auto const& entity : mEntities)
    {
        auto& collider = gCoordinator.GetComponent<CollisionComponent>(entity);
        collider.grounded = false;
    }

    std::vector<Entity> entitiesToDestroy;

    // Outer loop (Entity A)
    for (auto const& entityA : mEntities)
    {
        auto& transformA = gCoordinator.GetComponent<TransformComponent>(entityA);
        auto& colliderA = gCoordinator.GetComponent<CollisionComponent>(entityA);

        SDL_FRect rectA{
           .x = transformA.position.x + colliderA.colliderRect.x,
           .y = transformA.position.y + colliderA.colliderRect.y,
           .w = colliderA.colliderRect.w,
           .h = colliderA.colliderRect.h
        };

        SDL_FRect sensorA{
            .x = rectA.x + (rectA.w / 4.0f),
            .y = rectA.y + rectA.h,
            .w = rectA.w / 2.0f,
            .h = 1.0f
        };

        // Inner loop (Entity B)
        for (auto const& entityB : mEntities)
        {
            if (entityA == entityB) continue;

            auto& transformB = gCoordinator.GetComponent<TransformComponent>(entityB);
            auto& colliderB = gCoordinator.GetComponent<CollisionComponent>(entityB);

            SDL_FRect rectB{
                .x = transformB.position.x + colliderB.colliderRect.x,
                .y = transformB.position.y + colliderB.colliderRect.y,
                .w = colliderB.colliderRect.w,
                .h = colliderB.colliderRect.h
            };

            if (HasAABBIntersection(sensorA, rectB))
            {
                colliderA.grounded = true;
            }

            SDL_FRect overlap{ 0 };
            if (CheckAABB(rectA, rectB, overlap))
            {
                CollisionResponse(entityA, entityB, rectA, rectB, overlap, entitiesToDestroy);
            }
        }

        // Debug rendering
        if (gs.debugMode)
        {
            SDL_FRect rectDebug{
               .x = transformA.position.x + colliderA.colliderRect.x - gs.mapViewport.x,
               .y = transformA.position.y + colliderA.colliderRect.y,
               .w = colliderA.colliderRect.w,
               .h = colliderA.colliderRect.h
            };

            SDL_FRect sensorDebug{
               .x = sensorA.x - gs.mapViewport.x,
               .y = sensorA.y,
               .w = sensorA.w,
               .h = sensorA.h
            };

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 150);
            SDL_RenderFillRect(renderer, &rectDebug);
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_RenderFillRect(renderer, &sensorDebug);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }
    }

    // Process destructions AFTER the loops
    for (Entity e : entitiesToDestroy)
    {
        if (gCoordinator.HasComponent<TagComponent>(e))
        {
            auto& tag = gCoordinator.GetComponent<TagComponent>(e);
            if (tag.tag == EntityTag::Enemy)
            {
                gCoordinator.RemoveComponent<CollisionComponent>(e);
                gCoordinator.RemoveComponent<PhysicsComponent>(e);
                gCoordinator.AddComponent(e, LifetimeComponent(3.0f));
                continue;
            }
        }

        gCoordinator.DestroyEntity(e);
    }
}

void CollisionSystem::CollisionResponse(
    Entity entityA, Entity entityB,
    const SDL_FRect& rectA, const SDL_FRect& rectB, const SDL_FRect& overlap,
    std::vector<Entity>& entitiesToDestroy)
{
    auto& transformA = gCoordinator.GetComponent<TransformComponent>(entityA);

    PhysicsComponent* physicsA = nullptr;
    if (gCoordinator.HasComponent<PhysicsComponent>(entityA))
        physicsA = &gCoordinator.GetComponent<PhysicsComponent>(entityA);

    auto& tagA = gCoordinator.GetComponent<TagComponent>(entityA);
    auto& tagB = gCoordinator.GetComponent<TagComponent>(entityB);

    const auto genericResponse = [&]()
        {
            if (overlap.w < overlap.h)
            {
                if (physicsA)
                {
                    if (physicsA->velocity.x > 0 && rectA.x < rectB.x)
                    {
                        transformA.position.x -= overlap.w;
                        physicsA->velocity.x = 0;
                    }
                    else if (physicsA->velocity.x < 0 && rectA.x > rectB.x)
                    {
                        transformA.position.x += overlap.w;
                        physicsA->velocity.x = 0;
                    }
                }
            }
            else
            {
                if (physicsA)
                {
                    if (physicsA->velocity.y > 0 && rectA.y < rectB.y)
                    {
                        transformA.position.y -= overlap.h;
                        physicsA->velocity.y = 0;
                    }
                    else if (physicsA->velocity.y < 0 && rectA.y > rectB.y)
                    {
                        transformA.position.y += overlap.h;
                        physicsA->velocity.y = 0;
                    }
                }
            }
        };

    switch (tagA.tag)
    {
    case EntityTag::Player:
    {
        switch (tagB.tag)
        {
        case EntityTag::Level:
            genericResponse();
            break;
        case EntityTag::Enemy:
        {
            if (gCoordinator.HasComponent<HealthComponent>(entityB))
            {
                auto& enemyHealth = gCoordinator.GetComponent<HealthComponent>(entityB);
                if (enemyHealth.health > 0)
                {
                    if (gCoordinator.HasComponent<RenderComponent>(entityA))
                    {
                        auto& render = gCoordinator.GetComponent<RenderComponent>(entityA);
                        render.shouldFlash = true;
                        render.flashTimer = 0.3f;
                    }
                    if (physicsA)
                    {
                        physicsA->velocity.x = 100.0f * -transformA.direction;
                    }
                }
            }
            break;
        }
        default:
            genericResponse();
            break;
        }
        break;
    }
    case EntityTag::Bullet:
    {
        bool passThrough = false;
        switch (tagB.tag)
        {
        case EntityTag::Enemy:
        {
            if (gCoordinator.HasComponent<HealthComponent>(entityB))
            {
                auto& enemyHealth = gCoordinator.GetComponent<HealthComponent>(entityB);
                if (enemyHealth.health > 0)
                {
                    if (gCoordinator.HasComponent<RenderComponent>(entityB))
                    {
                        auto& render = gCoordinator.GetComponent<RenderComponent>(entityB);
                        render.shouldFlash = true;
                    }
                    enemyHealth.health -= 10;
                    if (enemyHealth.health <= 0)
                    {
                        entitiesToDestroy.push_back(entityB);
                    }
                }
                else
                {
                    passThrough = true;
                }
            }
            break;
        }
        }

        if (!passThrough)
        {
            genericResponse();
            if (physicsA)
            {
                physicsA->velocity *= 0.0f;
            }
            entitiesToDestroy.push_back(entityA);
        }
        break;
    }
    case EntityTag::Enemy:
        genericResponse();
        break;
    default:
        genericResponse();
        break;
    }
}