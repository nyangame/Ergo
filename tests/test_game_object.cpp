#include "test_framework.hpp"
#include "engine/core/game_object.hpp"

namespace {
struct Health { int hp = 100; };
struct Speed { float value = 5.0f; };
} // namespace

TEST_CASE(GameObject_DefaultValues) {
    GameObject obj;
    ASSERT_EQ(obj.id, (uint64_t)0);
    ASSERT_EQ(obj.object_type(), (uint32_t)0);
}

TEST_CASE(GameObject_NameAndType) {
    GameObject obj;
    obj.name_ = "Enemy";
    obj.object_type_ = 42;
    ASSERT_TRUE(obj.name() == "Enemy");
    ASSERT_EQ(obj.object_type(), (uint32_t)42);
}

TEST_CASE(GameObject_Transform) {
    GameObject obj;
    obj.transform().position = {10.0f, 20.0f};
    obj.transform().rotation = 1.5f;
    obj.transform().size = {32.0f, 32.0f};
    ASSERT_NEAR(obj.transform().position.x, 10.0f, 0.001f);
    ASSERT_NEAR(obj.transform().position.y, 20.0f, 0.001f);
    ASSERT_NEAR(obj.transform().rotation, 1.5f, 0.001f);
}

TEST_CASE(GameObject_AddGetComponent) {
    GameObject obj;
    obj.add_component(Health{80});
    obj.add_component(Speed{3.5f});

    auto* hp = obj.get_component<Health>();
    ASSERT_TRUE(hp != nullptr);
    ASSERT_EQ(hp->hp, 80);

    auto* spd = obj.get_component<Speed>();
    ASSERT_TRUE(spd != nullptr);
    ASSERT_NEAR(spd->value, 3.5f, 0.001f);
}

TEST_CASE(GameObject_MissingComponent) {
    GameObject obj;
    obj.add_component(Health{100});
    auto* spd = obj.get_component<Speed>();
    ASSERT_TRUE(spd == nullptr);
}

TEST_CASE(GameObject_OverwriteComponent) {
    GameObject obj;
    obj.add_component(Health{100});
    obj.add_component(Health{50});

    auto* hp = obj.get_component<Health>();
    ASSERT_TRUE(hp != nullptr);
    ASSERT_EQ(hp->hp, 50);
}
