#include "test_framework.hpp"
#include "engine/core/easing.hpp"

TEST_CASE(Easing_Linear) {
    ASSERT_NEAR(easing::linear(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(easing::linear(0.5f), 0.5f, 0.001f);
    ASSERT_NEAR(easing::linear(1.0f), 1.0f, 0.001f);
}

TEST_CASE(Easing_InQuad_Endpoints) {
    ASSERT_NEAR(easing::in_quad(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(easing::in_quad(1.0f), 1.0f, 0.001f);
}

TEST_CASE(Easing_OutQuad_Endpoints) {
    ASSERT_NEAR(easing::out_quad(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(easing::out_quad(1.0f), 1.0f, 0.001f);
}

TEST_CASE(Easing_InOutQuad_Midpoint) {
    ASSERT_NEAR(easing::in_out_quad(0.5f), 0.5f, 0.001f);
}

TEST_CASE(Easing_InCubic) {
    ASSERT_NEAR(easing::in_cubic(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(easing::in_cubic(1.0f), 1.0f, 0.001f);
    ASSERT_NEAR(easing::in_cubic(0.5f), 0.125f, 0.001f);
}

TEST_CASE(Easing_OutCubic) {
    ASSERT_NEAR(easing::out_cubic(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(easing::out_cubic(1.0f), 1.0f, 0.001f);
}

TEST_CASE(Easing_Sine_Endpoints) {
    ASSERT_NEAR(easing::in_sine(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(easing::in_sine(1.0f), 1.0f, 0.001f);
    ASSERT_NEAR(easing::out_sine(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(easing::out_sine(1.0f), 1.0f, 0.001f);
}

TEST_CASE(Easing_Expo_Endpoints) {
    ASSERT_NEAR(easing::in_expo(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(easing::in_expo(1.0f), 1.0f, 0.001f);
    ASSERT_NEAR(easing::out_expo(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(easing::out_expo(1.0f), 1.0f, 0.001f);
}

TEST_CASE(Easing_Elastic_Endpoints) {
    ASSERT_NEAR(easing::in_elastic(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(easing::in_elastic(1.0f), 1.0f, 0.001f);
    ASSERT_NEAR(easing::out_elastic(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(easing::out_elastic(1.0f), 1.0f, 0.001f);
}

TEST_CASE(Easing_Bounce_Endpoints) {
    ASSERT_NEAR(easing::out_bounce(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(easing::out_bounce(1.0f), 1.0f, 0.001f);
    ASSERT_NEAR(easing::in_bounce(0.0f), 0.0f, 0.001f);
    ASSERT_NEAR(easing::in_bounce(1.0f), 1.0f, 0.001f);
}

TEST_CASE(Easing_Back_Overshoots) {
    // in_back should go negative before 0.5
    float early = easing::in_back(0.1f);
    ASSERT_TRUE(early < 0.0f);

    // out_back should overshoot past 1.0
    float late = easing::out_back(0.9f);
    ASSERT_TRUE(late > 1.0f);
}
