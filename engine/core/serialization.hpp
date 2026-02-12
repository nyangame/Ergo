#pragma once
#include "../math/vec2.hpp"
#include "../math/vec3.hpp"
#include "../math/color.hpp"
#include "../math/size2.hpp"
#include "../math/quat.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <variant>
#include <sstream>
#include <fstream>

// Lightweight JSON-like value for serialization without external dependencies.
// For production use, replace with nlohmann/json or RapidJSON.
namespace ergo {

struct JsonValue;
using JsonObject = std::unordered_map<std::string, JsonValue>;
using JsonArray = std::vector<JsonValue>;

struct JsonValue {
    enum Type { Null, Bool, Number, String, Array, Object };

    Type type = Null;
    bool bool_val = false;
    double number_val = 0.0;
    std::string string_val;
    JsonArray array_val;
    JsonObject object_val;

    JsonValue() = default;
    JsonValue(bool b) : type(Bool), bool_val(b) {}
    JsonValue(double n) : type(Number), number_val(n) {}
    JsonValue(float n) : type(Number), number_val(static_cast<double>(n)) {}
    JsonValue(int n) : type(Number), number_val(static_cast<double>(n)) {}
    JsonValue(const char* s) : type(String), string_val(s) {}
    JsonValue(std::string s) : type(String), string_val(std::move(s)) {}
    JsonValue(JsonArray a) : type(Array), array_val(std::move(a)) {}
    JsonValue(JsonObject o) : type(Object), object_val(std::move(o)) {}

    bool is_null() const { return type == Null; }
    bool is_bool() const { return type == Bool; }
    bool is_number() const { return type == Number; }
    bool is_string() const { return type == String; }
    bool is_array() const { return type == Array; }
    bool is_object() const { return type == Object; }

    float as_float() const { return static_cast<float>(number_val); }
    int as_int() const { return static_cast<int>(number_val); }

    const JsonValue& operator[](const std::string& key) const {
        static JsonValue null_val;
        if (type != Object) return null_val;
        auto it = object_val.find(key);
        return (it != object_val.end()) ? it->second : null_val;
    }

    const JsonValue& operator[](size_t index) const {
        static JsonValue null_val;
        if (type != Array || index >= array_val.size()) return null_val;
        return array_val[index];
    }

    // Simple serialization to JSON string
    std::string to_string(int indent = 0) const;
};

// Serialization helpers for engine types
inline JsonValue serialize(Vec2f v) {
    return JsonObject{{"x", v.x}, {"y", v.y}};
}
inline Vec2f deserialize_vec2f(const JsonValue& j) {
    return {j["x"].as_float(), j["y"].as_float()};
}

inline JsonValue serialize(Vec3f v) {
    return JsonObject{{"x", v.x}, {"y", v.y}, {"z", v.z}};
}
inline Vec3f deserialize_vec3f(const JsonValue& j) {
    return {j["x"].as_float(), j["y"].as_float(), j["z"].as_float()};
}

inline JsonValue serialize(Color c) {
    return JsonObject{{"r", c.r}, {"g", c.g}, {"b", c.b}, {"a", c.a}};
}
inline Color deserialize_color(const JsonValue& j) {
    return {
        static_cast<uint8_t>(j["r"].as_int()),
        static_cast<uint8_t>(j["g"].as_int()),
        static_cast<uint8_t>(j["b"].as_int()),
        static_cast<uint8_t>(j["a"].as_int())
    };
}

inline JsonValue serialize(Size2f s) {
    return JsonObject{{"w", s.w}, {"h", s.h}};
}
inline Size2f deserialize_size2f(const JsonValue& j) {
    return {j["w"].as_float(), j["h"].as_float()};
}

inline JsonValue serialize(Quat q) {
    return JsonObject{{"x", q.x}, {"y", q.y}, {"z", q.z}, {"w", q.w}};
}
inline Quat deserialize_quat(const JsonValue& j) {
    return {j["x"].as_float(), j["y"].as_float(), j["z"].as_float(), j["w"].as_float()};
}

// File I/O
inline bool save_json(const std::string& path, const JsonValue& val) {
    std::ofstream f(path);
    if (!f.is_open()) return false;
    f << val.to_string(2);
    return true;
}

} // namespace ergo
