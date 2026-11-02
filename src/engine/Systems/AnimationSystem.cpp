#include "AnimationSystem.h"
#include "../Components/AnimationComponent.h"

extern Coordinator gCoordinator;

void AnimationSystem::Update(float deltaTime)
{
    for (auto const& entity : mEntities)
    {
        auto& anim = gCoordinator.GetComponent<AnimationComponent>(entity);

        if (anim.frameCount <= 1 || anim.isFinished) continue;

        float frameDuration = anim.totalDuration / anim.frameCount;

        anim.timer += deltaTime;

        if (anim.timer >= frameDuration)
        {
            anim.timer -= frameDuration;
            anim.currentFrame++;

            if (anim.currentFrame >= anim.frameCount)
            {
                if (anim.isLooping)
                {
                    anim.currentFrame = 0;
                }
                else
                {
                    anim.currentFrame = anim.frameCount - 1;
                    anim.isFinished = true;
                }
            }
        }
    }
}