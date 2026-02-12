#pragma once
#include "../math/vec3.hpp"
#include "../math/mat4.hpp"
#include "../math/color.hpp"
#include <vector>
#include <cstdint>
#include <string>

// Maximum bones influencing a single vertex
inline constexpr uint32_t kMaxBoneInfluences = 4;

// Maximum bones per skeleton (uniform array size in shader)
inline constexpr uint32_t kMaxBones = 128;

// Vertex format for skinned 3D meshes
struct SkinnedVertex {
    Vec3f position;
    Vec3f normal;
    float uv_x = 0.0f;
    float uv_y = 0.0f;

    // Bone skinning data
    int32_t bone_indices[kMaxBoneInfluences] = {0, 0, 0, 0};
    float bone_weights[kMaxBoneInfluences] = {0.0f, 0.0f, 0.0f, 0.0f};

    // Assign a bone influence to the next available slot
    void add_bone(int32_t bone_index, float weight) {
        for (uint32_t i = 0; i < kMaxBoneInfluences; ++i) {
            if (bone_weights[i] == 0.0f) {
                bone_indices[i] = bone_index;
                bone_weights[i] = weight;
                return;
            }
        }
        // All slots full — replace the smallest weight if this one is larger
        uint32_t min_idx = 0;
        for (uint32_t i = 1; i < kMaxBoneInfluences; ++i) {
            if (bone_weights[i] < bone_weights[min_idx]) min_idx = i;
        }
        if (weight > bone_weights[min_idx]) {
            bone_indices[min_idx] = bone_index;
            bone_weights[min_idx] = weight;
        }
    }

    // Normalize bone weights so they sum to 1.0
    void normalize_weights() {
        float sum = 0.0f;
        for (uint32_t i = 0; i < kMaxBoneInfluences; ++i) sum += bone_weights[i];
        if (sum > 0.0f) {
            float inv = 1.0f / sum;
            for (uint32_t i = 0; i < kMaxBoneInfluences; ++i) bone_weights[i] *= inv;
        }
    }
};

// Submesh: a contiguous range of indices sharing one material
struct SkinnedSubMesh {
    uint32_t index_offset = 0;
    uint32_t index_count = 0;
    uint32_t material_index = 0;
};

// CPU-side skinned mesh data (uploaded to GPU at load time)
struct SkinnedMeshData {
    uint64_t id = 0;
    std::string name;
    std::vector<SkinnedVertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<SkinnedSubMesh> submeshes;

    // GPU handles (set by the renderer backend after upload)
    uint64_t gpu_vertex_buffer = 0;
    uint64_t gpu_index_buffer = 0;
    bool uploaded = false;
};
