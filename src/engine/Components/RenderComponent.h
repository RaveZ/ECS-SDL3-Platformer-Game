#pragma once

#include <SDL3/SDL.h>

const int TILE_SIZE = 32;

enum RenderLayer {
	BACKGROUND = 0,
	LEVEL = 1,
	CHARACTER = 2,
	FOREGROUND = 3
};

struct RenderComponent
{
	SDL_FRect srcRect{ 0, 0, TILE_SIZE, TILE_SIZE };
	SDL_FRect destRect{ 0.f, 0.f, TILE_SIZE, TILE_SIZE };
	SDL_Texture* texture = nullptr;

	int layer = 0;

	bool shouldFlash = false;
	float flashTimer = 0.1f;

	RenderComponent() = default;

	RenderComponent(SDL_Texture* tex, int inLayer) : texture(tex), layer(inLayer)
	{
	}
};