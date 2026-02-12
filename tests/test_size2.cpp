#include "test_framework.hpp"
#include "engine/math/size2.hpp"

TEST_CASE(Size2f_Default) {
    Size2f s;
    ASSERT_NEAR(s.w, 0.0f, 0.001f);
    ASSERT_NEAR(s.h, 0.0f, 0.001f);
}

TEST_CASE(Size2f_Custom) {
    Size2f s{100.0f, 60.0f};
    ASSERT_NEAR(s.w, 100.0f, 0.001f);
    ASSERT_NEAR(s.h, 60.0f, 0.001f);
}

TEST_CASE(Size2f_HalfW) {
    Size2f s{200.0f, 100.0f};
    ASSERT_NEAR(s.half_w(), 100.0f, 0.001f);
}

TEST_CASE(Size2f_HalfH) {
    Size2f s{200.0f, 100.0f};
    ASSERT_NEAR(s.half_h(), 50.0f, 0.001f);
}

TEST_CASE(Size2f_Radius) {
    Size2f s{30.0f, 30.0f};
    ASSERT_NEAR(s.radius(), 15.0f, 0.001f);
}
