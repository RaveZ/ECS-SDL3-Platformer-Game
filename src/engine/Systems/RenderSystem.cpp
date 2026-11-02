#include "RenderSystem.h"
#include "../Components/TransformComponent.h"
#include "../Components/RenderComponent.h"
#include "../Components/AnimationComponent.h"
#include <vector>
#include <map>

extern Coordinator gCoordinator;

void RenderSystem::Update(GameState& gs, Entity player, SDL_Renderer* renderer, float deltaTime)
{
    if (gCoordinator.HasComponent<TransformComponent>(player))
    {
        auto& playerTransform = gCoordinator.GetComponent<TransformComponent>(player);
        gs.mapViewport.x = (playerTransform.position.x + TILE_SIZE / 2) - gs.mapViewport.w / 2;
    }

    struct Drawable {
        RenderComponent* render;
        TransformComponent* transform;
        AnimationComponent* anim;
    };

    std::map<int, std::vector<Drawable>> buckets;

    for (auto const& entity : mEntities)
    {
        auto& renderComp = gCoordinator.GetComponent<RenderComponent>(entity);
        auto& transformComp = gCoordinator.GetComponent<TransformComponent>(entity);

        if (renderComp.texture == nullptr) continue;

        AnimationComponent* anim = nullptr;
        if (gCoordinator.HasComponent<AnimationComponent>(entity))
        {
            anim = &gCoordinator.GetComponent<AnimationComponent>(entity);
        }

        buckets[renderComp.layer].push_back({ &renderComp, &transformComp, anim });
    }

    for (auto& [layer, bucket] : buckets)
    {
        for (auto& d : bucket)
        {
            SDL_FRect finalSrcRect = d.render->srcRect;

            if (d.anim)
            {
                finalSrcRect.x = static_cast<float>(d.anim->currentFrame * d.anim->frameWidth);
                finalSrcRect.y = 0;
                finalSrcRect.w = static_cast<float>(d.anim->frameWidth);
                finalSrcRect.h = static_cast<float>(d.anim->frameHeight);
            }

            d.render->destRect.x = d.transform->position.x - gs.mapViewport.x;
            d.render->destRect.y = d.transform->position.y;

            SDL_FlipMode flipMode = (d.transform->direction == -1) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

            if (d.render->shouldFlash)
            {
                SDL_SetTextureColorModFloat(d.render->texture, 2.5f, 1.0f, 1.0f);

                SDL_RenderTextureRotated(renderer, d.render->texture, &finalSrcRect, &d.render->destRect, 0.f, nullptr, flipMode);

                SDL_SetTextureColorModFloat(d.render->texture, 1.0f, 1.0f, 1.0f);

                d.render->flashTimer -= deltaTime;
                if (d.render->flashTimer <= 0.0f)
                {
                    d.render->shouldFlash = false;
                }
            }
            else
            {
                SDL_RenderTextureRotated(renderer, d.render->texture, &finalSrcRect, &d.render->destRect, 0.f, nullptr, flipMode);
            }
        }
    }
}