#pragma once
#include "../ECS.h"

class EnemyAISystem : public System {
public:
    void Update(Entity player, float deltaTime);
};