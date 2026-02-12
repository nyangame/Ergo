#include "fbx_loader.hpp"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cstdio>

// FBX binary magic bytes
static constexpr char FBX_MAGIC[] = "Kaydara FBX Binary  ";
static constexpr size_t FBX_HEADER_SIZE = 27;

// Helper: read little-endian values from byte buffer
template<typename T>
static T read_le(const uint8_t* p) {
    T val{};
    std::memcpy(&val, p, sizeof(T));
    return val;
}

bool FbxLoader::parse_header(const uint8_t* data, size_t size, FbxHeader& out) {
    if (size < FBX_HEADER_SIZE) return false;
    if (std::memcmp(data, FBX_MAGIC, 20) != 0) return false;

    std::memcpy(out.magic, data, 21);
    out.padding[0] = data[21];
    out.padding[1] = data[22];
    out.version = read_le<uint32_t>(data + 23);
    return true;
}

bool FbxLoader::parse_node(const uint8_t* data, size_t size, size_t& offset,
                            FbxNode& out, uint32_t version) {
    // Node record format depends on FBX version
    // Version >= 7500 uses 64-bit offsets, earlier uses 32-bit
    bool use64 = (version >= 7500);
    size_t header_size = use64 ? 25 : 13;

    if (offset + header_size > size) return false;

    if (use64) {
        out.end_offset = read_le<uint64_t>(data + offset);
        out.property_count = read_le<uint64_t>(data + offset + 8);
        out.property_list_len = read_le<uint64_t>(data + offset + 16);
    } else {
        out.end_offset = read_le<uint32_t>(data + offset);
        out.property_count = read_le<uint32_t>(data + offset + 4);
        out.property_list_len = read_le<uint32_t>(data + offset + 8);
    }

    // Null node (sentinel)
    if (out.end_offset == 0) {
        offset += header_size;
        return true;
    }

    size_t name_offset = offset + (use64 ? 24 : 12);
    if (name_offset >= size) return false;

    uint8_t name_len = data[name_offset];
    name_offset += 1;

    if (name_offset + name_len > size) return false;
    out.name.assign(reinterpret_cast<const char*>(data + name_offset), name_len);

    // Skip property data
    size_t prop_start = name_offset + name_len;
    if (prop_start + out.property_list_len > size) return false;

    // Store raw property data for later extraction
    out.property_data.assign(data + prop_start,
                              data + prop_start + out.property_list_len);

    size_t child_offset = prop_start + out.property_list_len;

    // Parse children until end_offset
    while (child_offset < out.end_offset && child_offset < size) {
        FbxNode child;
        size_t prev = child_offset;
        if (!parse_node(data, size, child_offset, child, version)) break;
        if (child.end_offset == 0) break;  // Null sentinel
        if (child_offset == prev) break;    // No progress
        out.children.push_back(std::move(child));
    }

    offset = out.end_offset;
    return true;
}

std::vector<float> FbxLoader::extract_float_array(const uint8_t* data, size_t size) {
    std::vector<float> result;
    if (size < 1) return result;

    // FBX property type: 'd' = double array, 'f' = float array
    char type = static_cast<char>(data[0]);
    size_t offset = 1;

    if (type == 'f' || type == 'd') {
        if (offset + 12 > size) return result;
        uint32_t count = read_le<uint32_t>(data + offset);
        // uint32_t encoding = read_le<uint32_t>(data + offset + 4);
        // uint32_t compressed_len = read_le<uint32_t>(data + offset + 8);
        offset += 12;

        result.reserve(count);
        if (type == 'f') {
            for (uint32_t i = 0; i < count && offset + 4 <= size; ++i) {
                result.push_back(read_le<float>(data + offset));
                offset += 4;
            }
        } else { // 'd'
            for (uint32_t i = 0; i < count && offset + 8 <= size; ++i) {
                result.push_back(static_cast<float>(read_le<double>(data + offset)));
                offset += 8;
            }
        }
    }
    return result;
}

std::vector<int32_t> FbxLoader::extract_int_array(const uint8_t* data, size_t size) {
    std::vector<int32_t> result;
    if (size < 1) return result;

    char type = static_cast<char>(data[0]);
    size_t offset = 1;

    if (type == 'i') {
        if (offset + 12 > size) return result;
        uint32_t count = read_le<uint32_t>(data + offset);
        offset += 12;

        result.reserve(count);
        for (uint32_t i = 0; i < count && offset + 4 <= size; ++i) {
            result.push_back(read_le<int32_t>(data + offset));
            offset += 4;
        }
    }
    return result;
}

void FbxLoader::parse_geometry(const FbxNode& node, MeshData& mesh) {
    for (const auto& child : node.children) {
        if (child.name == "Vertices" && !child.property_data.empty()) {
            auto floats = extract_float_array(
                child.property_data.data(), child.property_data.size());
            mesh.vertices.resize(floats.size() / 3);
            for (size_t i = 0; i + 2 < floats.size(); i += 3) {
                mesh.vertices[i / 3].position = {floats[i], floats[i + 1], floats[i + 2]};
            }
        }
        else if (child.name == "PolygonVertexIndex" && !child.property_data.empty()) {
            auto ints = extract_int_array(
                child.property_data.data(), child.property_data.size());
            // FBX polygon indices: negative value marks end of polygon (XOR with -1)
            for (size_t i = 0; i < ints.size(); ++i) {
                int32_t idx = ints[i];
                if (idx < 0) idx = ~idx;  // Decode end-of-polygon marker
                mesh.indices.push_back(static_cast<uint32_t>(idx));
            }
        }
        else if (child.name == "LayerElementNormal") {
            for (const auto& sub : child.children) {
                if (sub.name == "Normals" && !sub.property_data.empty()) {
                    auto floats = extract_float_array(
                        sub.property_data.data(), sub.property_data.size());
                    for (size_t i = 0; i + 2 < floats.size() && i / 3 < mesh.vertices.size(); i += 3) {
                        mesh.vertices[i / 3].normal = {floats[i], floats[i + 1], floats[i + 2]};
                    }
                }
            }
        }
        else if (child.name == "LayerElementUV") {
            for (const auto& sub : child.children) {
                if (sub.name == "UV" && !sub.property_data.empty()) {
                    auto floats = extract_float_array(
                        sub.property_data.data(), sub.property_data.size());
                    for (size_t i = 0; i + 1 < floats.size() && i / 2 < mesh.vertices.size(); i += 2) {
                        mesh.vertices[i / 2].uv_x = floats[i];
                        mesh.vertices[i / 2].uv_y = floats[i + 1];
                    }
                }
            }
        }
    }

    // Default submesh covering all indices
    if (!mesh.indices.empty()) {
        mesh.submeshes.push_back({0, static_cast<uint32_t>(mesh.indices.size()), 0});
    }
}

void FbxLoader::parse_material(const FbxNode& node, MaterialData& material) {
    for (const auto& child : node.children) {
        if (child.name == "Properties70") {
            // Parse material properties (P nodes within Properties70)
            for (const auto& prop : child.children) {
                if (prop.name == "P" && prop.property_data.size() > 0) {
                    // Property parsing is simplified here
                    // Full implementation would decode property name + value
                }
            }
        }
    }
    if (material.name.empty()) {
        material.name = node.name;
    }
}

FbxLoadResult FbxLoader::load(std::string_view path) {
    std::ifstream file(std::string(path), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {{}, {}, false, "Failed to open file"};
    }

    auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return {{}, {}, false, "Failed to read file"};
    }

    return load_from_memory(buffer.data(), buffer.size());
}

FbxLoadResult FbxLoader::load_from_memory(const uint8_t* data, size_t size) {
    FbxLoadResult result;
    result.success = false;

    FbxHeader header;
    if (!parse_header(data, size, header)) {
        result.error = "Invalid FBX header";
        return result;
    }

    // Parse top-level nodes
    size_t offset = FBX_HEADER_SIZE;
    std::vector<FbxNode> root_nodes;

    while (offset < size) {
        FbxNode node;
        size_t prev = offset;
        if (!parse_node(data, size, offset, node, header.version)) break;
        if (node.end_offset == 0) break;
        if (offset == prev) break;
        root_nodes.push_back(std::move(node));
    }

    // Extract geometry and materials from Objects node
    for (const auto& root : root_nodes) {
        if (root.name == "Objects") {
            for (const auto& obj : root.children) {
                if (obj.name == "Geometry") {
                    MeshData mesh;
                    mesh.name = obj.name;
                    parse_geometry(obj, mesh);
                    if (!mesh.vertices.empty()) {
                        result.meshes.push_back(std::move(mesh));
                    }
                }
                else if (obj.name == "Material") {
                    MaterialData mat;
                    parse_material(obj, mat);
                    result.materials.push_back(std::move(mat));
                }
            }
        }
    }

    result.success = true;
    return result;
}
