#pragma once
#include "../ECS.h"
#include "../../Resources.h"

class AnimationStateSystem : public System {
public:
    void Update(Resources& res, float deltaTime);
};