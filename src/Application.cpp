#include "application.h"
#include "engine/Components/CollisionComponent.h"
#include "engine/Components/RenderComponent.h"
#include "engine/Components/InputComponent.h"
#include "engine/Components/TransformComponent.h"


const int Application::SCREEN_WIDTH = 1200;
const int Application::SCREEN_HEIGHT = 700;
const int Application::HALF_WIDTH = SCREEN_WIDTH / 2;
const int Application::HALF_HEIGHT = SCREEN_HEIGHT / 2;
const int Application::LOGICAL_WIDTH = 640;
const int Application::LOGICAL_HEIGHT = 320;

const int MAP_ROWS = 5;
const int MAP_COLS = 50;

void drawParalaxBackground(SDL_Renderer* renderer, SDL_Texture* texture, float xVelocity, float& scrollPos, float scrollFactor, float deltaTime);

Application::Application()
{
}

bool Application::Initialize()
{
    state = SDLState();
    state.width = SCREEN_WIDTH;
    state.height = SCREEN_HEIGHT;
    state.logW = LOGICAL_WIDTH;
    state.logH = LOGICAL_HEIGHT;

   
    bool initSuccess = true;
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
        initSuccess = false;
    }

    //create window
    state.window = SDL_CreateWindow("SDL3 Demo", state.width, state.height, SDL_WINDOW_RESIZABLE);

    if (!state.window)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
        cleanup();
        initSuccess = false;
    }

    //create the renderer
    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if (!state.renderer)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", state.window);
        cleanup();
        initSuccess = false;
    }
    SDL_SetRenderVSync(state.renderer, true);


    //configure presentation
    SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    //create mixer device
    MIX_Init();
    state.mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!state.mixer)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating mixer device", state.window);
        cleanup();
        initSuccess = false;
    }

    //setup entity manager
    entityManager = std::make_unique<EntityManager>();

    //setup systems
	renderSystem = std::make_unique<RenderSystem>(state.renderer);
    movementSystem = std::make_unique<MovementSystem>();
	collisionSystem = std::make_unique<CollisionSystem>(state.renderer);
	inputSystem = std::make_unique<InputSystem>();
    animationSystem = std::make_unique<AnimationSystem>();
	animationStateSystem = std::make_unique<AnimationStateSystem>();    
	lifetimeSystem = std::make_unique<LifetimeSystem>();
    return initSuccess;
}


int Application::run()
{
    if (!Initialize())
    {
        return 1;
    }

    //load the assets
    res.load(state);


    //setup game data
    gs = GameState(state);
    createTiles();
    uint64_t prevTime = SDL_GetTicks();


    //play main music
    MIX_Track* track = MIX_CreateTrack(state.mixer);
    MIX_SetTrackAudio(track, res.musicMain);
    MIX_SetTrackGain(track, 0.33f);
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    MIX_PlayTrack(track, props);

    //start the game loop
    bool running = true;
    while (running)
    {
        uint64_t nowTime = SDL_GetTicks();
        float deltaTime = (nowTime - prevTime) / 1000.f; // convert to seconds 
        SDL_Event event{ 0 };
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                {
                    running = false;
                    break;
                }
                case SDL_EVENT_WINDOW_RESIZED:
                {
                    state.width = event.window.data1;
                    state.height = event.window.data2;
                    break;
                }
                case SDL_EVENT_KEY_UP:
                {
                    if (event.key.scancode == SDL_SCANCODE_F10)
                    {
                        gs.debugMode = !gs.debugMode;
                    }

                    if (event.key.scancode == SDL_SCANCODE_F11)
                    {
                        state.fullscreen = !state.fullscreen;
                        SDL_SetWindowFullscreen(state.window, state.fullscreen);
                    }
                    break;
                }
            }
            
        }

        update(deltaTime);
       
        prevTime = nowTime;
    }

    res.unload();
    cleanup();
    return 0;

}

void Application::update(float deltaTime)
{ 
    //perform drawing commands
    SDL_SetRenderDrawColor(state.renderer, 50, 50, 50, 1);
    SDL_RenderClear(state.renderer);

    //draw background images
    SDL_RenderTexture(state.renderer, res.texBg1, nullptr, nullptr);
    drawParalaxBackground(state.renderer, res.texBg4, 0, gs.bg2Scroll, 0.075f, deltaTime);
    drawParalaxBackground(state.renderer, res.texBg3, 0, gs.bg2Scroll, 0.15f, deltaTime);
    drawParalaxBackground(state.renderer, res.texBg2, 0, gs.bg2Scroll, 0.3f, deltaTime);


    //update systems
    renderSystem->update(*entityManager, gs, player, deltaTime);
    movementSystem->update(*entityManager, deltaTime);
	collisionSystem->update(*entityManager, gs, deltaTime);
	inputSystem->update(*entityManager, player, res, state.keys, deltaTime);
	animationSystem->update(*entityManager, deltaTime);
	animationStateSystem->update(*entityManager, res, deltaTime);
	lifetimeSystem->update(*entityManager, deltaTime);

    //swap buffer and present
    SDL_RenderPresent(state.renderer);
}




void Application::cleanup()
{
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}


void Application::createTiles()
{
    /*
    *   1 - Ground
    *   2 - Panel
    *   3 - Enemy
    *   4 - Player
    *   5 - Grass
    *   6 - Brick
    */
    short map[MAP_ROWS][MAP_COLS] = {
        0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 3, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 0, 2, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    };

    short foreground[MAP_ROWS][MAP_COLS] = {
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    };

    short background[MAP_ROWS][MAP_COLS] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 6, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    };


    const auto loadMap = [this](short layer[MAP_ROWS][MAP_COLS])
        {
            const auto createObject = [this](int r, int c, SDL_Texture* tex, ObjectType type)
                {
                    GameObject o;
                    o.type = type;
                    o.position = glm::vec2(c * TILE_SIZE, state.logH - (MAP_ROWS - r) * TILE_SIZE);
                    o.texture = tex;
                    o.collider = {
                        .x = 0, .y = 0,
                        .w = TILE_SIZE, .h = TILE_SIZE
                    };
                    return o;
                };

            for (int r = 0; r < MAP_ROWS; r++)
            {
                for (int c = 0; c < MAP_COLS; c++)
                {
					float posX = static_cast<float>(c * TILE_SIZE);
					float posY = static_cast<float>(state.logH - (MAP_ROWS - r) * TILE_SIZE);
                    switch (layer[r][c])
                    {
                    case 1: //ground
                    {
						Entity e = entityManager->createEntity();
						entityManager->AddComponent(e, TransformComponent(posX, posY));

                        entityManager->AddComponent(e, CollisionComponent(SDL_FRect{ .x = 0, .y = 0, .w = 32, .h = 32 }));
						entityManager->AddComponent(e, RenderComponent(res.texGround, RenderLayer::LEVEL));
                        entityManager->AddComponent(e, TagComponent(EntityTag::Level));
                        break;
                    }
                    case 2: //panel
                    {
                        Entity e = entityManager->createEntity();
                        entityManager->AddComponent(e, TransformComponent(posX, posY));
                        entityManager->AddComponent(e, CollisionComponent(SDL_FRect{ .x = 0, .y = 0, .w = 32, .h = 32 }));
                        entityManager->AddComponent(e, RenderComponent(res.texPanel, RenderLayer::LEVEL));

                        entityManager->AddComponent(e, TagComponent(EntityTag::Level));
                        break;
                    }
                    case 3: //enemy
                    {
                        GameObject o = createObject(r, c, res.texEnemy, ObjectType::enemy);
                        o.data.enemy = EnemyData();
                        o.currentAnimation = res.ANIM_ENEMY;
                        o.animations = res.enemyAnims;
                        o.collider = {
                            .x = 10, .y = 4,
                            .w = 12, .h = 28
                        };
                        o.maxSpeedX = 15;
                        o.dynamic = true;
                        gs.layers[LAYER_IDX_CHARACTERS].push_back(o);

                        Entity e = entityManager->createEntity();
                        entityManager->AddComponent(e, TransformComponent(posX, posY));
                        entityManager->AddComponent(e, CollisionComponent(SDL_FRect{ .x = 10, .y = 4, .w = 12, .h = 28 }));
						entityManager->AddComponent(e, RenderComponent(res.texEnemy, RenderLayer::CHARACTER));
						entityManager->AddComponent(e, PhysicsComponent());
                        entityManager->AddComponent(e, TagComponent(EntityTag::Enemy));

                        AnimationComponent enemyAnim;
                        enemyAnim.texture = res.texEnemy;
                        enemyAnim.frameCount = 8;
                        enemyAnim.totalDuration = 1.0f;
                        enemyAnim.isLooping = true;

                        entityManager->AddComponent(e, enemyAnim);

                        entityManager->AddComponent(e, HealthComponent(50, 50));
                        break;
                    }
                    case 4: //player
                    {
                        //create our player
						player = entityManager->createEntity();
						entityManager->AddComponent(player, InputComponent());
						entityManager->AddComponent(player, TransformComponent(posX, posY));
						entityManager->AddComponent(player, RenderComponent(res.texIdle, RenderLayer::CHARACTER));
						entityManager->AddComponent(player, CollisionComponent(SDL_FRect { .x = 11, .y = 6, .w = 10, .h = 26 }));
						entityManager->AddComponent(player, PhysicsComponent());

                        AnimationComponent playerAnim;
                        playerAnim.texture = res.texIdle;
                        playerAnim.frameCount = 8;
                        playerAnim.totalDuration = 1.6f;
                        playerAnim.isLooping = true;

                        entityManager->AddComponent(player, playerAnim);
						entityManager->AddComponent(player, HealthComponent(100, 100));
						entityManager->AddComponent(player, TagComponent(EntityTag::Player));

                        break;
                    }
                    case 5: //grass
                    {
                        Entity e = entityManager->createEntity();
                        entityManager->AddComponent(e, TransformComponent(posX, posY));
                        entityManager->AddComponent(e, RenderComponent(res.texGrass, RenderLayer::FOREGROUND));
                        break;
                    }
                    case 6: //brick
                    {
                        Entity e = entityManager->createEntity();
                        entityManager->AddComponent(e, TransformComponent(posX, posY));
                        entityManager->AddComponent(e, RenderComponent(res.texBrick, RenderLayer::BACKGROUND));
                        break;
                    }
                    }
                }
            }
        };

    loadMap(map);
    loadMap(foreground);
    loadMap(background);

}


void drawParalaxBackground(SDL_Renderer* renderer, SDL_Texture* texture, float xVelocity, float& scrollPos, float scrollFactor, float deltaTime)
{
    scrollPos -= xVelocity * scrollFactor * deltaTime;

    if (scrollPos <= -texture->w)
    {
        scrollPos = 0;
    }

    SDL_FRect dst{
        .x = scrollPos, .y = 30,
        .w = texture->w * 2.0f, .h = static_cast<float>(texture->h)
    };

    SDL_RenderTextureTiled(renderer, texture, nullptr, 1, &dst);
}