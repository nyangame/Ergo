#include "system/platform.hpp"
#include "engine/core/task_system.hpp"
#include "engine/physics/physics_system.hpp"
#include "runtime/engine_context.hpp"
#include "runtime/dll_loader.hpp"
#include <chrono>
#include <cstdio>

int main(int argc, char** argv) {
    // 1. Platform initialization
    ergo::PlatformWindow window;
    if (!window.create(800, 600, "Ergo Engine")) {
        std::fprintf(stderr, "[Ergo] Failed to create window\n");
        return 1;
    }

    ergo::PlatformRenderer renderer;
    if (!renderer.initialize()) {
        std::fprintf(stderr, "[Ergo] Failed to initialize renderer\n");
        return 1;
    }

    ergo::PlatformInput input;

    // 2. Engine systems (g_physics is an inline global in physics_system.hpp)

    // 3. Task manager
    TaskManager task_mgr;

    // 4. Build EngineContext & load game DLL
    auto engine_api = build_engine_api(renderer, input);

    const char* dll_path = (argc > 1) ? argv[1] : "libshooting_game.so";
    auto game = load_game_dll(dll_path);
    if (game.valid()) {
        game.callbacks->on_init(&engine_api);
    } else {
        std::fprintf(stderr, "[Ergo] Running without game DLL\n");
    }

    // 5. Main loop (corresponds to CppSampleGame's while loop)
    auto last_time = std::chrono::high_resolution_clock::now();

    while (!window.should_close()) {
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - last_time).count();
        last_time = now;

        // Event processing
        window.poll_events();
        input.poll_events();

        // DESTROY phase (CppSampleGame: TaskManager::Run(RUN_TYPE::DESTROY))
        task_mgr.run(RunPhase::Destroy, dt);

        // PHYSICS phase (CppSampleGame: TaskManager::Run(RUN_TYPE::PHYSICS))
        task_mgr.run(RunPhase::Physics, dt);

        // UPDATE phase (CppSampleGame: TaskManager::Run(RUN_TYPE::DO))
        task_mgr.run(RunPhase::Update, dt);
        if (game.valid()) {
            game.callbacks->on_update(dt);
        }

        // Collision detection (CppSampleGame: SysPhysics::Run())
        g_physics.run();

        // DRAW phase (CppSampleGame: TaskManager::Run(RUN_TYPE::DRAW))
        renderer.begin_frame();
        auto* ctx = renderer.context();
        task_mgr.run(RunPhase::Draw, dt, ctx);
        if (game.valid()) {
            game.callbacks->on_draw();
        }
        renderer.end_frame();
    }

    // 6. Shutdown (CppSampleGame: SysPhysics::Release(), TaskManager::Release())
    if (game.valid()) {
        game.callbacks->on_shutdown();
    }
    unload_game_dll(game);
    renderer.shutdown();

    return 0;
}
