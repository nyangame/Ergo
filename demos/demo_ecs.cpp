#include "demo_framework.hpp"
#include "engine/ecs/world.hpp"
#include <cstdio>

namespace {

struct Position {
    float x, y;
};

struct Velocity {
    float vx, vy;
};

struct Health {
    int hp;
};

} // namespace

DEMO(ECS_EntityLifecycle) {
    World world;

    auto e1 = world.create_entity();
    auto e2 = world.create_entity();
    auto e3 = world.create_entity();
    std::printf("  Created entities: %llu, %llu, %llu\n",
                (unsigned long long)e1, (unsigned long long)e2, (unsigned long long)e3);
    std::printf("  Entity count: %zu\n", world.entity_count());

    world.destroy_entity(e2);
    std::printf("  After destroying e2: count=%zu, e2 exists=%s\n",
                world.entity_count(), world.entity_exists(e2) ? "yes" : "no");
}

DEMO(ECS_Components) {
    World world;

    auto player = world.create_entity();
    world.add_component(player, Position{10.0f, 20.0f});
    world.add_component(player, Velocity{1.0f, 0.5f});
    world.add_component(player, Health{100});

    auto bullet = world.create_entity();
    world.add_component(bullet, Position{0.0f, 0.0f});
    world.add_component(bullet, Velocity{10.0f, 0.0f});

    std::printf("  Player has Position: %s\n", world.has_component<Position>(player) ? "yes" : "no");
    std::printf("  Player has Health:   %s\n", world.has_component<Health>(player) ? "yes" : "no");
    std::printf("  Bullet has Health:   %s\n", world.has_component<Health>(bullet) ? "yes" : "no");

    auto* pos = world.get_component<Position>(player);
    if (pos) std::printf("  Player position: (%.1f, %.1f)\n", pos->x, pos->y);
}

DEMO(ECS_Query) {
    World world;

    for (int i = 0; i < 5; ++i) {
        auto e = world.create_entity();
        world.add_component(e, Position{static_cast<float>(i * 10), 0.0f});
        world.add_component(e, Velocity{1.0f, 0.0f});
    }
    // Add an entity without Velocity
    auto static_entity = world.create_entity();
    world.add_component(static_entity, Position{999.0f, 999.0f});

    std::printf("  Querying entities with Position + Velocity:\n");
    int count = 0;
    world.each<Position, Velocity>([&](uint64_t id, Position& pos, Velocity& vel) {
        pos.x += vel.vx;
        std::printf("    Entity %llu: pos=(%.1f, %.1f)\n",
                    (unsigned long long)id, pos.x, pos.y);
        ++count;
    });
    std::printf("  Matched %d entities (static entity excluded)\n", count);
}
