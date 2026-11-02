#include "resources.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "SDL3_image/SDL_image.h"

SDL_Texture* Resources::loadTexture(SDL_Renderer* renderer, const std::string& filePath)
{
	SDL_Texture* tex = IMG_LoadTexture(renderer, filePath.c_str());
	SDL_SetTextureScaleMode(tex, SDL_ScaleMode::SDL_SCALEMODE_NEAREST);

	return tex;
	
}

MIX_Audio* Resources::loadAudio(MIX_Mixer* mixer, const std::string& filePath)
{
	MIX_Audio* audio = MIX_LoadAudio(mixer, filePath.c_str(), true);
	audioList.push_back(audio);

	return audio;
}

void Resources::load(SDLState& state)
{
	//load animations
	playerAnims.resize(5);
	playerAnims[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);
	playerAnims[ANIM_PLAYER_RUN] = Animation(4, 0.5f);
	playerAnims[ANIM_PLAYER_SLIDE] = Animation(1, 1.0f);
	playerAnims[ANIM_PLAYER_SHOOT] = Animation(4, 0.5f);
	playerAnims[ANIM_PLAYER_SLIDE_SHOOT] = Animation(4, 0.5f);

	bulletAnims.resize(2);
	bulletAnims[ANIM_BULLET_MOVING] = Animation(4, 0.05f);
	bulletAnims[ANIM_BULLET_HIT] = Animation(4, 0.15f);

	enemyAnims.resize(3);
	enemyAnims[ANIM_ENEMY] = Animation(8, 1.0f);
	enemyAnims[ANIM_ENEMY_HIT] = Animation(8, 1.f);
	enemyAnims[ANIM_ENEMY_DIE] = Animation(18, 2.0f);

	//load textures
	texIdle = loadTexture(state.renderer, "data/idle.png");
	texRun = loadTexture(state.renderer, "data/run.png");
	texSlide = loadTexture(state.renderer, "data/slide.png");
	texBrick = loadTexture(state.renderer, "data/tiles/brick.png");
	texGrass = loadTexture(state.renderer, "data/tiles/grass.png");
	texGround = loadTexture(state.renderer, "data/tiles/ground.png");
	texPanel = loadTexture(state.renderer, "data/tiles/panel.png");
	texBg1 = loadTexture(state.renderer, "data/bg/bg.jpg");
	texBg2 = loadTexture(state.renderer, "data/bg/bg_layer2.png");
	texBg3 = loadTexture(state.renderer, "data/bg/bg_layer3.png");
	texBg4 = loadTexture(state.renderer, "data/bg/bg_layer4.png");
	texBullet = loadTexture(state.renderer, "data/bullet.png");
	texBulletHit = loadTexture(state.renderer, "data/bullet_hit.png");
	texShoot = loadTexture(state.renderer, "data/shoot.png");
	texRunShoot = loadTexture(state.renderer, "data/shoot_run.png");
	texSlideShoot = loadTexture(state.renderer, "data/slide_shoot.png");
	texEnemy = loadTexture(state.renderer, "data/enemy.png");
	texEnemyHit = loadTexture(state.renderer, "data/enemy_hit.png");
	texEnemyDie = loadTexture(state.renderer, "data/enemy_die.png");

	//load audio
	audioShoot = loadAudio(state.mixer, "data/audio/shoot.wav");
	audioShootHit = loadAudio(state.mixer, "data/audio/wall_hit.wav");
	audioEnemyHit = loadAudio(state.mixer, "data/audio/enemy_hit.wav");

	musicMain = loadAudio(state.mixer, "data/audio/Juhani Junkala [Retro Game Music Pack] Level 1.mp3");
}

void Resources::unload()
{
	for (SDL_Texture* tex : textures)
	{
		SDL_DestroyTexture(tex);
	}

	for (auto& audio : audioList)
	{
		MIX_DestroyAudio(audio);
	}
}