#include "test_framework.hpp"
#include "engine/core/serialization.hpp"

using namespace ergo;

TEST_CASE(JsonValue_Null) {
    JsonValue v;
    ASSERT_TRUE(v.is_null());
}

TEST_CASE(JsonValue_Bool) {
    JsonValue v(true);
    ASSERT_TRUE(v.is_bool());
    ASSERT_TRUE(v.bool_val);
}

TEST_CASE(JsonValue_Number) {
    JsonValue v(42.0);
    ASSERT_TRUE(v.is_number());
    ASSERT_NEAR(v.number_val, 42.0, 0.001);
    ASSERT_EQ(v.as_int(), 42);
}

TEST_CASE(JsonValue_String) {
    JsonValue v("hello");
    ASSERT_TRUE(v.is_string());
    ASSERT_TRUE(v.string_val == "hello");
}

TEST_CASE(JsonValue_Array) {
    JsonValue v(JsonArray{JsonValue(1), JsonValue(2), JsonValue(3)});
    ASSERT_TRUE(v.is_array());
    ASSERT_EQ(v.array_val.size(), (size_t)3);
    ASSERT_NEAR(v[0].number_val, 1.0, 0.001);
    ASSERT_NEAR(v[2].number_val, 3.0, 0.001);
}

TEST_CASE(JsonValue_Object) {
    JsonValue v(JsonObject{{"key", JsonValue("value")}});
    ASSERT_TRUE(v.is_object());
    ASSERT_TRUE(v["key"].is_string());
    ASSERT_TRUE(v["key"].string_val == "value");
}

TEST_CASE(Serialize_Vec2f) {
    Vec2f original{3.14f, 2.71f};
    auto json = serialize(original);
    auto result = deserialize_vec2f(json);
    ASSERT_NEAR(result.x, 3.14f, 0.001f);
    ASSERT_NEAR(result.y, 2.71f, 0.001f);
}

TEST_CASE(Serialize_Vec3f) {
    Vec3f original{1.0f, 2.0f, 3.0f};
    auto json = serialize(original);
    auto result = deserialize_vec3f(json);
    ASSERT_NEAR(result.x, 1.0f, 0.001f);
    ASSERT_NEAR(result.y, 2.0f, 0.001f);
    ASSERT_NEAR(result.z, 3.0f, 0.001f);
}

TEST_CASE(Serialize_Color) {
    Color original{255, 128, 64, 200};
    auto json = serialize(original);
    auto result = deserialize_color(json);
    ASSERT_EQ(result.r, (uint8_t)255);
    ASSERT_EQ(result.g, (uint8_t)128);
    ASSERT_EQ(result.b, (uint8_t)64);
    ASSERT_EQ(result.a, (uint8_t)200);
}

TEST_CASE(Serialize_Size2f) {
    Size2f original{800.0f, 600.0f};
    auto json = serialize(original);
    auto result = deserialize_size2f(json);
    ASSERT_NEAR(result.w, 800.0f, 0.001f);
    ASSERT_NEAR(result.h, 600.0f, 0.001f);
}

TEST_CASE(Serialize_Quat) {
    Quat original = Quat::from_axis_angle(Vec3f::up(), 1.57f);
    auto json = serialize(original);
    auto result = deserialize_quat(json);
    ASSERT_NEAR(result.x, original.x, 0.001f);
    ASSERT_NEAR(result.y, original.y, 0.001f);
    ASSERT_NEAR(result.z, original.z, 0.001f);
    ASSERT_NEAR(result.w, original.w, 0.001f);
}
