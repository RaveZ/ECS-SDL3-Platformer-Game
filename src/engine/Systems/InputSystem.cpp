#include "InputSystem.h"
#include "../Components/PhysicsComponent.h"
#include "../Components/CollisionComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/RenderComponent.h"
#include <iostream> 

void InputSystem::update(EntityManager& entityManager, Entity player, Resources& res, const bool* keys, float deltaTime)
{
    auto* playerPhysics = entityManager.GetComponent<PhysicsComponent>(player);
    auto* playerTransform = entityManager.GetComponent<TransformComponent>(player);
    auto* playerCollision = entityManager.GetComponent<CollisionComponent>(player);

    if (!playerPhysics || !playerTransform) return;

    // Horizontal Movement
    float direction = 0.0f;
    if (keys[SDL_SCANCODE_A]) direction -= 1.0f;
    if (keys[SDL_SCANCODE_D]) direction += 1.0f;

    if (direction != 0.0f) {
        playerTransform->direction = (direction > 0) ? 1 : -1;
        playerPhysics->acceleration.x = direction * 300.0f;
    }
    else {
        playerPhysics->acceleration.x = 0.0f;
    }

    // Jump
    bool isJumpPressed = keys[SDL_SCANCODE_SPACE];
    bool jumpJustPressed = isJumpPressed && !wasJumpPressed;
    bool isGrounded = playerCollision && playerCollision->grounded;

    if (jumpJustPressed && isGrounded)
    {
        playerPhysics->velocity.y = -200.0f;
        playerCollision->grounded = false;
    }

    wasJumpPressed = isJumpPressed;

    // Shooting

    if (shootCooldown > 0.0f) {
        shootCooldown -= deltaTime;
    }

    bool bShootPressed = keys[SDL_SCANCODE_E];

    if (bShootPressed && shootCooldown <= 0.0f)
    {
        shootCooldown = shootFireRate;

        Entity Bullet = entityManager.createEntity();

        TransformComponent bulletTransform(playerTransform->position.x, playerTransform->position.y);
        bulletTransform.direction = playerTransform->direction;

        const float left = -4;
        const float right = 24;
        const float t = (playerTransform->direction + 1) / 2.f;
        const float xOffset = left + right * t;

        bulletTransform.position = glm::vec2(
            playerTransform->position.x + xOffset,
            playerTransform->position.y + TILE_SIZE / 2.0f
        );
        entityManager.AddComponent(Bullet, bulletTransform);

        PhysicsComponent bulletPhysics;
        const int yVariation = 60;
        const float yVelocity = SDL_rand(yVariation) - yVariation / 2.0f;

        bulletPhysics.velocity = glm::vec2(playerPhysics->velocity.x + 600.f * playerTransform->direction, yVelocity);
        bulletPhysics.useGravity = false;
        bulletPhysics.maxSpeedX = 1000.f;
        entityManager.AddComponent(Bullet, bulletPhysics);

        float bulletSize = static_cast<float>(res.texBullet->h);
        CollisionComponent bulletCollision(SDL_FRect{ .x = 0, .y = 0, .w = bulletSize, .h = bulletSize });
        entityManager.AddComponent(Bullet, bulletCollision);

        RenderComponent bulletRender(res.texBullet, RenderLayer::LEVEL);
        bulletRender.srcRect = SDL_FRect{ .x = 0, .y = 0, .w = bulletSize, .h = bulletSize };
        bulletRender.destRect = SDL_FRect{ .x = 0, .y = 0, .w = bulletSize, .h = bulletSize };
        bulletRender.layer = 1;
        entityManager.AddComponent(Bullet, bulletRender);
		entityManager.AddComponent(Bullet, LifetimeComponent(3.0f));
		entityManager.AddComponent(Bullet, TagComponent(EntityTag::Bullet));
    }
}