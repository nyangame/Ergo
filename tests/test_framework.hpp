#pragma once
// Minimal test framework (no external dependencies)
// Replace with Catch2 or Google Test for production use.

#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>
#include <functional>

namespace test {

struct TestCase {
    std::string name;
    std::function<void()> func;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> cases;
    return cases;
}

inline int g_pass = 0;
inline int g_fail = 0;

inline void register_test(const char* name, std::function<void()> func) {
    registry().push_back({name, std::move(func)});
}

inline int run_all() {
    g_pass = 0;
    g_fail = 0;
    for (auto& tc : registry()) {
        std::printf("  [RUN ] %s\n", tc.name.c_str());
        try {
            tc.func();
            std::printf("  [PASS] %s\n", tc.name.c_str());
        } catch (const std::exception& e) {
            ++g_fail;
            std::printf("  [FAIL] %s: %s\n", tc.name.c_str(), e.what());
        }
    }
    std::printf("\nResults: %d passed, %d failed, %d total\n",
                g_pass, g_fail, g_pass + g_fail);
    return g_fail > 0 ? 1 : 0;
}

struct AssertFailed : std::exception {
    std::string msg;
    AssertFailed(std::string m) : msg(std::move(m)) {}
    const char* what() const noexcept override { return msg.c_str(); }
};

} // namespace test

#define TEST_CASE(name) \
    static void test_##name(); \
    namespace { struct Register_##name { \
        Register_##name() { test::register_test(#name, test_##name); } \
    } register_##name##_instance; } \
    static void test_##name()

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) throw test::AssertFailed( \
        std::string(__FILE__) + ":" + std::to_string(__LINE__) + ": " #expr " is false"); \
    ++test::g_pass; } while(0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

#define ASSERT_EQ(a, b) do { \
    if (!((a) == (b))) throw test::AssertFailed( \
        std::string(__FILE__) + ":" + std::to_string(__LINE__) + ": " #a " != " #b); \
    ++test::g_pass; } while(0)

#define ASSERT_NEAR(a, b, eps) do { \
    if (std::abs((a) - (b)) > (eps)) throw test::AssertFailed( \
        std::string(__FILE__) + ":" + std::to_string(__LINE__) + ": " #a " !~ " #b + \
        " (diff=" + std::to_string(std::abs((a)-(b))) + ")"); \
    ++test::g_pass; } while(0)
