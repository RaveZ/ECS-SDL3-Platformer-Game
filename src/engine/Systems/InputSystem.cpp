#include "InputSystem.h"
#include "../Components/PhysicsComponent.h"
#include "../Components/CollisionComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/RenderComponent.h"
#include "../Components/LifetimeComponent.h"
#include "../Components/TagComponent.h"
#include <SDL3/SDL.h>

extern Coordinator gCoordinator;

void InputSystem::Update(Entity player, Resources& res, const bool* keys, float deltaTime)
{
    if (!gCoordinator.HasComponent<PhysicsComponent>(player) ||
        !gCoordinator.HasComponent<TransformComponent>(player))
    {
        return;
    }

    auto& playerPhysics = gCoordinator.GetComponent<PhysicsComponent>(player);
    auto& playerTransform = gCoordinator.GetComponent<TransformComponent>(player);

    CollisionComponent* playerCollision = nullptr;
    if (gCoordinator.HasComponent<CollisionComponent>(player))
        playerCollision = &gCoordinator.GetComponent<CollisionComponent>(player);

    //Horizontal Movement
    float direction = 0.0f;
    if (keys[SDL_SCANCODE_A]) direction -= 1.0f;
    if (keys[SDL_SCANCODE_D]) direction += 1.0f;

    if (direction != 0.0f) {
        playerTransform.direction = (direction > 0) ? 1 : -1;
        playerPhysics.acceleration.x = direction * 300.0f;
    }
    else {
        playerPhysics.acceleration.x = 0.0f;
    }

    //Jumping
    bool isJumpPressed = keys[SDL_SCANCODE_SPACE];
    bool jumpJustPressed = isJumpPressed && !wasJumpPressed;
    bool isGrounded = playerCollision && playerCollision->grounded;

    if (jumpJustPressed && isGrounded)
    {
        playerPhysics.velocity.y = -200.0f;
        playerCollision->grounded = false;
    }

    wasJumpPressed = isJumpPressed;

    //Shooting
    if (shootCooldown > 0.0f) {
        shootCooldown -= deltaTime;
    }

    bool bShootPressed = keys[SDL_SCANCODE_E];

    if (bShootPressed && shootCooldown <= 0.0f)
    {
        shootCooldown = shootFireRate;

        Entity bullet = gCoordinator.CreateEntity();

        TransformComponent bulletTransform(playerTransform.position.x, playerTransform.position.y);
        bulletTransform.direction = playerTransform.direction;

        const float left = -4;
        const float right = 24;
        const float t = (playerTransform.direction + 1) / 2.f;
        const float xOffset = left + right * t;

        bulletTransform.position = glm::vec2(
            playerTransform.position.x + xOffset,
            playerTransform.position.y + TILE_SIZE / 2.0f
        );
        gCoordinator.AddComponent(bullet, bulletTransform);

        PhysicsComponent bulletPhysics;
        const int yVariation = 60;
        const float yVelocity = SDL_rand(yVariation) - yVariation / 2.0f;

        bulletPhysics.velocity = glm::vec2(playerPhysics.velocity.x + 600.f * playerTransform.direction, yVelocity);
        bulletPhysics.useGravity = false;
        bulletPhysics.maxSpeedX = 1000.f;
        gCoordinator.AddComponent(bullet, bulletPhysics);

        float bulletSize = static_cast<float>(res.texBullet->h);
        CollisionComponent bulletCollision(SDL_FRect{ .x = 0, .y = 0, .w = bulletSize, .h = bulletSize });
        gCoordinator.AddComponent(bullet, bulletCollision);

        RenderComponent bulletRender(res.texBullet, RenderLayer::LEVEL);
        bulletRender.srcRect = SDL_FRect{ .x = 0, .y = 0, .w = bulletSize, .h = bulletSize };
        bulletRender.destRect = SDL_FRect{ .x = 0, .y = 0, .w = bulletSize, .h = bulletSize };
        bulletRender.layer = 1;
        gCoordinator.AddComponent(bullet, bulletRender);

        gCoordinator.AddComponent(bullet, LifetimeComponent(3.0f));
        gCoordinator.AddComponent(bullet, TagComponent(EntityTag::Bullet));
    }
}