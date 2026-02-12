#include "demo_framework.hpp"
#include "engine/core/state_machine.hpp"
#include <cstdio>

namespace {

struct MenuState {
    void enter() { std::printf("    MenuState::enter()\n"); }
    void update(float dt) { std::printf("    MenuState::update(dt=%.3f)\n", dt); }
    void exit() { std::printf("    MenuState::exit()\n"); }
};

struct PlayState {
    int score = 0;
    void enter() { std::printf("    PlayState::enter() score=%d\n", score); }
    void update(float dt) {
        score += 10;
        std::printf("    PlayState::update(dt=%.3f) score=%d\n", dt, score);
    }
    void exit() { std::printf("    PlayState::exit() final_score=%d\n", score); }
};

struct PauseState {
    void enter() { std::printf("    PauseState::enter()\n"); }
    void update(float dt) { std::printf("    PauseState::update(dt=%.3f)\n", dt); }
    void exit() { std::printf("    PauseState::exit()\n"); }
};

} // namespace

DEMO(StateMachine_Transitions) {
    StateMachine<MenuState, PlayState, PauseState> sm;

    std::printf("  Transition to MenuState:\n");
    sm.transition<MenuState>();
    std::printf("  is MenuState? %s\n", sm.is_state<MenuState>() ? "yes" : "no");

    std::printf("  Update:\n");
    sm.update(0.016f);

    std::printf("  Transition to PlayState:\n");
    sm.transition<PlayState>();
    std::printf("  is PlayState? %s\n", sm.is_state<PlayState>() ? "yes" : "no");
    sm.update(0.016f);
    sm.update(0.016f);

    std::printf("  Transition to PauseState:\n");
    sm.transition<PauseState>();
    sm.update(0.016f);

    std::printf("  Back to PlayState:\n");
    sm.transition<PlayState>();
    sm.update(0.016f);
}
