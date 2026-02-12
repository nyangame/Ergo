#include "test_framework.hpp"
#include "engine/render/light.hpp"

TEST_CASE(LightManager_AddLight) {
    LightManager mgr;
    Light l;
    l.type = LightType::Directional;
    l.intensity = 1.5f;
    size_t idx = mgr.add_light(l);
    ASSERT_EQ(idx, (size_t)0);
    ASSERT_EQ(mgr.light_count(), (size_t)1);
}

TEST_CASE(LightManager_GetLight) {
    LightManager mgr;
    Light l;
    l.type = LightType::Point;
    l.intensity = 2.0f;
    l.range = 10.0f;
    mgr.add_light(l);

    auto* found = mgr.get_light(0);
    ASSERT_TRUE(found != nullptr);
    ASSERT_NEAR(found->intensity, 2.0f, 0.001f);
    ASSERT_NEAR(found->range, 10.0f, 0.001f);
}

TEST_CASE(LightManager_RemoveLight) {
    LightManager mgr;
    Light l1; l1.type = LightType::Directional;
    Light l2; l2.type = LightType::Point;
    mgr.add_light(l1);
    mgr.add_light(l2);

    mgr.remove_light(0);
    ASSERT_EQ(mgr.light_count(), (size_t)1);
}

TEST_CASE(LightManager_MaxLights) {
    LightManager mgr;
    for (size_t i = 0; i < LightManager::MAX_LIGHTS; ++i) {
        Light l;
        mgr.add_light(l);
    }
    ASSERT_EQ(mgr.light_count(), LightManager::MAX_LIGHTS);

    // Adding beyond max should fail
    size_t idx = mgr.add_light(Light{});
    ASSERT_EQ(idx, SIZE_MAX);
    ASSERT_EQ(mgr.light_count(), LightManager::MAX_LIGHTS);
}

TEST_CASE(LightManager_Ambient) {
    LightManager mgr;
    mgr.set_ambient({100, 120, 140, 255});
    ASSERT_EQ(mgr.ambient().r, (uint8_t)100);
    ASSERT_EQ(mgr.ambient().g, (uint8_t)120);
    ASSERT_EQ(mgr.ambient().b, (uint8_t)140);
}

TEST_CASE(LightManager_Clear) {
    LightManager mgr;
    mgr.add_light(Light{});
    mgr.add_light(Light{});
    mgr.clear();
    ASSERT_EQ(mgr.light_count(), (size_t)0);
}
