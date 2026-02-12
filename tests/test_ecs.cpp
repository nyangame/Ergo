#include "test_framework.hpp"
#include "engine/ecs/world.hpp"

namespace {

struct Position {
    float x = 0, y = 0;
};

struct Velocity {
    float dx = 0, dy = 0;
};

struct Health {
    int hp = 100;
};

} // anonymous namespace

TEST_CASE(ecs_create_entity) {
    World world;
    uint64_t e1 = world.create_entity();
    uint64_t e2 = world.create_entity();
    ASSERT_TRUE(e1 != e2);
    ASSERT_TRUE(world.entity_exists(e1));
    ASSERT_TRUE(world.entity_exists(e2));
    ASSERT_EQ(world.entity_count(), 2u);
}

TEST_CASE(ecs_destroy_entity) {
    World world;
    uint64_t e = world.create_entity();
    world.destroy_entity(e);
    ASSERT_FALSE(world.entity_exists(e));
    ASSERT_EQ(world.entity_count(), 0u);
}

TEST_CASE(ecs_add_and_get_component) {
    World world;
    uint64_t e = world.create_entity();
    world.add_component(e, Position{10.0f, 20.0f});

    auto* pos = world.get_component<Position>(e);
    ASSERT_TRUE(pos != nullptr);
    ASSERT_NEAR(pos->x, 10.0f, 1e-6f);
    ASSERT_NEAR(pos->y, 20.0f, 1e-6f);
}

TEST_CASE(ecs_has_component) {
    World world;
    uint64_t e = world.create_entity();
    ASSERT_FALSE(world.has_component<Position>(e));

    world.add_component(e, Position{});
    ASSERT_TRUE(world.has_component<Position>(e));
    ASSERT_FALSE(world.has_component<Velocity>(e));
}

TEST_CASE(ecs_query_entities) {
    World world;
    uint64_t e1 = world.create_entity();
    world.add_component(e1, Position{1.0f, 0.0f});
    world.add_component(e1, Velocity{2.0f, 0.0f});

    uint64_t e2 = world.create_entity();
    world.add_component(e2, Position{3.0f, 0.0f});
    // e2 has no Velocity

    int count = 0;
    world.each<Position, Velocity>([&](uint64_t id, Position& pos, Velocity& vel) {
        (void)id;
        pos.x += vel.dx;
        ++count;
    });

    ASSERT_EQ(count, 1);

    auto* pos = world.get_component<Position>(e1);
    ASSERT_TRUE(pos != nullptr);
    ASSERT_NEAR(pos->x, 3.0f, 1e-6f);  // 1.0 + 2.0
}
