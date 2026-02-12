#include "demo_framework.hpp"
#include "engine/core/scene_manager.hpp"
#include <cstdio>
#include <string>

// RenderContext is forward-declared in scene_manager.hpp; define it here for demo
struct RenderContext {};

namespace {

struct TitleScene : Scene {
    void on_enter() override { std::printf("    TitleScene::on_enter()\n"); }
    void on_exit() override { std::printf("    TitleScene::on_exit()\n"); }
    void on_update(float dt) override { std::printf("    TitleScene::on_update(dt=%.3f)\n", dt); }
    void on_draw(RenderContext&) override { std::printf("    TitleScene::on_draw()\n"); }
    void on_pause() override { std::printf("    TitleScene::on_pause()\n"); }
    void on_resume() override { std::printf("    TitleScene::on_resume()\n"); }
};

struct GameScene : Scene {
    void on_enter() override { std::printf("    GameScene::on_enter()\n"); }
    void on_exit() override { std::printf("    GameScene::on_exit()\n"); }
    void on_update(float dt) override { std::printf("    GameScene::on_update(dt=%.3f)\n", dt); }
    void on_draw(RenderContext&) override { std::printf("    GameScene::on_draw()\n"); }
};

struct PauseScene : Scene {
    void on_enter() override { std::printf("    PauseScene::on_enter()\n"); }
    void on_exit() override { std::printf("    PauseScene::on_exit()\n"); }
    void on_update(float dt) override { std::printf("    PauseScene::on_update(dt=%.3f)\n", dt); }
    void on_draw(RenderContext&) override { std::printf("    PauseScene::on_draw()\n"); }
};

} // namespace

DEMO(SceneManager_ChangeScene) {
    SceneManager mgr;

    std::printf("  Change to TitleScene:\n");
    mgr.change(std::make_unique<TitleScene>());
    mgr.update(0.016f);

    std::printf("  Change to GameScene:\n");
    mgr.change(std::make_unique<GameScene>());
    mgr.update(0.016f);

    std::printf("  Stack size: %zu\n", mgr.stack_size());
}

DEMO(SceneManager_PushPop) {
    SceneManager mgr;

    mgr.change(std::make_unique<GameScene>());
    std::printf("  Stack size: %zu\n", mgr.stack_size());

    std::printf("  Push PauseScene:\n");
    mgr.push(std::make_unique<PauseScene>());
    std::printf("  Stack size: %zu\n", mgr.stack_size());
    mgr.update(0.016f);

    std::printf("  Pop (return to GameScene):\n");
    mgr.pop();
    std::printf("  Stack size: %zu\n", mgr.stack_size());
    mgr.update(0.016f);
}
