#pragma once
#include "../ECS.h"
#include "../../Resources.h"

class InputSystem : public System {
public:
    void Update(Entity player, Resources& res, const bool* keys, float deltaTime);

private:
    bool wasJumpPressed = false;
    float shootCooldown = 0.0f;
    const float shootFireRate = 0.15f;
};