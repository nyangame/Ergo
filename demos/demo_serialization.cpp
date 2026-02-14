#include "demo_framework.hpp"
#include "engine/core/serialization.hpp"
#include <cstdio>

DEMO(Serialization_JsonValue) {
    using namespace ergo;

    JsonValue num(42.0);
    std::printf("  Number: %s\n", num.to_string().c_str());

    JsonValue str("hello");
    std::printf("  String: %s\n", str.to_string().c_str());

    JsonValue boolean(true);
    std::printf("  Bool: %s\n", boolean.to_string().c_str());

    JsonValue arr(JsonArray{JsonValue(1), JsonValue(2), JsonValue(3)});
    std::printf("  Array: %s\n", arr.to_string().c_str());

    JsonValue obj(JsonObject{
        {"name", JsonValue("Ergo")},
        {"version", JsonValue(1.0)},
        {"active", JsonValue(true)}
    });
    std::printf("  Object: %s\n", obj.to_string().c_str());
}

DEMO(Serialization_EngineTypes) {
    using namespace ergo;

    Vec2f v2{3.14f, 2.71f};
    auto j2 = serialize(v2);
    auto v2_back = deserialize_vec2f(j2);
    std::printf("  Vec2f: (%.2f, %.2f) -> JSON -> (%.2f, %.2f)\n",
                v2.x, v2.y, v2_back.x, v2_back.y);

    Vec3f v3{1.0f, 2.0f, 3.0f};
    auto j3 = serialize(v3);
    auto v3_back = deserialize_vec3f(j3);
    std::printf("  Vec3f: (%.1f, %.1f, %.1f) -> JSON -> (%.1f, %.1f, %.1f)\n",
                v3.x, v3.y, v3.z, v3_back.x, v3_back.y, v3_back.z);

    Color c{255, 128, 0, 200};
    auto jc = serialize(c);
    auto c_back = deserialize_color(jc);
    std::printf("  Color: (%d,%d,%d,%d) -> JSON -> (%d,%d,%d,%d)\n",
                c.r, c.g, c.b, c.a, c_back.r, c_back.g, c_back.b, c_back.a);

    Size2f sz{800.0f, 600.0f};
    auto jsz = serialize(sz);
    auto sz_back = deserialize_size2f(jsz);
    std::printf("  Size2f: (%.1f, %.1f) -> JSON -> (%.1f, %.1f)\n",
                sz.w, sz.h, sz_back.w, sz_back.h);

    Quat q = Quat::from_axis_angle(Vec3f::up(), 1.57f);
    auto jq = serialize(q);
    auto q_back = deserialize_quat(jq);
    std::printf("  Quat: (%.4f,%.4f,%.4f,%.4f) -> JSON -> (%.4f,%.4f,%.4f,%.4f)\n",
                q.x, q.y, q.z, q.w, q_back.x, q_back.y, q_back.z, q_back.w);
}
