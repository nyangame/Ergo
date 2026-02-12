#include "test_framework.hpp"
#include "engine/physics/spatial_grid.hpp"

TEST_CASE(SpatialGrid_CellSize) {
    SpatialGrid2D grid(128.0f);
    ASSERT_NEAR(grid.cell_size(), 128.0f, 0.001f);
}

TEST_CASE(SpatialGrid_InsertAndQuery) {
    SpatialGrid2D grid(64.0f);

    Transform2D t1; t1.position = {10.0f, 10.0f};
    Collider c1; c1.handle = {1}; c1.transform = &t1;
    c1.shape = AABBData{Vec2f{5.0f, 5.0f}};

    grid.insert(&c1);

    auto results = grid.query({0.0f, 0.0f}, {64.0f, 64.0f});
    ASSERT_EQ(results.size(), (size_t)1);
}

TEST_CASE(SpatialGrid_QueryOutOfRange) {
    SpatialGrid2D grid(64.0f);

    Transform2D t1; t1.position = {10.0f, 10.0f};
    Collider c1; c1.handle = {1}; c1.transform = &t1;
    c1.shape = AABBData{Vec2f{5.0f, 5.0f}};

    grid.insert(&c1);

    auto results = grid.query({500.0f, 500.0f}, {600.0f, 600.0f});
    ASSERT_EQ(results.size(), (size_t)0);
}

TEST_CASE(SpatialGrid_MultipleColliders) {
    SpatialGrid2D grid(64.0f);

    Transform2D t1; t1.position = {10.0f, 10.0f};
    Transform2D t2; t2.position = {20.0f, 20.0f};
    Transform2D t3; t3.position = {500.0f, 500.0f};

    Collider c1; c1.handle = {1}; c1.transform = &t1;
    c1.shape = AABBData{Vec2f{5.0f, 5.0f}};
    Collider c2; c2.handle = {2}; c2.transform = &t2;
    c2.shape = CircleData{5.0f};
    Collider c3; c3.handle = {3}; c3.transform = &t3;
    c3.shape = AABBData{Vec2f{5.0f, 5.0f}};

    grid.insert(&c1);
    grid.insert(&c2);
    grid.insert(&c3);

    auto near = grid.query({0.0f, 0.0f}, {64.0f, 64.0f});
    ASSERT_EQ(near.size(), (size_t)2);

    auto far = grid.query({450.0f, 450.0f}, {550.0f, 550.0f});
    ASSERT_EQ(far.size(), (size_t)1);
}

TEST_CASE(SpatialGrid_Clear) {
    SpatialGrid2D grid(64.0f);

    Transform2D t1; t1.position = {10.0f, 10.0f};
    Collider c1; c1.handle = {1}; c1.transform = &t1;
    c1.shape = AABBData{Vec2f{5.0f, 5.0f}};
    grid.insert(&c1);

    grid.clear();
    auto results = grid.query({0.0f, 0.0f}, {64.0f, 64.0f});
    ASSERT_EQ(results.size(), (size_t)0);
}
