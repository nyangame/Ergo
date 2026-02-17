#include "framework/test_framework.hpp"

using namespace ergo::test;

// Forward declarations — each test file registers its suites
void register_math_tests(TestRunner& runner);
void register_physics_tests(TestRunner& runner);
void register_core_tests(TestRunner& runner);
void register_gameplay_tests(TestRunner& runner);
void register_physics_extended_tests(TestRunner& runner);
void register_render_tests(TestRunner& runner);
void register_ui_shader_tests(TestRunner& runner);
void register_animation_debug_tests(TestRunner& runner);
void register_ecs_task_tests(TestRunner& runner);
void register_particle_emitter_component_tests(TestRunner& runner);

int main() {
    TestRunner runner;

    register_math_tests(runner);
    register_physics_tests(runner);
    register_core_tests(runner);
    register_gameplay_tests(runner);
    register_physics_extended_tests(runner);
    register_render_tests(runner);
    register_ui_shader_tests(runner);
    register_animation_debug_tests(runner);
    register_ecs_task_tests(runner);
    register_particle_emitter_component_tests(runner);

    return runner.run();
}
