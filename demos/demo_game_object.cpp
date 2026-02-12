#include "demo_framework.hpp"
#include "engine/core/game_object.hpp"
#include <cstdio>
#include <string>

namespace {

struct Health {
    int current = 100;
    int max = 100;
};

struct Velocity {
    float vx = 0.0f;
    float vy = 0.0f;
};

struct Tag {
    std::string value;
};

} // namespace

DEMO(GameObject_Components) {
    GameObject obj;
    obj.id = 1;
    obj.name_ = "Player";
    obj.object_type_ = 1;
    obj.transform_.position = {100.0f, 200.0f};
    obj.transform_.size = {32.0f, 48.0f};

    std::printf("  Object: id=%llu name='%s' type=%u\n",
                (unsigned long long)obj.id,
                std::string(obj.name()).c_str(),
                obj.object_type());
    std::printf("  Position: (%.1f, %.1f)\n",
                obj.transform().position.x, obj.transform().position.y);

    // Add components
    obj.add_component(Health{80, 100});
    obj.add_component(Velocity{5.0f, -2.0f});
    obj.add_component(Tag{"hero"});

    // Retrieve components
    auto* hp = obj.get_component<Health>();
    if (hp) std::printf("  Health: %d/%d\n", hp->current, hp->max);

    auto* vel = obj.get_component<Velocity>();
    if (vel) std::printf("  Velocity: (%.1f, %.1f)\n", vel->vx, vel->vy);

    auto* tag = obj.get_component<Tag>();
    if (tag) std::printf("  Tag: '%s'\n", tag->value.c_str());

    // Nonexistent component returns nullptr
    auto* missing = obj.get_component<int>();
    std::printf("  Missing component: %s\n", missing ? "found" : "nullptr (correct)");
}
