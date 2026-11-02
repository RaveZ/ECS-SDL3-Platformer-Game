#pragma once
#include "../ECS.h"

class MovementSystem : public System {
public:
    void Update(float deltaTime);
};