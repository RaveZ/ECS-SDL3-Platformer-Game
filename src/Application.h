#pragma once

#include "engine/GameState.h"
#include "engine/SDLState.h"
#include "resources.h"
#include "engine/Systems/RenderSystem.h"
#include "engine/Systems/MovementSystem.h"
#include "engine/Systems/CollisionSystem.h"
#include "engine/Systems/InputSystem.h"
#include "engine/Systems/AnimationSystem.h"
#include "engine/Systems/AnimationStateSystem.h"
#include "engine/Systems/LifetimeSystem.h"
#include "engine/ECS.h"
#include "engine/Systems/EnemyAISystem.h"

#ifndef APPLICATION_H
#define APPLICATION_H



class Application
{
public:
    static const int SCREEN_WIDTH;
    static const int SCREEN_HEIGHT;
    static const int HALF_WIDTH;
    static const int HALF_HEIGHT;
	static const int LOGICAL_WIDTH;
	static const int LOGICAL_HEIGHT;

    Application();

    int run();
    bool Initialize();
	void update(float deltaTime);   
    void cleanup();
    void createTiles();

private:
    GameState gs;
    SDLState state;
    Resources res;
    Entity player;


    std::shared_ptr<RenderSystem> renderSystem;
    std::shared_ptr<MovementSystem> movementSystem;
    std::shared_ptr<CollisionSystem> collisionSystem;
    std::shared_ptr<InputSystem> inputSystem;
    std::shared_ptr<AnimationSystem> animationSystem;
    std::shared_ptr<AnimationStateSystem> animationStateSystem;
    std::shared_ptr<LifetimeSystem> lifetimeSystem;

    std::shared_ptr<EnemyAISystem> enemyAISystem;
};


#endif // APPLICATION_H
