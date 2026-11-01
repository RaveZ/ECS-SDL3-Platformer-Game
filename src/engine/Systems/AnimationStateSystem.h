#pragma once
#include "../EntityManager.h"
#include "../../Resources.h"

class AnimationStateSystem {
public:
    void update(EntityManager& entityManager, Resources& res, float deltaTime);
};