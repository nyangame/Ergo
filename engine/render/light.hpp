#pragma once
#include "../math/vec3.hpp"
#include "../math/color.hpp"
#include <vector>
#include <cstdint>

enum class LightType : uint8_t { Directional, Point, Spot };

struct Light {
    LightType type = LightType::Directional;
    Vec3f position;
    Vec3f direction{0.0f, -1.0f, 0.0f};
    Color color{255, 255, 255, 255};
    float intensity = 1.0f;
    float range = 10.0f;
    float spot_angle = 45.0f;
    float spot_softness = 0.5f;
    bool enabled = true;
};

class LightManager {
public:
    static constexpr size_t MAX_LIGHTS = 16;

    size_t add_light(Light light) {
        if (lights_.size() >= MAX_LIGHTS) return SIZE_MAX;
        lights_.push_back(std::move(light));
        return lights_.size() - 1;
    }

    void remove_light(size_t index) {
        if (index < lights_.size()) {
            lights_.erase(lights_.begin() + static_cast<ptrdiff_t>(index));
        }
    }

    Light* get_light(size_t index) {
        return (index < lights_.size()) ? &lights_[index] : nullptr;
    }

    const std::vector<Light>& lights() const { return lights_; }
    size_t light_count() const { return lights_.size(); }

    void clear() { lights_.clear(); }

    // Get ambient light color
    Color ambient() const { return ambient_; }
    void set_ambient(Color c) { ambient_ = c; }

private:
    std::vector<Light> lights_;
    Color ambient_{30, 30, 30, 255};
};

inline LightManager g_lights;
