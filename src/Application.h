#pragma once

#include "engine/GameState.h"
#include "engine/SDLState.h"
#include "resources.h"
#include "engine/EntityManager.h"
#include "engine/Systems/RenderSystem.h"
#include "engine/Systems/MovementSystem.h"
#include "engine/Systems/CollisionSystem.h"
#include "engine/Systems/InputSystem.h"
#include "engine/Systems/AnimationSystem.h"
#include "engine/Systems/AnimationStateSystem.h"
#include "engine/Systems/LifetimeSystem.h"

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
	
    std::unique_ptr<EntityManager> entityManager;
    std::unique_ptr<RenderSystem> renderSystem;
	std::unique_ptr<MovementSystem> movementSystem;
	std::unique_ptr<CollisionSystem> collisionSystem;
	std::unique_ptr<InputSystem> inputSystem;
	std::unique_ptr<AnimationSystem> animationSystem;
	std::unique_ptr<AnimationStateSystem> animationStateSystem;
	std::unique_ptr<LifetimeSystem> lifetimeSystem;
};


#endif // APPLICATION_H
