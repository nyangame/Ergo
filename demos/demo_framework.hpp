#pragma once
// Demo framework: lightweight runner for feature demonstrations.
// Each demo prints its results to stdout.

#include <cstdio>
#include <string>
#include <vector>
#include <functional>

namespace demo {

struct DemoEntry {
    std::string name;
    std::function<void()> func;
};

inline std::vector<DemoEntry>& registry() {
    static std::vector<DemoEntry> entries;
    return entries;
}

inline void register_demo(const char* name, std::function<void()> func) {
    registry().push_back({name, std::move(func)});
}

inline void run_all() {
    for (auto& d : registry()) {
        std::printf("====================================\n");
        std::printf("  Demo: %s\n", d.name.c_str());
        std::printf("====================================\n");
        d.func();
        std::printf("\n");
    }
    std::printf("=== All %zu demos completed ===\n", registry().size());
}

} // namespace demo

#define DEMO(name) \
    static void demo_##name(); \
    namespace { struct DemoReg_##name { \
        DemoReg_##name() { demo::register_demo(#name, demo_##name); } \
    } demo_reg_##name##_instance; } \
    static void demo_##name()
