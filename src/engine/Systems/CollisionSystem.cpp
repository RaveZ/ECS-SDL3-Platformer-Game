#include "CollisionSystem.h"
#include <cmath>
#include <SDL3/SDL.h>
#include <vector>
#include "../Components/TagComponent.h"
#include "../Components/HealthComponent.h"

void CollisionSystem::update(EntityManager& entityManager, GameState& gs, float deltaTime)
{
    auto& collisionMap = entityManager.GetComponentMap<CollisionComponent>();
    auto& transformMap = entityManager.GetComponentMap<TransformComponent>();

    for (auto& [entity, collider] : collisionMap)
    {
        collider.grounded = false;
    }

    std::vector<Entity> entitiesToDestroy;

    for (auto itA = collisionMap.begin(); itA != collisionMap.end(); ++itA)
    {
        Entity entityA = itA->first;

        auto transformAIt = transformMap.find(entityA);
        if (transformAIt == transformMap.end()) continue;
        auto& transformA = transformAIt->second;
        auto& colliderA = itA->second;

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

        for (auto itB = collisionMap.begin(); itB != collisionMap.end(); ++itB)
        {
            if (itA == itB) continue;

            Entity entityB = itB->first;

            auto transformBIt = transformMap.find(entityB);
            if (transformBIt == transformMap.end()) continue;
            auto& transformB = transformBIt->second;
            auto& colliderB = itB->second;

            SDL_FRect rectB{
                .x = transformB.position.x + colliderB.colliderRect.x,
                .y = transformB.position.y + colliderB.colliderRect.y,
                .w = colliderB.colliderRect.w,
                .h = colliderB.colliderRect.h
            };


            if (hasAABBIntersection(sensorA, rectB))
            {
                colliderA.grounded = true;
            }

            SDL_FRect overlap{ 0 };
            if (checkAABB(rectA, rectB, overlap))
            {
                CollisionResponse(entityManager, entityA, entityB, rectA, rectB, overlap, entitiesToDestroy);
            }
        }

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

    for (Entity e : entitiesToDestroy)
    {
		auto* tag = entityManager.GetComponent<TagComponent>(e);
        if(tag && tag->tag == EntityTag::Enemy)
        {
			entityManager.RemoveComponent<CollisionComponent>(e);
            entityManager.RemoveComponent<PhysicsComponent>(e);
			entityManager.AddComponent(e, LifetimeComponent(3.0f));
		}
        else
        {
            entityManager.destroyEntity(e);
        }
    }
}

bool CollisionSystem::hasAABBIntersection(const SDL_FRect& a, const SDL_FRect& b)
{
    return (a.x < b.x + b.w && a.x + a.w > b.x &&
        a.y < b.y + b.h && a.y + a.h > b.y);
}

bool CollisionSystem::checkAABB(const SDL_FRect& a, const SDL_FRect& b, SDL_FRect& outOverlap)
{
    // Check for overlap on X axis
    float overlapX = std::min(a.x + a.w, b.x + b.w) - std::max(a.x, b.x);

    // Check for overlap on Y axis
    float overlapY = std::min(a.y + a.h, b.y + b.h) - std::max(a.y, b.y);

    if (overlapX <= 0.0f || overlapY <= 0.0f) {
        return false;
    }

    outOverlap.x = std::max(a.x, b.x);
    outOverlap.y = std::max(a.y, b.y);
    outOverlap.w = overlapX;
    outOverlap.h = overlapY;

    return true;
}

void CollisionSystem::CollisionResponse(
    EntityManager& entityManager,
    Entity entityA, Entity entityB,
    const SDL_FRect& rectA, const SDL_FRect& rectB, const SDL_FRect& overlap,
    std::vector<Entity>& entitiesToDestroy)
{
    auto* transformA = entityManager.GetComponent<TransformComponent>(entityA);
    auto* physicsA = entityManager.GetComponent<PhysicsComponent>(entityA);

    auto* tagA = entityManager.GetComponent<TagComponent>(entityA);
    auto* tagB = entityManager.GetComponent<TagComponent>(entityB);

    const auto genericResponse = [&]()
        {
            if (overlap.w < overlap.h)
            { //horizontal collision
                if (physicsA)
                {
					if (physicsA->velocity.x > 0 && rectA.x < rectB.x) //going right
                    {
                        transformA->position.x -= overlap.w;
                        physicsA->velocity.x = 0;
                    }
					else if (physicsA->velocity.x < 0 && rectA.x > rectB.x) // going left
                    {
                        transformA->position.x += overlap.w;
                        physicsA->velocity.x = 0;
                    }
                }
            }
            else
            { //vertical collision
                if (physicsA)
                {
					if (physicsA->velocity.y > 0 && rectA.y < rectB.y)//going down
                    {
                        transformA->position.y -= overlap.h;
                        physicsA->velocity.y = 0;
                    }
					else if (physicsA->velocity.y < 0 && rectA.y > rectB.y)//going up
                    {
                        transformA->position.y += overlap.h;
                        physicsA->velocity.y = 0;
                    }
                }
            }
        };

    switch (tagA->tag)
    {
    case EntityTag::Player:
    {
        switch (tagB->tag)
        {
        case EntityTag::Level:
        {
            genericResponse();
            break;
        }
        case EntityTag::Enemy:
        {
            
            auto* enemyHealth = entityManager.GetComponent<HealthComponent>(entityB);
            if (enemyHealth && enemyHealth->health > 0)
            {
                auto* render = entityManager.GetComponent<RenderComponent>(entityA);
                if (render) {
                    render->shouldFlash = true;
					render->flashTimer = 0.3f;
                }
                if (physicsA)
                {
                    physicsA->velocity.x = 100.0f * -transformA->direction;
                }
            }
            break;
        }
        default:
        {
            genericResponse();
            break;
        }
        }
        break;
    }
    case EntityTag::Bullet:
    {
        bool passThrough = false;
        switch (tagB->tag)
        {
        case EntityTag::Enemy:
        {
            auto* enemyHealth = entityManager.GetComponent<HealthComponent>(entityB);
            if (enemyHealth && enemyHealth->health > 0)
            {
                auto* render = entityManager.GetComponent<RenderComponent>(entityB);
                if (render) {
                    render->shouldFlash = true;
                }
                enemyHealth->health -= 10;
                if (enemyHealth->health <= 0)
                {
                    entitiesToDestroy.push_back(entityB);
                }
            }
            else
            {
                passThrough = true;
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
    {
        genericResponse();
        break;
    }
    default:
    {
        genericResponse();
        break;
    }
    }
}