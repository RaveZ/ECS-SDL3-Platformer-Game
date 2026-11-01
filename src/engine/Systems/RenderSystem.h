#pragma once

#include <SDL3/SDL.h>
#include "../EntityManager.h"
#include "../GameState.h"

class RenderSystem
{
public:
	RenderSystem(SDL_Renderer* inRenderer) : renderer(inRenderer)
	{

	}
	void update(EntityManager& entityManager, GameState& gs, Entity player, float deltaTime);

private:
	SDL_Renderer* renderer;

};