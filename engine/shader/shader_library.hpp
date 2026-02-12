#pragma once
#include "shader_node.hpp"

// ============================================================
// ShaderNodeLibrary: factory for creating built-in shader nodes
// ============================================================

struct ShaderNodeLibrary {
    static uint32_t next_port_id;

    static uint32_t alloc_port_id() { return next_port_id++; }

    // --- Property nodes ---

    static ShaderNode create_float_property(const std::string& name, float value,
                                            float min_val = 0.0f, float max_val = 1.0f) {
        ShaderNode node;
        node.name = name;
        node.data = NodePropertyFloat{value, min_val, max_val, name};
        node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Float,
                                ShaderValue::from_float(value)});
        return node;
    }

    static ShaderNode create_color_property(const std::string& name,
                                            float r = 1.0f, float g = 1.0f,
                                            float b = 1.0f, float a = 1.0f) {
        ShaderNode node;
        node.name = name;
        node.data = NodePropertyVec4{{r, g, b, a}, name};
        node.outputs.push_back({alloc_port_id(), "RGBA", ShaderDataType::Vec4,
                                ShaderValue::from_vec4(r, g, b, a)});
        node.outputs.push_back({alloc_port_id(), "RGB", ShaderDataType::Vec3,
                                ShaderValue::from_vec3(r, g, b)});
        node.outputs.push_back({alloc_port_id(), "R", ShaderDataType::Float,
                                ShaderValue::from_float(r)});
        node.outputs.push_back({alloc_port_id(), "G", ShaderDataType::Float,
                                ShaderValue::from_float(g)});
        node.outputs.push_back({alloc_port_id(), "B", ShaderDataType::Float,
                                ShaderValue::from_float(b)});
        node.outputs.push_back({alloc_port_id(), "A", ShaderDataType::Float,
                                ShaderValue::from_float(a)});
        return node;
    }

    // --- Math nodes ---

    static ShaderNode create_math(MathOp op) {
        ShaderNode node;
        const char* names[] = {
            "Add", "Subtract", "Multiply", "Divide",
            "Power", "SquareRoot", "Abs",
            "Min", "Max", "Clamp", "Lerp",
            "Dot", "Cross", "Normalize", "Length",
            "Negate", "Fract", "Floor", "Ceil",
            "Step", "SmoothStep"
        };
        node.name = names[static_cast<int>(op)];
        node.data = NodeMath{op};

        // Determine input/output configuration based on operation
        switch (op) {
        case MathOp::Add: case MathOp::Subtract:
        case MathOp::Multiply: case MathOp::Divide:
        case MathOp::Power: case MathOp::Min: case MathOp::Max:
            node.inputs.push_back({alloc_port_id(), "A", ShaderDataType::Float});
            node.inputs.push_back({alloc_port_id(), "B", ShaderDataType::Float});
            node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Float});
            break;
        case MathOp::Clamp:
            node.inputs.push_back({alloc_port_id(), "Value", ShaderDataType::Float});
            node.inputs.push_back({alloc_port_id(), "Min", ShaderDataType::Float,
                                   ShaderValue::from_float(0.0f)});
            node.inputs.push_back({alloc_port_id(), "Max", ShaderDataType::Float,
                                   ShaderValue::from_float(1.0f)});
            node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Float});
            break;
        case MathOp::Lerp:
            node.inputs.push_back({alloc_port_id(), "A", ShaderDataType::Float});
            node.inputs.push_back({alloc_port_id(), "B", ShaderDataType::Float});
            node.inputs.push_back({alloc_port_id(), "T", ShaderDataType::Float,
                                   ShaderValue::from_float(0.5f)});
            node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Float});
            break;
        case MathOp::Dot:
            node.inputs.push_back({alloc_port_id(), "A", ShaderDataType::Vec3});
            node.inputs.push_back({alloc_port_id(), "B", ShaderDataType::Vec3});
            node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Float});
            break;
        case MathOp::Cross:
            node.inputs.push_back({alloc_port_id(), "A", ShaderDataType::Vec3});
            node.inputs.push_back({alloc_port_id(), "B", ShaderDataType::Vec3});
            node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Vec3});
            break;
        case MathOp::Normalize:
            node.inputs.push_back({alloc_port_id(), "In", ShaderDataType::Vec3});
            node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Vec3});
            break;
        case MathOp::Length:
            node.inputs.push_back({alloc_port_id(), "In", ShaderDataType::Vec3});
            node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Float});
            break;
        case MathOp::SmoothStep:
            node.inputs.push_back({alloc_port_id(), "Edge0", ShaderDataType::Float,
                                   ShaderValue::from_float(0.0f)});
            node.inputs.push_back({alloc_port_id(), "Edge1", ShaderDataType::Float,
                                   ShaderValue::from_float(1.0f)});
            node.inputs.push_back({alloc_port_id(), "X", ShaderDataType::Float});
            node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Float});
            break;
        case MathOp::Step:
            node.inputs.push_back({alloc_port_id(), "Edge", ShaderDataType::Float});
            node.inputs.push_back({alloc_port_id(), "X", ShaderDataType::Float});
            node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Float});
            break;
        default:
            // Unary operations
            node.inputs.push_back({alloc_port_id(), "In", ShaderDataType::Float});
            node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Float});
            break;
        }
        return node;
    }

    // --- Trigonometric nodes ---

    static ShaderNode create_trig(TrigOp op) {
        ShaderNode node;
        const char* names[] = {"Sin", "Cos", "Tan", "Asin", "Acos", "Atan", "Atan2"};
        node.name = names[static_cast<int>(op)];
        node.data = NodeTrig{op};

        if (op == TrigOp::Atan2) {
            node.inputs.push_back({alloc_port_id(), "Y", ShaderDataType::Float});
            node.inputs.push_back({alloc_port_id(), "X", ShaderDataType::Float});
        } else {
            node.inputs.push_back({alloc_port_id(), "In", ShaderDataType::Float});
        }
        node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Float});
        return node;
    }

    // --- Texture sample ---

    static ShaderNode create_texture_sample(const std::string& texture_name,
                                            TextureOp op = TextureOp::Sample) {
        ShaderNode node;
        node.name = "TextureSample";
        node.data = NodeTextureSample{op, texture_name};

        node.inputs.push_back({alloc_port_id(), "UV", ShaderDataType::Vec2});
        if (op == TextureOp::SampleLod || op == TextureOp::SampleBias) {
            node.inputs.push_back({alloc_port_id(), "Lod", ShaderDataType::Float,
                                   ShaderValue::from_float(0.0f)});
        }
        node.outputs.push_back({alloc_port_id(), "RGBA", ShaderDataType::Vec4});
        node.outputs.push_back({alloc_port_id(), "RGB", ShaderDataType::Vec3});
        node.outputs.push_back({alloc_port_id(), "R", ShaderDataType::Float});
        node.outputs.push_back({alloc_port_id(), "G", ShaderDataType::Float});
        node.outputs.push_back({alloc_port_id(), "B", ShaderDataType::Float});
        node.outputs.push_back({alloc_port_id(), "A", ShaderDataType::Float});
        return node;
    }

    // --- Swizzle ---

    static ShaderNode create_swizzle(const char* mask, uint8_t count) {
        ShaderNode node;
        node.name = "Swizzle";
        NodeSwizzle sw;
        for (uint8_t i = 0; i < count && i < 4; ++i) sw.components[i] = mask[i];
        sw.count = count;
        node.data = sw;

        node.inputs.push_back({alloc_port_id(), "In", ShaderDataType::Vec4});
        switch (count) {
        case 1: node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Float}); break;
        case 2: node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Vec2}); break;
        case 3: node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Vec3}); break;
        default: node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Vec4}); break;
        }
        return node;
    }

    // --- Split / Combine ---

    static ShaderNode create_split() {
        ShaderNode node;
        node.name = "Split";
        node.data = NodeSplit{};
        node.inputs.push_back({alloc_port_id(), "In", ShaderDataType::Vec4});
        node.outputs.push_back({alloc_port_id(), "X", ShaderDataType::Float});
        node.outputs.push_back({alloc_port_id(), "Y", ShaderDataType::Float});
        node.outputs.push_back({alloc_port_id(), "Z", ShaderDataType::Float});
        node.outputs.push_back({alloc_port_id(), "W", ShaderDataType::Float});
        return node;
    }

    static ShaderNode create_combine(ShaderDataType output_type = ShaderDataType::Vec4) {
        ShaderNode node;
        node.name = "Combine";
        node.data = NodeCombine{output_type};
        node.inputs.push_back({alloc_port_id(), "X", ShaderDataType::Float,
                               ShaderValue::from_float(0.0f)});
        node.inputs.push_back({alloc_port_id(), "Y", ShaderDataType::Float,
                               ShaderValue::from_float(0.0f)});
        if (output_type == ShaderDataType::Vec3 || output_type == ShaderDataType::Vec4) {
            node.inputs.push_back({alloc_port_id(), "Z", ShaderDataType::Float,
                                   ShaderValue::from_float(0.0f)});
        }
        if (output_type == ShaderDataType::Vec4) {
            node.inputs.push_back({alloc_port_id(), "W", ShaderDataType::Float,
                                   ShaderValue::from_float(1.0f)});
        }
        node.outputs.push_back({alloc_port_id(), "Out", output_type});
        return node;
    }

    // --- Time ---

    static ShaderNode create_time(float speed = 1.0f, bool use_sin = false) {
        ShaderNode node;
        node.name = "Time";
        node.data = NodeTime{use_sin, speed};
        node.outputs.push_back({alloc_port_id(), "Time", ShaderDataType::Float});
        node.outputs.push_back({alloc_port_id(), "SinTime", ShaderDataType::Float});
        node.outputs.push_back({alloc_port_id(), "CosTime", ShaderDataType::Float});
        return node;
    }

    // --- UV ---

    static ShaderNode create_uv(UvOp op = UvOp::TilingOffset) {
        ShaderNode node;
        node.name = "UV";
        node.data = NodeUV{op};

        switch (op) {
        case UvOp::TilingOffset:
            node.inputs.push_back({alloc_port_id(), "UV", ShaderDataType::Vec2});
            node.inputs.push_back({alloc_port_id(), "Tiling", ShaderDataType::Vec2,
                                   ShaderValue::from_vec2(1.0f, 1.0f)});
            node.inputs.push_back({alloc_port_id(), "Offset", ShaderDataType::Vec2,
                                   ShaderValue::from_vec2(0.0f, 0.0f)});
            node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Vec2});
            break;
        case UvOp::Rotate:
            node.inputs.push_back({alloc_port_id(), "UV", ShaderDataType::Vec2});
            node.inputs.push_back({alloc_port_id(), "Center", ShaderDataType::Vec2,
                                   ShaderValue::from_vec2(0.5f, 0.5f)});
            node.inputs.push_back({alloc_port_id(), "Rotation", ShaderDataType::Float,
                                   ShaderValue::from_float(0.0f)});
            node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Vec2});
            break;
        case UvOp::Polar:
            node.inputs.push_back({alloc_port_id(), "UV", ShaderDataType::Vec2});
            node.inputs.push_back({alloc_port_id(), "Center", ShaderDataType::Vec2,
                                   ShaderValue::from_vec2(0.5f, 0.5f)});
            node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Vec2});
            break;
        default:
            node.inputs.push_back({alloc_port_id(), "UV", ShaderDataType::Vec2});
            node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Vec2});
            break;
        }
        return node;
    }

    // --- Geometry inputs ---

    static ShaderNode create_normal(bool world_space = true) {
        ShaderNode node;
        node.name = world_space ? "WorldNormal" : "ObjectNormal";
        node.data = NodeNormal{world_space, false};
        node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Vec3});
        return node;
    }

    static ShaderNode create_position(bool world_space = true) {
        ShaderNode node;
        node.name = world_space ? "WorldPosition" : "ObjectPosition";
        node.data = NodePosition{world_space};
        node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Vec3});
        return node;
    }

    static ShaderNode create_view_direction(bool world_space = true) {
        ShaderNode node;
        node.name = "ViewDirection";
        node.data = NodeViewDirection{world_space};
        node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Vec3});
        return node;
    }

    // --- Lighting ---

    static ShaderNode create_lighting(LightModel model = LightModel::Lambert) {
        ShaderNode node;
        const char* names[] = {"Lambert", "BlinnPhong", "CookTorrance", "Toon", "Unlit"};
        node.name = names[static_cast<int>(model)];
        node.data = NodeLighting{model};

        node.inputs.push_back({alloc_port_id(), "Normal", ShaderDataType::Vec3});
        node.inputs.push_back({alloc_port_id(), "Albedo", ShaderDataType::Vec3,
                               ShaderValue::from_vec3(1.0f, 1.0f, 1.0f)});

        if (model == LightModel::BlinnPhong || model == LightModel::CookTorrance) {
            node.inputs.push_back({alloc_port_id(), "Specular", ShaderDataType::Float,
                                   ShaderValue::from_float(0.5f)});
        }
        if (model == LightModel::CookTorrance) {
            node.inputs.push_back({alloc_port_id(), "Metallic", ShaderDataType::Float,
                                   ShaderValue::from_float(0.0f)});
            node.inputs.push_back({alloc_port_id(), "Roughness", ShaderDataType::Float,
                                   ShaderValue::from_float(0.5f)});
        }
        if (model == LightModel::Toon) {
            node.inputs.push_back({alloc_port_id(), "Steps", ShaderDataType::Float,
                                   ShaderValue::from_float(3.0f)});
        }
        node.outputs.push_back({alloc_port_id(), "Color", ShaderDataType::Vec3});
        node.outputs.push_back({alloc_port_id(), "Diffuse", ShaderDataType::Float});
        return node;
    }

    // --- Blend ---

    static ShaderNode create_blend(BlendMode mode = BlendMode::Normal) {
        ShaderNode node;
        node.name = "Blend";
        node.data = NodeBlend{mode};
        node.inputs.push_back({alloc_port_id(), "Base", ShaderDataType::Vec3});
        node.inputs.push_back({alloc_port_id(), "Blend", ShaderDataType::Vec3});
        node.inputs.push_back({alloc_port_id(), "Opacity", ShaderDataType::Float,
                               ShaderValue::from_float(1.0f)});
        node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Vec3});
        return node;
    }

    // --- Compare / Branch ---

    static ShaderNode create_compare(CompareOp op = CompareOp::Greater) {
        ShaderNode node;
        node.name = "Compare";
        node.data = NodeCompare{op};
        node.inputs.push_back({alloc_port_id(), "A", ShaderDataType::Float});
        node.inputs.push_back({alloc_port_id(), "B", ShaderDataType::Float});
        node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Bool});
        return node;
    }

    static ShaderNode create_branch() {
        ShaderNode node;
        node.name = "Branch";
        node.data = NodeBranch{};
        node.inputs.push_back({alloc_port_id(), "Condition", ShaderDataType::Bool});
        node.inputs.push_back({alloc_port_id(), "True", ShaderDataType::Float});
        node.inputs.push_back({alloc_port_id(), "False", ShaderDataType::Float});
        node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Float});
        return node;
    }

    // --- Fresnel ---

    static ShaderNode create_fresnel(float power = 5.0f) {
        ShaderNode node;
        node.name = "Fresnel";
        node.data = NodeFresnel{power};
        node.inputs.push_back({alloc_port_id(), "Normal", ShaderDataType::Vec3});
        node.inputs.push_back({alloc_port_id(), "ViewDir", ShaderDataType::Vec3});
        node.inputs.push_back({alloc_port_id(), "Power", ShaderDataType::Float,
                               ShaderValue::from_float(power)});
        node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Float});
        return node;
    }

    // --- Constant ---

    static ShaderNode create_constant(float value) {
        ShaderNode node;
        node.name = "Constant";
        node.data = NodeConstant{ShaderValue::from_float(value), ShaderDataType::Float};
        node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Float,
                                ShaderValue::from_float(value)});
        return node;
    }

    static ShaderNode create_constant_vec3(float x, float y, float z) {
        ShaderNode node;
        node.name = "ConstantVec3";
        node.data = NodeConstant{ShaderValue::from_vec3(x, y, z), ShaderDataType::Vec3};
        node.outputs.push_back({alloc_port_id(), "Out", ShaderDataType::Vec3,
                                ShaderValue::from_vec3(x, y, z)});
        return node;
    }

    // --- Custom GLSL/WGSL ---

    static ShaderNode create_custom(const std::string& glsl, const std::string& wgsl,
                                    const std::vector<ShaderPort>& ins,
                                    const std::vector<ShaderPort>& outs) {
        ShaderNode node;
        node.name = "Custom";
        node.data = NodeCustom{glsl, wgsl};
        node.inputs = ins;
        node.outputs = outs;
        return node;
    }

    // --- Surface Output ---

    static ShaderNode create_output() {
        ShaderNode node;
        node.name = "SurfaceOutput";
        node.data = NodeOutput{};
        node.inputs.push_back({alloc_port_id(), "Albedo", ShaderDataType::Vec3,
                               ShaderValue::from_vec3(0.5f, 0.5f, 0.5f)});
        node.inputs.push_back({alloc_port_id(), "Normal", ShaderDataType::Vec3,
                               ShaderValue::from_vec3(0.0f, 0.0f, 1.0f)});
        node.inputs.push_back({alloc_port_id(), "Metallic", ShaderDataType::Float,
                               ShaderValue::from_float(0.0f)});
        node.inputs.push_back({alloc_port_id(), "Roughness", ShaderDataType::Float,
                               ShaderValue::from_float(0.5f)});
        node.inputs.push_back({alloc_port_id(), "Emission", ShaderDataType::Vec3,
                               ShaderValue::from_vec3(0.0f, 0.0f, 0.0f)});
        node.inputs.push_back({alloc_port_id(), "Alpha", ShaderDataType::Float,
                               ShaderValue::from_float(1.0f)});
        node.inputs.push_back({alloc_port_id(), "AO", ShaderDataType::Float,
                               ShaderValue::from_float(1.0f)});
        return node;
    }
};
