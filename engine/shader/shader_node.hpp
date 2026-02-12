#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <array>
#include "../math/vec2.hpp"
#include "../math/vec3.hpp"
#include "../math/color.hpp"

// ============================================================
// Shader data types flowing through node ports
// ============================================================

enum class ShaderDataType : uint8_t {
    Float,
    Vec2,
    Vec3,
    Vec4,
    Mat3,
    Mat4,
    Texture2D,
    Sampler,
    Bool
};

inline const char* shader_data_type_name(ShaderDataType t) {
    switch (t) {
    case ShaderDataType::Float:     return "float";
    case ShaderDataType::Vec2:      return "vec2";
    case ShaderDataType::Vec3:      return "vec3";
    case ShaderDataType::Vec4:      return "vec4";
    case ShaderDataType::Mat3:      return "mat3";
    case ShaderDataType::Mat4:      return "mat4";
    case ShaderDataType::Texture2D: return "texture2D";
    case ShaderDataType::Sampler:   return "sampler";
    case ShaderDataType::Bool:      return "bool";
    }
    return "unknown";
}

// ============================================================
// Default values for ports
// ============================================================

struct ShaderValue {
    std::variant<
        float,
        std::array<float, 2>,
        std::array<float, 3>,
        std::array<float, 4>,
        bool
    > data = 0.0f;

    static ShaderValue from_float(float v) { return {v}; }
    static ShaderValue from_vec2(float x, float y) { return {std::array<float,2>{x, y}}; }
    static ShaderValue from_vec3(float x, float y, float z) { return {std::array<float,3>{x, y, z}}; }
    static ShaderValue from_vec4(float x, float y, float z, float w) { return {std::array<float,4>{x, y, z, w}}; }
    static ShaderValue from_bool(bool v) { return {v}; }
};

// ============================================================
// Port: connection point on a node
// ============================================================

struct ShaderPort {
    uint32_t id = 0;
    std::string name;
    ShaderDataType data_type = ShaderDataType::Float;
    ShaderValue default_value;
};

// ============================================================
// Connection between two ports
// ============================================================

struct ShaderConnection {
    uint64_t id = 0;
    uint32_t source_node = 0;
    uint32_t source_port = 0;   // output port index
    uint32_t target_node = 0;
    uint32_t target_port = 0;   // input port index
};

// ============================================================
// Node operation types
// ============================================================

enum class MathOp : uint8_t {
    Add, Subtract, Multiply, Divide,
    Power, SquareRoot, Abs,
    Min, Max, Clamp, Lerp,
    Dot, Cross, Normalize, Length,
    Negate, Fract, Floor, Ceil,
    Step, SmoothStep
};

enum class TrigOp : uint8_t {
    Sin, Cos, Tan, Asin, Acos, Atan, Atan2
};

enum class TextureOp : uint8_t {
    Sample, SampleLod, SampleGrad, SampleBias
};

enum class ChannelMask : uint8_t {
    R, G, B, A,
    RG, RGB, RGBA,
    XY, XYZ, XYZW
};

enum class BlendMode : uint8_t {
    Normal, Additive, Multiply, Screen, Overlay
};

enum class CompareOp : uint8_t {
    Equal, NotEqual, Greater, Less, GreaterEqual, LessEqual
};

enum class UvOp : uint8_t {
    TilingOffset, Rotate, Polar, Spherical, Triplanar
};

enum class LightModel : uint8_t {
    Lambert, BlinnPhong, CookTorrance, Toon, Unlit
};

// ============================================================
// Node type definitions (variant-based, no inheritance)
// ============================================================

struct NodePropertyFloat {
    float value = 0.0f;
    float min_value = 0.0f;
    float max_value = 1.0f;
    std::string uniform_name;
};

struct NodePropertyVec4 {
    std::array<float, 4> value = {0.0f, 0.0f, 0.0f, 1.0f};
    std::string uniform_name;
};

struct NodeMath {
    MathOp op = MathOp::Add;
};

struct NodeTrig {
    TrigOp op = TrigOp::Sin;
};

struct NodeTextureSample {
    TextureOp op = TextureOp::Sample;
    std::string texture_uniform;
};

struct NodeSwizzle {
    char components[4] = {'x', 'y', 'z', 'w'};
    uint8_t count = 4;
};

struct NodeSplit {
    // Splits a vector into individual float channels
};

struct NodeCombine {
    // Combines float channels into a vector
    ShaderDataType output_type = ShaderDataType::Vec4;
};

struct NodeTime {
    bool use_sin = false;
    float speed = 1.0f;
};

struct NodeUV {
    UvOp op = UvOp::TilingOffset;
};

struct NodeNormal {
    bool world_space = true;
    bool tangent_space = false;
};

struct NodePosition {
    bool world_space = true;
};

struct NodeViewDirection {
    bool world_space = true;
};

struct NodeLighting {
    LightModel model = LightModel::Lambert;
};

struct NodeCustom {
    std::string glsl_code;
    std::string wgsl_code;
};

struct NodeBlend {
    BlendMode mode = BlendMode::Normal;
};

struct NodeCompare {
    CompareOp op = CompareOp::Greater;
};

struct NodeBranch {
    // If-else based on boolean input
};

struct NodeFresnel {
    float power = 5.0f;
};

struct NodeConstant {
    ShaderValue value;
    ShaderDataType output_type = ShaderDataType::Float;
};

struct NodeOutput {
    // Surface output: aggregates all final shader values
    // Inputs: Albedo, Normal, Metallic, Roughness, Emission, Alpha, AO
};

// The node variant - all node types without inheritance
using ShaderNodeData = std::variant<
    NodePropertyFloat,
    NodePropertyVec4,
    NodeMath,
    NodeTrig,
    NodeTextureSample,
    NodeSwizzle,
    NodeSplit,
    NodeCombine,
    NodeTime,
    NodeUV,
    NodeNormal,
    NodePosition,
    NodeViewDirection,
    NodeLighting,
    NodeCustom,
    NodeBlend,
    NodeCompare,
    NodeBranch,
    NodeFresnel,
    NodeConstant,
    NodeOutput
>;

// ============================================================
// ShaderNode: a single node in the composition graph
// ============================================================

struct ShaderNode {
    uint32_t id = 0;
    std::string name;
    ShaderNodeData data;
    std::vector<ShaderPort> inputs;
    std::vector<ShaderPort> outputs;

    // Visual editor position (for serialization)
    float pos_x = 0.0f;
    float pos_y = 0.0f;
};
