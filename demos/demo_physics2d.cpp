#include "demo_framework.hpp"
#include "engine/physics/hit_test.hpp"
#include "engine/physics/spatial_grid.hpp"
#include <cstdio>

DEMO(Physics2D_AABB_Collision) {
    AABBData boxA{Vec2f{16.0f, 16.0f}};
    AABBData boxB{Vec2f{16.0f, 16.0f}};

    Transform2D ta; ta.position = {0.0f, 0.0f};
    Transform2D tb; tb.position = {20.0f, 0.0f};

    bool hit = hit_test(boxA, ta, boxB, tb);
    std::printf("  AABB(0,0) vs AABB(20,0): %s\n", hit ? "HIT" : "no hit");

    tb.position = {100.0f, 100.0f};
    hit = hit_test(boxA, ta, boxB, tb);
    std::printf("  AABB(0,0) vs AABB(100,100): %s\n", hit ? "HIT" : "no hit");
}

DEMO(Physics2D_Circle_Collision) {
    CircleData c1{10.0f};
    CircleData c2{10.0f};

    Transform2D ta; ta.position = {0.0f, 0.0f};
    Transform2D tb; tb.position = {15.0f, 0.0f};

    bool hit = hit_test(c1, ta, c2, tb);
    std::printf("  Circle(r=10, 0,0) vs Circle(r=10, 15,0): %s\n", hit ? "HIT" : "no hit");

    tb.position = {25.0f, 0.0f};
    hit = hit_test(c1, ta, c2, tb);
    std::printf("  Circle(r=10, 0,0) vs Circle(r=10, 25,0): %s\n", hit ? "HIT" : "no hit");
}

DEMO(Physics2D_Mixed_Collision) {
    CircleData circle{10.0f};
    AABBData aabb{Vec2f{16.0f, 16.0f}};

    Transform2D tc; tc.position = {0.0f, 0.0f};
    Transform2D ta; ta.position = {20.0f, 0.0f};

    bool hit = hit_test(circle, tc, aabb, ta);
    std::printf("  Circle(r=10, 0,0) vs AABB(16x16, 20,0): %s\n", hit ? "HIT" : "no hit");
}

DEMO(Physics2D_SpatialGrid) {
    SpatialGrid2D grid(64.0f);
    std::printf("  Cell size: %.1f\n", grid.cell_size());

    Transform2D t1; t1.position = {10.0f, 10.0f};
    Transform2D t2; t2.position = {500.0f, 500.0f};
    Transform2D t3; t3.position = {30.0f, 30.0f};

    Collider c1; c1.handle = {1}; c1.transform = &t1;
    c1.shape = AABBData{Vec2f{8.0f, 8.0f}};

    Collider c2; c2.handle = {2}; c2.transform = &t2;
    c2.shape = CircleData{10.0f};

    Collider c3; c3.handle = {3}; c3.transform = &t3;
    c3.shape = AABBData{Vec2f{8.0f, 8.0f}};

    grid.insert(&c1);
    grid.insert(&c2);
    grid.insert(&c3);

    auto near = grid.query({0.0f, 0.0f}, {64.0f, 64.0f});
    std::printf("  Query (0,0)-(64,64): found %zu colliders\n", near.size());

    auto far = grid.query({400.0f, 400.0f}, {600.0f, 600.0f});
    std::printf("  Query (400,400)-(600,600): found %zu colliders\n", far.size());
}
