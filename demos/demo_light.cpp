#include "demo_framework.hpp"
#include "engine/render/light.hpp"
#include <cstdio>

DEMO(LightManager_Lights) {
    LightManager mgr;
    mgr.set_ambient({50, 50, 60, 255});

    Light dir;
    dir.type = LightType::Directional;
    dir.direction = {-0.5f, -1.0f, -0.3f};
    dir.color = {255, 245, 230, 255};
    dir.intensity = 1.0f;
    mgr.add_light(dir);

    Light point;
    point.type = LightType::Point;
    point.position = {5.0f, 3.0f, 2.0f};
    point.color = {255, 200, 100, 255};
    point.intensity = 2.0f;
    point.range = 15.0f;
    mgr.add_light(point);

    Light spot;
    spot.type = LightType::Spot;
    spot.position = {0.0f, 10.0f, 0.0f};
    spot.direction = {0.0f, -1.0f, 0.0f};
    spot.color = {255, 255, 255, 255};
    spot.intensity = 3.0f;
    spot.spot_angle = 30.0f;
    spot.spot_softness = 0.8f;
    mgr.add_light(spot);

    std::printf("  Lights: %zu / %zu max\n", mgr.light_count(), LightManager::MAX_LIGHTS);
    std::printf("  Ambient: (%d, %d, %d)\n",
                mgr.ambient().r, mgr.ambient().g, mgr.ambient().b);

    for (size_t i = 0; i < mgr.light_count(); ++i) {
        auto* l = mgr.get_light(i);
        const char* type_str = "?";
        if (l->type == LightType::Directional) type_str = "Directional";
        else if (l->type == LightType::Point) type_str = "Point";
        else if (l->type == LightType::Spot) type_str = "Spot";
        std::printf("    [%zu] %s: intensity=%.1f\n", i, type_str, l->intensity);
    }

    mgr.remove_light(1);
    std::printf("  After removing light 1: %zu lights\n", mgr.light_count());
}
