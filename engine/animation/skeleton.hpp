#pragma once
#include "../math/mat4.hpp"
#include "../math/vec3.hpp"
#include "../math/quat.hpp"
#include <string>
#include <vector>
#include <string_view>

struct Bone {
    std::string name;
    int32_t parent_index = -1;
    Mat4 local_bind_pose;
    Mat4 inverse_bind_pose;
};

struct Skeleton {
    std::vector<Bone> bones;

    int32_t find_bone(std::string_view name) const {
        for (size_t i = 0; i < bones.size(); ++i) {
            if (bones[i].name == name) return static_cast<int32_t>(i);
        }
        return -1;
    }

    size_t bone_count() const { return bones.size(); }
};
