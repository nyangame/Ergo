#include "demo_framework.hpp"
#include "engine/render/render_command.hpp"
#include "engine/render/command_buffer.hpp"
#include "engine/render/double_buffer.hpp"
#include <cstdio>

DEMO(RenderCommand_CommandBuffer) {
    CommandBuffer buf;

    buf.push(RenderCmd_Clear{{0, 0, 0, 255}, 1.0f});
    buf.push(RenderCmd_SetViewProjection{Mat4{}, Mat4{}});
    buf.push(RenderCmd_DrawRect{{10.0f, 20.0f, 0.0f}, 100.0f, 50.0f, {255, 0, 0, 255}, true});
    buf.push(RenderCmd_DrawCircle{{200.0f, 200.0f, 0.0f}, 30.0f, {0, 255, 0, 255}, true});
    buf.push(RenderCmd_DrawDebugLine{{0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 0.0f}, {255, 255, 0, 255}});

    std::printf("  Buffer commands: %zu\n", buf.size());

    for (const auto& cmd : buf.commands()) {
        std::visit([](const auto& c) {
            using T = std::decay_t<decltype(c)>;
            if constexpr (std::is_same_v<T, RenderCmd_Clear>)
                std::printf("    Clear: color=(%d,%d,%d)\n", c.color.r, c.color.g, c.color.b);
            else if constexpr (std::is_same_v<T, RenderCmd_SetViewProjection>)
                std::printf("    SetViewProjection\n");
            else if constexpr (std::is_same_v<T, RenderCmd_DrawRect>)
                std::printf("    DrawRect: pos=(%.1f,%.1f) size=%.1fx%.1f\n",
                            c.position.x, c.position.y, c.width, c.height);
            else if constexpr (std::is_same_v<T, RenderCmd_DrawCircle>)
                std::printf("    DrawCircle: center=(%.1f,%.1f) r=%.1f\n",
                            c.center.x, c.center.y, c.radius);
            else if constexpr (std::is_same_v<T, RenderCmd_DrawDebugLine>)
                std::printf("    DrawDebugLine\n");
            else
                std::printf("    (other command)\n");
        }, cmd);
    }
}

DEMO(RenderCommand_DoubleBuffer) {
    DoubleBufferedCommands db;

    // Write to back buffer
    db.write_buffer().push(RenderCmd_Clear{{30, 30, 30, 255}, 1.0f});
    db.write_buffer().push(RenderCmd_DrawRect{{0, 0, 0}, 64, 64, {255, 0, 0, 255}, true});
    std::printf("  Back buffer (write): %zu commands\n", db.write_buffer().size());
    std::printf("  Front buffer (read): %zu commands\n", db.read_buffer().size());

    db.swap();
    std::printf("  After swap:\n");
    std::printf("  Back buffer (write): %zu commands\n", db.write_buffer().size());
    std::printf("  Front buffer (read): %zu commands\n", db.read_buffer().size());
}

DEMO(RenderCommand_SharedCollector) {
    SharedCommandCollector collector;

    // Simulate multi-threaded submission
    CommandBuffer thread1;
    thread1.push(RenderCmd_DrawRect{{0, 0, 0}, 10, 10, {255, 0, 0, 255}, true});
    thread1.push(RenderCmd_DrawRect{{20, 0, 0}, 10, 10, {0, 255, 0, 255}, true});

    CommandBuffer thread2;
    thread2.push(RenderCmd_DrawCircle{{50, 50, 0}, 25, {0, 0, 255, 255}, true});

    collector.submit(thread1);
    collector.submit(thread2);

    auto merged = collector.take();
    std::printf("  Thread1: 2 commands, Thread2: 1 command\n");
    std::printf("  Merged total: %zu commands\n", merged.size());
}
