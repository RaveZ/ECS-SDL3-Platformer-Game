cmake_minimum_required(VERSION 3.8)
project("SDL_Game" C CXX)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_mixer REQUIRED)

add_executable(SDL_Game 
    "src/engine/Timer.h" 
    "src/engine/Animation.h" 
    "src/engine/GameObject.h" 
    "src/engine/EntityManager.cpp"
    "src/engine/EntityManager.h"
    "src/engine/Components/TransformComponent.h" 
    "src/engine/Components/RenderComponent.h"
    "src/engine/Components/CollisionComponent.h"
    "src/engine/Components/InputComponent.h" 
    "src/Main.cpp" 
    "src/Application.h" 
    "src/Application.cpp"
 "src/engine/GameState.h" "src/Resources.h" "src/Resources.cpp" "src/engine/SDLState.h" "src/engine/Systems/RenderSystem.cpp" "src/engine/Systems/RenderSystem.h" "src/engine/Systems/MovementSystem.h" "src/engine/Systems/MovementSystem.cpp" "src/engine/Components/PhysicsComponent.h" "src/engine/Systems/CollisionSystem.h" "src/engine/Systems/CollisionSystem.cpp" "src/engine/Systems/InputSystem.h" "src/engine/Systems/InputSystem.cpp" "src/engine/Components/AnimationComponent.h" "src/engine/Systems/AnimationSystem.h" "src/engine/Systems/AnimationSystem.cpp" "src/engine/Components/HealthComponent.h" "src/engine/Systems/LifetimeSystem.h" "src/engine/Systems/LifetimeSystem.cpp" "src/engine/Components/LifetimeComponent.h" "src/engine/Components/TagComponent.h" "src/engine/Systems/AnimationStateSystem.h" "src/engine/Systems/AnimationStateSystem.cpp")

if(CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET SDL_Game PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(SDL_Game PRIVATE SDL3::SDL3 SDL3_image::SDL3_image SDL3_mixer::SDL3_mixer)
target_include_directories(SDL_Game PRIVATE "ext/")
target_include_directories(SDL_Game PRIVATE "engine/")
target_include_directories(SDL_Game PRIVATE "ext/glm/")

# --- Asset handling ---

# 1) Copy data/ next to the exe after each build
add_custom_command(TARGET SDL_Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/data"
        "$<TARGET_FILE_DIR:SDL_Game>/data"
    COMMENT "Copying data/ next to executable"
)

# 2) Run from the exe's directory so relative paths resolve
set_property(TARGET SDL_Game PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:SDL_Game>")