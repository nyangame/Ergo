#include "demo_framework.hpp"
#include "engine/core/easing.hpp"
#include <cstdio>

DEMO(Easing_Functions) {
    float t_values[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};

    std::printf("  %-16s", "t");
    for (float t : t_values) std::printf(" %7.2f", t);
    std::printf("\n");

    std::printf("  %-16s", "linear");
    for (float t : t_values) std::printf(" %7.4f", easing::linear(t));
    std::printf("\n");

    std::printf("  %-16s", "in_quad");
    for (float t : t_values) std::printf(" %7.4f", easing::in_quad(t));
    std::printf("\n");

    std::printf("  %-16s", "out_quad");
    for (float t : t_values) std::printf(" %7.4f", easing::out_quad(t));
    std::printf("\n");

    std::printf("  %-16s", "in_out_cubic");
    for (float t : t_values) std::printf(" %7.4f", easing::in_out_cubic(t));
    std::printf("\n");

    std::printf("  %-16s", "in_elastic");
    for (float t : t_values) std::printf(" %7.4f", easing::in_elastic(t));
    std::printf("\n");

    std::printf("  %-16s", "out_bounce");
    for (float t : t_values) std::printf(" %7.4f", easing::out_bounce(t));
    std::printf("\n");

    std::printf("  %-16s", "in_back");
    for (float t : t_values) std::printf(" %7.4f", easing::in_back(t));
    std::printf("\n");

    std::printf("  %-16s", "out_expo");
    for (float t : t_values) std::printf(" %7.4f", easing::out_expo(t));
    std::printf("\n");
}
