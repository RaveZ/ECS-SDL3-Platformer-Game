#include "MovementSystem.h"
#include <cmath>

void MovementSystem::update(EntityManager& entityManager, float deltaTime)
{
    auto& physicsMap = entityManager.GetComponentMap<PhysicsComponent>();
    auto& transformMap = entityManager.GetComponentMap<TransformComponent>();
    auto& collisionMap = entityManager.GetComponentMap<CollisionComponent>();

    for (auto& [entity, physics] : physicsMap)
    {
        // Find transform
        auto transformIt = transformMap.find(entity);
        if (transformIt == transformMap.end()) continue;
        auto& transform = transformIt->second;

        auto collisionIt = collisionMap.find(entity);
        CollisionComponent* collision = (collisionIt != collisionMap.end()) ? &collisionIt->second : nullptr;

        if (physics.useGravity && (!collision || !collision->grounded)) {
            physics.velocity.y += 500.0f * deltaTime;
        }

        physics.velocity += physics.acceleration * deltaTime;

        if (std::abs(physics.velocity.x) > physics.maxSpeedX) {
            physics.velocity.x = (physics.velocity.x > 0 ? 1.0f : -1.0f) * physics.maxSpeedX;
        }

        if (collision && collision->grounded && physics.acceleration.x == 0.0f) {
            if (physics.velocity.x != 0.0f) {
                float frictionFactor = physics.velocity.x > 0 ? -1.5f : 1.5f;
                float frictionAmount = frictionFactor * 300.0f * deltaTime;
                if (std::abs(physics.velocity.x) < std::abs(frictionAmount)) {
                    physics.velocity.x = 0.0f;
                }
                else {
                    physics.velocity.x += frictionAmount;
                }
            }
        }

        transform.position += physics.velocity * deltaTime;
        physics.acceleration.x = 0.0f;
    }
}