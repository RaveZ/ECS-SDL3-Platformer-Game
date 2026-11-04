#include "EnemyAISystem.h"
#include "../Components/TransformComponent.h"
#include "../Components/PhysicsComponent.h"
#include "../Components/TagComponent.h"
#include "../Components/HealthComponent.h"
#include <cmath>

extern Coordinator gCoordinator;

void EnemyAISystem::Update(Entity player, float deltaTime)
{
    // Ensure the player actually exists before trying to chase them
    if (!gCoordinator.HasComponent<TransformComponent>(player)) return;

    auto& playerTransform = gCoordinator.GetComponent<TransformComponent>(player);

    for (auto const& entity : mEntities)
    {
        auto& transform = gCoordinator.GetComponent<TransformComponent>(entity);
        auto& physics = gCoordinator.GetComponent<PhysicsComponent>(entity);
        auto& tag = gCoordinator.GetComponent<TagComponent>(entity);

        if (tag.tag == EntityTag::Enemy) {
            if (gCoordinator.HasComponent<HealthComponent>(entity)) {
                auto& health = gCoordinator.GetComponent<HealthComponent>(entity);
                if (health.health <= 0) {
                    physics.acceleration.x = 0.0f;
                    continue;
                }
            }

            float dx = playerTransform.position.x - transform.position.x;
            float distance = std::abs(dx);

            // Chase
            if (distance < 150.0f && distance > 20.0f)
            {
                float direction = (dx > 0) ? 1.0f : -1.0f;
                transform.direction = static_cast<int>(direction);

                physics.acceleration.x = direction * 50.0f;
            }
            else
            {
                physics.acceleration.x = 0.0f;
                if (physics.velocity.x != 0.0f) {
                    physics.velocity.x *= 0.8f;
                    if (std::abs(physics.velocity.x) < 0.1f) physics.velocity.x = 0.0f;
                }
            }
        }
    }
}