cmake_minimum_required(VERSION 3.8)
project("SDL_Game" C CXX)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_mixer REQUIRED)

# --- Main Game ---
add_executable(SDL_Game 
    "src/engine/Components/TransformComponent.h" 
    "src/engine/Components/RenderComponent.h"
    "src/engine/Components/CollisionComponent.h"
    "src/Main.cpp" 
    "src/Application.h" 
    "src/Application.cpp"
    "src/engine/GameState.h" "src/Resources.h" "src/Resources.cpp" "src/engine/SDLState.h" "src/engine/Systems/RenderSystem.cpp" "src/engine/Systems/RenderSystem.h" "src/engine/Systems/MovementSystem.h" "src/engine/Systems/MovementSystem.cpp" "src/engine/Components/PhysicsComponent.h" "src/engine/Systems/CollisionSystem.h" "src/engine/Systems/CollisionSystem.cpp" "src/engine/Systems/InputSystem.h" "src/engine/Systems/InputSystem.cpp" "src/engine/Components/AnimationComponent.h" "src/engine/Systems/AnimationSystem.h" "src/engine/Systems/AnimationSystem.cpp" "src/engine/Components/HealthComponent.h" "src/engine/Systems/LifetimeSystem.h" "src/engine/Systems/LifetimeSystem.cpp" "src/engine/Components/LifetimeComponent.h" "src/engine/Components/TagComponent.h" "src/engine/Systems/AnimationStateSystem.h" "src/engine/Systems/AnimationStateSystem.cpp" "src/engine/ECS.h"
 "src/engine/MathUtils.h" "src/engine/Systems/EnemyAISystem.h" "src/engine/Systems/EnemyAISystem.cpp")

if(CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET SDL_Game PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(SDL_Game PRIVATE SDL3::SDL3 SDL3_image::SDL3_image SDL3_mixer::SDL3_mixer)
target_include_directories(SDL_Game PRIVATE "ext/")
target_include_directories(SDL_Game PRIVATE "engine/")
target_include_directories(SDL_Game PRIVATE "ext/glm/")

# --- Catch2 Setup ---
add_library(Catch2 STATIC ext/catch2/catch_amalgamated.cpp "src/engine/MathUtils.h" "src/engine/Systems/EnemyAISystem.h" "src/engine/Systems/EnemyAISystem.cpp")
target_include_directories(Catch2 PUBLIC ext/catch2)

# --- Test Executable ---
add_executable(ECS_Tests
    tests/ECS_Tests.cpp
    src/engine/ECS.h
 "src/engine/MathUtils.h" "src/engine/Systems/EnemyAISystem.h" "src/engine/Systems/EnemyAISystem.cpp")

if(CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET ECS_Tests PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(ECS_Tests PRIVATE Catch2 SDL3::SDL3 SDL3_image::SDL3_image SDL3_mixer::SDL3_mixer)
target_include_directories(ECS_Tests PRIVATE "src/")
target_include_directories(ECS_Tests PRIVATE "src/engine/")
target_include_directories(ECS_Tests PRIVATE "ext/")
target_include_directories(ECS_Tests PRIVATE "ext/glm/")

# Copy SDL DLLs to the executable directory after building
if(WIN32)
    add_custom_command(TARGET SDL_Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/ALPHA/SDL/SDKs/SDL3-3.4.14/lib/x64/SDL3.dll"
            "C:/ALPHA/SDL/SDKs/SDL3_image-3.4.6/lib/x64/SDL3_image.dll"
            "C:/ALPHA/SDL/SDKs/SDL3_mixer-3.2.4/lib/x64/SDL3_mixer.dll"
            $<TARGET_FILE_DIR:SDL_Game>
        COMMENT "Copying SDL DLLs to output directory"
    )
endif()

# --- Asset handling ---
add_custom_command(TARGET SDL_Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data"
        "$<TARGET_FILE_DIR:SDL_Game>/data"
    COMMENT "Copying data/ next to executable"
)

set_property(TARGET SDL_Game PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:SDL_Game>")