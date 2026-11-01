#include "RenderSystem.h"
#include <vector>
#include <map>

void RenderSystem::update(EntityManager& entityManager, GameState& gs, Entity player, float deltaTime)
{
    auto* playerTransform = entityManager.GetComponent<TransformComponent>(player);

    if (playerTransform)
    {
        gs.mapViewport.x = (playerTransform->position.x + TILE_SIZE / 2) - gs.mapViewport.w / 2;
    }

    auto& renderComponents = entityManager.GetComponentMap<RenderComponent>();
    auto& transformComponents = entityManager.GetComponentMap<TransformComponent>();
    auto& animationComponents = entityManager.GetComponentMap<AnimationComponent>();

    struct Drawable {
        RenderComponent* render;
        TransformComponent* transform;
        AnimationComponent* anim;
    };

    std::map<int, std::vector<Drawable>> buckets;

    for (auto& [entity, renderComp] : renderComponents)
    {
        auto transformIt = transformComponents.find(entity);
        if (transformIt == transformComponents.end()) continue;

        if (renderComp.texture == nullptr) continue;

        auto animIt = animationComponents.find(entity);
        AnimationComponent* anim = (animIt != animationComponents.end()) ? &animIt->second : nullptr;

        buckets[renderComp.layer].push_back({ &renderComp, &transformIt->second, anim });
    }

    for (auto& [layer, bucket] : buckets)
    {
        for (auto& d : bucket)
        {
            SDL_FRect finalSrcRect = d.render->srcRect;

            if (d.anim)
            {
                finalSrcRect.x = d.anim->currentFrame * d.anim->frameWidth;
                finalSrcRect.y = 0;
                finalSrcRect.w = d.anim->frameWidth;
                finalSrcRect.h = d.anim->frameHeight;
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