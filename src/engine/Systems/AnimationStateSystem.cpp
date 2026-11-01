#include "AnimationStateSystem.h"
#include "../Components/TagComponent.h"
#include "../Components/PhysicsComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/RenderComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/CollisionComponent.h"
#include <cmath>

void AnimationStateSystem::update(EntityManager& entityManager, Resources& res, float deltaTime)
{
    auto& animMap = entityManager.GetComponentMap<AnimationComponent>();

    for (auto& [entity, anim] : animMap)
    {
        auto* render = entityManager.GetComponent<RenderComponent>(entity);
        auto* physics = entityManager.GetComponent<PhysicsComponent>(entity);
        auto* tag = entityManager.GetComponent<TagComponent>(entity);
        auto* collision = entityManager.GetComponent<CollisionComponent>(entity);

        if (!render || !tag ) continue;

        auto* health = entityManager.GetComponent<HealthComponent>(entity);
        
		bool isDead = health && health->health <= 0;
        bool isMoving = physics ? std::abs(physics->velocity.x) > 5.0f : false;
        bool isJumping = collision ? !collision->grounded && physics->velocity.y < 0 : false;
        bool isFalling = collision ? !collision->grounded && physics->velocity.y >= 0 : false;

        if (tag->tag == EntityTag::Player)
        {
            if (isMoving || isJumping || isFalling)
            {
                if (render->texture != res.texRun)
                {
                    render->texture = res.texRun;
                    anim.frameCount = 4;
					anim.totalDuration = 0.5f;
                    anim.currentFrame = 0;
                    anim.timer = 0.0f;
                    anim.isLooping = true;
                }
            }
            else
            {
                if (render->texture != res.texIdle)
                {
                    render->texture = res.texIdle;
                    anim.frameCount = 8;
					anim.totalDuration = 1.6f;
                    anim.currentFrame = 0;
                    anim.timer = 0.0f;
                    anim.isLooping = true;
                }
            }
        }
        else if (tag->tag == EntityTag::Enemy)
        {
            if (isDead)
            {
                if (render->texture != res.texEnemyDie)
                {
                    render->texture = res.texEnemyDie;
                    anim.frameCount = 18;
                    anim.totalDuration = 2.0f;
                    anim.currentFrame = 0;
                    anim.timer = 0.0f;
                    anim.isLooping = false;
                }
                continue;
            }
            if (render->shouldFlash)
            {
                continue;
            }
            

            if (isMoving)
            {
                if (render->texture != res.texEnemy)
                {
                    render->texture = res.texEnemy;
                    anim.frameCount = 8;
					anim.totalDuration = 1.0f;
                    anim.currentFrame = 0;
                    anim.timer = 0.0f;
                    anim.isLooping = true;
                }
            }
        }
    }
}