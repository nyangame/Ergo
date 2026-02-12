#pragma once
#include "../render/mesh.hpp"
#include <string_view>
#include <vector>
#include <optional>
#include <cstdint>

// FBX binary file loader
// Parses FBX binary format and extracts mesh geometry + materials
// Supports FBX 7100-7700 (binary format)

struct FbxLoadResult {
    std::vector<MeshData> meshes;
    std::vector<MaterialData> materials;
    bool success = false;
    std::string error;
};

class FbxLoader {
    // FBX binary header
    struct FbxHeader {
        char magic[21];    // "Kaydara FBX Binary  \0"
        uint8_t padding[2];
        uint32_t version;
    };

    // FBX node in the binary tree
    struct FbxNode {
        std::string name;
        uint64_t end_offset = 0;
        uint64_t property_count = 0;
        uint64_t property_list_len = 0;
        std::vector<FbxNode> children;

        // Property data (simplified - stores raw bytes)
        std::vector<uint8_t> property_data;
    };

    // Parse helpers
    static bool parse_header(const uint8_t* data, size_t size, FbxHeader& out);
    static bool parse_node(const uint8_t* data, size_t size, size_t& offset,
                           FbxNode& out, uint32_t version);
    static void parse_geometry(const FbxNode& node, MeshData& mesh);
    static void parse_material(const FbxNode& node, MaterialData& material);

    // Extract float array from FBX property data
    static std::vector<float> extract_float_array(const uint8_t* data, size_t size);
    static std::vector<int32_t> extract_int_array(const uint8_t* data, size_t size);

public:
    // Load FBX from file path
    static FbxLoadResult load(std::string_view path);

    // Load FBX from memory buffer
    static FbxLoadResult load_from_memory(const uint8_t* data, size_t size);
};
