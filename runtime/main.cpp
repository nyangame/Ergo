#include "system/platform.hpp"
#include "engine/core/task_system.hpp"
#include "engine/core/job_system.hpp"
#include "engine/core/time.hpp"
#include "engine/core/log.hpp"
#include "engine/physics/physics_system.hpp"
#include "engine/physics/rigid_body_world.hpp"
#include "engine/render/render_pipeline.hpp"
#include "engine/debug/profiler.hpp"
#include "engine/core/tween.hpp"
#include "engine/resource/resource_manager.hpp"
#include "runtime/engine_context.hpp"
#include "runtime/dll_loader.hpp"
#include "runtime/plugin_loader.hpp"
#include <chrono>
#include <cstring>

int main(int argc, char** argv) {
    // -------------------------------------------------------
    // 1. Platform initialization (System Assembly)
    // -------------------------------------------------------
    ergo::log::set_level(LogLevel::Info);
    ERGO_LOG_INFO("Engine", "Ergo Engine starting...");

    ergo::PlatformWindow window;
    if (!window.create(800, 600, "Ergo Engine")) {
        ERGO_LOG_ERROR("Engine", "Failed to create window");
        return 1;
    }

    ergo::PlatformRenderer renderer;
    if (!renderer.initialize()) {
        ERGO_LOG_ERROR("Engine", "Failed to initialize renderer");
        return 1;
    }

    ergo::PlatformInput input;

    // -------------------------------------------------------
    // 2. Engine systems
    // -------------------------------------------------------
    // g_physics (2D) is an inline global in physics_system.hpp
    // g_rigid_body_world (3D) is an inline global in rigid_body_world.hpp

    // Job system for data-parallel work (ECS, physics, etc.)
    g_job_system.initialize(0);  // 0 = auto-detect thread count
    ERGO_LOG_INFO("Engine", "JobSystem initialized with %u workers", g_job_system.worker_count());

    // Render pipeline with multi-CPU worker threads
    RenderPipeline render_pipeline;
    render_pipeline.initialize(0);  // 0 = auto-detect thread count

    // Task manager
    TaskManager task_mgr;

    // Frame rate limiter
    FrameRateLimiter fps_limiter;
    fps_limiter.target_fps = 60.0f;

    // -------------------------------------------------------
    // 3. Application Assembly: load game DLL + plugins
    // -------------------------------------------------------
    auto engine_api = build_engine_api(renderer, input);

    const char* dll_path = (argc > 1) ? argv[1] : "libshooting_game.so";
    auto game = load_game_dll(dll_path);
    if (game.valid()) {
        game.callbacks->on_init(&engine_api);
        ERGO_LOG_INFO("Engine", "Game DLL loaded: %s", dll_path);
    } else {
        ERGO_LOG_WARN("Engine", "Running without game DLL");
    }

    // Load plugin DLLs (arguments after --plugin)
    PluginManager plugin_mgr;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--plugin") == 0 && i + 1 < argc) {
            uint64_t pid = plugin_mgr.load(argv[++i]);
            if (pid) {
                ERGO_LOG_INFO("Engine", "Plugin loaded (id=%llu)", pid);
            }
        }
    }
    plugin_mgr.init_all(&engine_api);

    // -------------------------------------------------------
    // 4. Main loop
    // -------------------------------------------------------
    auto last_time = std::chrono::high_resolution_clock::now();

    while (!window.should_close()) {
        fps_limiter.begin_frame();

        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - last_time).count();
        last_time = now;

        // Update global time
        g_time.tick(dt);

        // --- Event processing ---
        window.poll_events();
        input.poll_events();

        // --- DESTROY phase: remove dead tasks ---
        g_profiler.begin("Destroy");
        task_mgr.run(RunPhase::Destroy, g_time.delta_time);
        g_profiler.end();

        // --- PHYSICS phase: task physics + 2D collisions + 3D rigid body ---
        g_profiler.begin("Physics");
        task_mgr.run(RunPhase::Physics, g_time.delta_time);
        g_physics.run();
        g_rigid_body_world.step(g_time.delta_time);
        g_profiler.end();

        // --- UPDATE phase: task updates + game update + plugins ---
        g_profiler.begin("Update");
        task_mgr.run(RunPhase::Update, g_time.delta_time);
        if (game.valid()) {
            game.callbacks->on_update(g_time.delta_time);
        }
        plugin_mgr.update_all(g_time.delta_time);
        g_tweens.update(g_time.delta_time);
        g_profiler.end();

        // --- DRAW phase: render pipeline ---
        g_profiler.begin("Draw");
        render_pipeline.begin_frame();
        renderer.begin_frame();

        auto* ctx = renderer.context();
        task_mgr.run(RunPhase::Draw, g_time.delta_time, ctx);
        if (game.valid()) {
            game.callbacks->on_draw();
        }
        plugin_mgr.draw_all();

        render_pipeline.end_frame();
        renderer.end_frame();
        g_profiler.end();

        // Frame rate limiting
        fps_limiter.wait();
    }

    // -------------------------------------------------------
    // 5. Shutdown
    // -------------------------------------------------------
    ERGO_LOG_INFO("Engine", "Shutting down... (ran %llu frames)", g_time.frame_count);
    plugin_mgr.unload_all();
    if (game.valid()) {
        game.callbacks->on_shutdown();
    }
    unload_game_dll(game);
    g_resources.shutdown();
    render_pipeline.shutdown();
    g_job_system.shutdown();
    renderer.shutdown();
    ergo::log::close_file();

    return 0;
}
