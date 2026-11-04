#include "catch_amalgamated.hpp"
#include "engine/ECS.h" 
#include "SDL3/SDL.h"
#include <Components/TransformComponent.h>
#include "MathUtils.h"
#include <Systems/CollisionSystem.h>



TEST_CASE("Collision System overlap properly", "[ecs]") {

    SDL_FRect rectA{
        .x = 5.0f,
        .y = 5.0f,
        .w = 10.0f,
        .h = 10.0f
    };

    SDL_FRect rectB{
        .x = 7.5f,
		.y = 6.0f,
        .w = 15.f,
		.h = 15.f
    };

    SDL_FRect overlap = { 0 };

	CheckAABB(rectA, rectB, overlap);

    REQUIRE(overlap.x == 7.5f);
    REQUIRE(overlap.y == 6.0f);
    REQUIRE(overlap.w == 7.5f);
	REQUIRE(overlap.h == 9.0f);
}

TEST_CASE("ComponentManager register the correct type", "[ecs]") {
    Coordinator coord;
    coord.Init();
    coord.RegisterComponent<TransformComponent>();
    ComponentType type = coord.GetComponentType<TransformComponent>();
    REQUIRE(type == 0); 
}

TEST_CASE("ComponentArray stores and retrieves data", "[ecs]") {
    Coordinator coord;
    coord.Init();
    coord.RegisterComponent<TransformComponent>();

    Entity e = coord.CreateEntity();
    coord.AddComponent(e, TransformComponent{ 10.0f, 20.0f });

    auto& transform = coord.GetComponent<TransformComponent>(e);

    REQUIRE(transform.position.x == 10.0f);
    REQUIRE(transform.position.y == 20.0f);
}