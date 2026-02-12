#include "test_framework.hpp"
#include "engine/core/id_generator.hpp"

TEST_CASE(IdGenerator_Unique) {
    auto a = IdGenerator::next();
    auto b = IdGenerator::next();
    auto c = IdGenerator::next();
    ASSERT_TRUE(a != b);
    ASSERT_TRUE(b != c);
    ASSERT_TRUE(a != c);
}

TEST_CASE(IdGenerator_Nonzero) {
    auto id = IdGenerator::next();
    ASSERT_TRUE(id != 0);
}

TEST_CASE(IdGenerator_Monotonic) {
    auto a = IdGenerator::next();
    auto b = IdGenerator::next();
    ASSERT_TRUE(b > a);
}
