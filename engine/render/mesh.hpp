#pragma once
#include "../math/vec3.hpp"
#include "../math/color.hpp"
#include <vector>
#include <cstdint>
#include <string>

// Vertex format for 3D meshes
struct Vertex {
    Vec3f position;
    Vec3f normal;
    float uv_x = 0.0f;
    float uv_y = 0.0f;
};

// Submesh: a contiguous range of indices sharing one material
struct SubMesh {
    uint32_t index_offset = 0;
    uint32_t index_count = 0;
    uint32_t material_index = 0;
};

// CPU-side mesh data (uploaded to GPU at load time)
struct MeshData {
    uint64_t id = 0;
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<SubMesh> submeshes;

    // GPU handles (set by the renderer backend after upload)
    uint64_t gpu_vertex_buffer = 0;
    uint64_t gpu_index_buffer = 0;
    bool uploaded = false;
};

// Material data
struct MaterialData {
    uint64_t id = 0;
    std::string name;
    Color diffuse_color{255, 255, 255, 255};
    float metallic = 0.0f;
    float roughness = 0.8f;
    uint64_t diffuse_texture = 0;
    uint64_t normal_texture = 0;
};
