#include "LifetimeSystem.h"
#include "../Components/LifetimeComponent.h"
#include <vector>

extern Coordinator gCoordinator;

void LifetimeSystem::Update(float deltaTime)
{
    std::vector<Entity> entitiesToDestroy;

    for (auto const& entity : mEntities)
    {
        auto& lifetime = gCoordinator.GetComponent<LifetimeComponent>(entity);

        lifetime.TTL -= deltaTime;

        if (lifetime.TTL <= 0.f)
        {
            entitiesToDestroy.push_back(entity);
        }
    }

    for (Entity entity : entitiesToDestroy)
    {
        gCoordinator.DestroyEntity(entity);
    }
}