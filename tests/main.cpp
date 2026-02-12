#include "framework/test_framework.hpp"

using namespace ergo::test;

// Forward declarations — each test file registers its suites
void register_math_tests(TestRunner& runner);
void register_physics_tests(TestRunner& runner);
void register_core_tests(TestRunner& runner);

int main() {
    TestRunner runner;

    register_math_tests(runner);
    register_physics_tests(runner);
    register_core_tests(runner);

    return runner.run();
}
