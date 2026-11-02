#include "application.h"
#include "engine/Components/CollisionComponent.h"
#include "engine/Components/RenderComponent.h"
#include "engine/Components/TransformComponent.h"
#include "engine/Components/PhysicsComponent.h"
#include "engine/Components/AnimationComponent.h"
#include "engine/Components/HealthComponent.h"
#include "engine/Components/LifetimeComponent.h"
#include "engine/Components/TagComponent.h"


const int Application::SCREEN_WIDTH = 1200;
const int Application::SCREEN_HEIGHT = 700;
const int Application::HALF_WIDTH = SCREEN_WIDTH / 2;
const int Application::HALF_HEIGHT = SCREEN_HEIGHT / 2;
const int Application::LOGICAL_WIDTH = 640;
const int Application::LOGICAL_HEIGHT = 320;

const int MAP_ROWS = 5;
const int MAP_COLS = 100;

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

   

    gCoordinator.Init();

    
    //Register Components
    gCoordinator.RegisterComponent<TransformComponent>();
    gCoordinator.RegisterComponent<CollisionComponent>();
    gCoordinator.RegisterComponent<RenderComponent>();
    gCoordinator.RegisterComponent<PhysicsComponent>();    
    gCoordinator.RegisterComponent<AnimationComponent>();
    gCoordinator.RegisterComponent<HealthComponent>();
    gCoordinator.RegisterComponent<LifetimeComponent>();
    gCoordinator.RegisterComponent<TagComponent>();

    //Register Systems
    renderSystem = gCoordinator.RegisterSystem<RenderSystem>();
    movementSystem = gCoordinator.RegisterSystem<MovementSystem>();
    collisionSystem = gCoordinator.RegisterSystem<CollisionSystem>();
    inputSystem = gCoordinator.RegisterSystem<InputSystem>();
    animationSystem = gCoordinator.RegisterSystem<AnimationSystem>();
    animationStateSystem = gCoordinator.RegisterSystem<AnimationStateSystem>();
    lifetimeSystem = gCoordinator.RegisterSystem<LifetimeSystem>();

    Signature renderSig;
    renderSig.set(gCoordinator.GetComponentType<RenderComponent>());
    renderSig.set(gCoordinator.GetComponentType<TransformComponent>());
    gCoordinator.SetSystemSignature<RenderSystem>(renderSig);

    Signature movementSig;
    movementSig.set(gCoordinator.GetComponentType<PhysicsComponent>());
    movementSig.set(gCoordinator.GetComponentType<TransformComponent>());
    gCoordinator.SetSystemSignature<MovementSystem>(movementSig);

    Signature collisionSig;
    collisionSig.set(gCoordinator.GetComponentType<CollisionComponent>());
    collisionSig.set(gCoordinator.GetComponentType<TransformComponent>());
    gCoordinator.SetSystemSignature<CollisionSystem>(collisionSig);

    Signature inputSig;
    inputSig.set(gCoordinator.GetComponentType<TransformComponent>());
    inputSig.set(gCoordinator.GetComponentType<PhysicsComponent>());
    gCoordinator.SetSystemSignature<InputSystem>(inputSig);

    Signature animationSig;
    animationSig.set(gCoordinator.GetComponentType<AnimationComponent>());
    gCoordinator.SetSystemSignature<AnimationSystem>(animationSig);

    Signature animStateSig;
    animStateSig.set(gCoordinator.GetComponentType<AnimationComponent>());
    animStateSig.set(gCoordinator.GetComponentType<RenderComponent>());
    animStateSig.set(gCoordinator.GetComponentType<TagComponent>());
    gCoordinator.SetSystemSignature<AnimationStateSystem>(animStateSig);

    Signature lifetimeSig;
    lifetimeSig.set(gCoordinator.GetComponentType<LifetimeComponent>());
    gCoordinator.SetSystemSignature<LifetimeSystem>(lifetimeSig);
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
    /*drawParalaxBackground(state.renderer, res.texBg4, 0, gs.bg2Scroll, 0.075f, deltaTime);
    drawParalaxBackground(state.renderer, res.texBg3, 0, gs.bg2Scroll, 0.15f, deltaTime);
    drawParalaxBackground(state.renderer, res.texBg2, 0, gs.bg2Scroll, 0.3f, deltaTime);*/


    //update systems
    inputSystem->Update(player, res, state.keys, deltaTime);
    movementSystem->Update(deltaTime);
    collisionSystem->Update(gs, state.renderer,deltaTime);
    animationStateSystem->Update(res, deltaTime);
    animationSystem->Update(deltaTime);
    lifetimeSystem->Update(deltaTime);

    renderSystem->Update(gs, player,state.renderer,deltaTime);

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
        0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 3, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 3, 2, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 4, 0, 2, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    };

    short foreground[MAP_ROWS][MAP_COLS] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    short background[MAP_ROWS][MAP_COLS] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 6, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };


    const auto loadMap = [this](short layer[MAP_ROWS][MAP_COLS])
        {
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
                        Entity e = gCoordinator.CreateEntity();
                        gCoordinator.AddComponent(e, TransformComponent(posX, posY));
                        gCoordinator.AddComponent(e, CollisionComponent(SDL_FRect{ .x = 0, .y = 0, .w = 32, .h = 32 }));
                        gCoordinator.AddComponent(e, RenderComponent(res.texGround, RenderLayer::LEVEL));
                        gCoordinator.AddComponent(e, TagComponent(EntityTag::Level));
                        break;
                    }
                    case 2: //panel
                    {
                        Entity e = gCoordinator.CreateEntity();
                        gCoordinator.AddComponent(e, TransformComponent(posX, posY));
                        gCoordinator.AddComponent(e, CollisionComponent(SDL_FRect{ .x = 0, .y = 0, .w = 32, .h = 32 }));
                        gCoordinator.AddComponent(e, RenderComponent(res.texPanel, RenderLayer::LEVEL));
                        gCoordinator.AddComponent(e, TagComponent(EntityTag::Level));
                        break;
                    }
                    case 3: //enemy
                    {
                        Entity e = gCoordinator.CreateEntity();

                        gCoordinator.AddComponent(e, TransformComponent(posX, posY));
                        gCoordinator.AddComponent(e, CollisionComponent(SDL_FRect{ .x = 10, .y = 4, .w = 12, .h = 28 }));

                        PhysicsComponent enemyPhysics;
                        enemyPhysics.maxSpeedX = 15.0f;
                        enemyPhysics.useGravity = true;
                        gCoordinator.AddComponent(e, enemyPhysics);

                        gCoordinator.AddComponent(e, RenderComponent(res.texEnemy, RenderLayer::CHARACTER));

                        gCoordinator.AddComponent(e, TagComponent(EntityTag::Enemy));

                        AnimationComponent enemyAnim;
                        enemyAnim.texture = res.texEnemy;
                        enemyAnim.frameCount = 8;
                        enemyAnim.totalDuration = 1.0f;
                        enemyAnim.isLooping = true;
                        gCoordinator.AddComponent(e, enemyAnim);

                        gCoordinator.AddComponent(e, HealthComponent(50, 50));
                        break;
                    }
                    case 4: //player
                    {
                        player = gCoordinator.CreateEntity();
                        gCoordinator.AddComponent(player, TransformComponent(posX, posY));
                        gCoordinator.AddComponent(player, RenderComponent(res.texIdle, RenderLayer::CHARACTER));
                        gCoordinator.AddComponent(player, CollisionComponent(SDL_FRect{ .x = 11, .y = 6, .w = 10, .h = 26 }));
                        gCoordinator.AddComponent(player, PhysicsComponent());

                        AnimationComponent playerAnim;
                        playerAnim.texture = res.texIdle;
                        playerAnim.frameCount = 8;
                        playerAnim.totalDuration = 1.6f;
                        playerAnim.isLooping = true;
                        gCoordinator.AddComponent(player, playerAnim);

                        gCoordinator.AddComponent(player, HealthComponent(100, 100));
                        gCoordinator.AddComponent(player, TagComponent(EntityTag::Player));
                        break;
                    }
                    case 5: //grass
                    {
                        Entity e = gCoordinator.CreateEntity();
                        gCoordinator.AddComponent(e, TransformComponent(posX, posY));
                        gCoordinator.AddComponent(e, RenderComponent(res.texGrass, RenderLayer::FOREGROUND));
                        break;
                    }
                    case 6: //brick
                    {
                        Entity e = gCoordinator.CreateEntity();
                        gCoordinator.AddComponent(e, TransformComponent(posX, posY));
                        gCoordinator.AddComponent(e, RenderComponent(res.texBrick, RenderLayer::BACKGROUND));
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