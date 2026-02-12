#include "test_framework.hpp"
#include "engine/math/color.hpp"

TEST_CASE(Color_Default) {
    Color c;
    ASSERT_EQ(c.r, (uint8_t)255);
    ASSERT_EQ(c.g, (uint8_t)255);
    ASSERT_EQ(c.b, (uint8_t)255);
    ASSERT_EQ(c.a, (uint8_t)255);
}

TEST_CASE(Color_Custom) {
    Color c{128, 64, 32, 200};
    ASSERT_EQ(c.r, (uint8_t)128);
    ASSERT_EQ(c.g, (uint8_t)64);
    ASSERT_EQ(c.b, (uint8_t)32);
    ASSERT_EQ(c.a, (uint8_t)200);
}

TEST_CASE(Color_DefaultAlpha) {
    Color c{100, 150, 200};
    ASSERT_EQ(c.a, (uint8_t)255);
}
