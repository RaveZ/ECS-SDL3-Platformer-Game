#include "MovementSystem.h"
#include "../Components/PhysicsComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/CollisionComponent.h"
#include <cmath>

extern Coordinator gCoordinator;

void MovementSystem::Update(float deltaTime)
{
    for (auto const& entity : mEntities)
    {
        auto& physics = gCoordinator.GetComponent<PhysicsComponent>(entity);
        auto& transform = gCoordinator.GetComponent<TransformComponent>(entity);

        CollisionComponent* collision = nullptr;
        if (gCoordinator.HasComponent<CollisionComponent>(entity))
        {
            collision = &gCoordinator.GetComponent<CollisionComponent>(entity);
        }

        // Apply gravity
        if (physics.useGravity && (!collision || !collision->grounded))
        {
            physics.velocity.y += 500.0f * deltaTime;
        }

        // Apply acceleration to velocity
        physics.velocity += physics.acceleration * deltaTime;

        // Clamp max speed
        if (std::abs(physics.velocity.x) > physics.maxSpeedX)
        {
            physics.velocity.x = (physics.velocity.x > 0 ? 1.0f : -1.0f) * physics.maxSpeedX;
        }

        // Apply friction
        if (collision && collision->grounded && physics.acceleration.x == 0.0f)
        {
            if (physics.velocity.x != 0.0f)
            {
                float frictionFactor = physics.velocity.x > 0 ? -1.5f : 1.5f;
                float frictionAmount = frictionFactor * 300.0f * deltaTime;

                if (std::abs(physics.velocity.x) < std::abs(frictionAmount))
                {
                    physics.velocity.x = 0.0f;
                }
                else
                {
                    physics.velocity.x += frictionAmount;
                }
            }
        }

        // Integrate position
        transform.position += physics.velocity * deltaTime;
        physics.acceleration.x = 0.0f;
    }
}