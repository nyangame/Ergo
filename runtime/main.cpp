#include "system/platform.hpp"
#include "engine/core/task_system.hpp"
#include "engine/physics/physics_system.hpp"
#include "engine/physics/rigid_body_world.hpp"
#include "engine/render/render_pipeline.hpp"
#include "runtime/engine_context.hpp"
#include "runtime/dll_loader.hpp"
#include <chrono>
#include <cstdio>

int main(int argc, char** argv) {
    // -------------------------------------------------------
    // 1. Platform initialization (System Assembly)
    // -------------------------------------------------------
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

    // -------------------------------------------------------
    // 2. Engine systems
    // -------------------------------------------------------
    // g_physics (2D) is an inline global in physics_system.hpp
    // g_rigid_body_world (3D) is an inline global in rigid_body_world.hpp

    // Render pipeline with multi-CPU worker threads
    RenderPipeline render_pipeline;
    render_pipeline.initialize(0);  // 0 = auto-detect thread count

    // Task manager
    TaskManager task_mgr;

    // -------------------------------------------------------
    // 3. Application Assembly: load game DLL
    // -------------------------------------------------------
    auto engine_api = build_engine_api(renderer, input);

    const char* dll_path = (argc > 1) ? argv[1] : "libshooting_game.so";
    auto game = load_game_dll(dll_path);
    if (game.valid()) {
        game.callbacks->on_init(&engine_api);
    } else {
        std::fprintf(stderr, "[Ergo] Running without game DLL\n");
    }

    // -------------------------------------------------------
    // 4. Main loop
    // -------------------------------------------------------
    auto last_time = std::chrono::high_resolution_clock::now();

    while (!window.should_close()) {
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - last_time).count();
        last_time = now;

        // --- Event processing ---
        window.poll_events();
        input.poll_events();

        // --- DESTROY phase: remove dead tasks ---
        task_mgr.run(RunPhase::Destroy, dt);

        // --- PHYSICS phase: task physics + 2D collisions + 3D rigid body ---
        task_mgr.run(RunPhase::Physics, dt);
        g_physics.run();
        g_rigid_body_world.step(dt);

        // --- UPDATE phase: task updates + game update ---
        task_mgr.run(RunPhase::Update, dt);
        if (game.valid()) {
            game.callbacks->on_update(dt);
        }

        // --- DRAW phase: render pipeline ---
        render_pipeline.begin_frame();
        renderer.begin_frame();

        auto* ctx = renderer.context();
        task_mgr.run(RunPhase::Draw, dt, ctx);
        if (game.valid()) {
            game.callbacks->on_draw();
        }

        render_pipeline.end_frame();
        renderer.end_frame();
    }

    // -------------------------------------------------------
    // 5. Shutdown
    // -------------------------------------------------------
    if (game.valid()) {
        game.callbacks->on_shutdown();
    }
    unload_game_dll(game);
    render_pipeline.shutdown();
    renderer.shutdown();

    return 0;
}
