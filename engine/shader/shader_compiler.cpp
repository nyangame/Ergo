#include "shader_compiler.hpp"
#include <sstream>
#include <cassert>
#include <algorithm>

// ============================================================
// Variable naming helper
// ============================================================

static std::string var_name(uint32_t node_id, uint32_t port) {
    return "n" + std::to_string(node_id) + "_p" + std::to_string(port);
}

// ============================================================
// GLSL generation
// ============================================================

std::string ShaderCompiler::glsl_header() const {
    return "#version 450\n\n";
}

std::string ShaderCompiler::glsl_vertex_body() const {
    std::ostringstream ss;
    ss << glsl_header();
    ss << "layout(location = 0) in vec3 a_position;\n";
    ss << "layout(location = 1) in vec3 a_normal;\n";
    ss << "layout(location = 2) in vec2 a_uv;\n\n";
    ss << "layout(set = 0, binding = 0) uniform GlobalUBO {\n";
    ss << "    mat4 u_view;\n";
    ss << "    mat4 u_projection;\n";
    ss << "    float u_time;\n";
    ss << "    vec3 u_camera_pos;\n";
    ss << "};\n\n";
    ss << "layout(set = 1, binding = 0) uniform ObjectUBO {\n";
    ss << "    mat4 u_model;\n";
    ss << "    mat4 u_normal_matrix;\n";
    ss << "};\n\n";
    ss << "layout(location = 0) out vec3 v_world_pos;\n";
    ss << "layout(location = 1) out vec3 v_world_normal;\n";
    ss << "layout(location = 2) out vec2 v_uv;\n";
    ss << "layout(location = 3) out vec3 v_view_dir;\n\n";
    ss << "void main() {\n";
    ss << "    vec4 world_pos = u_model * vec4(a_position, 1.0);\n";
    ss << "    v_world_pos = world_pos.xyz;\n";
    ss << "    v_world_normal = normalize((u_normal_matrix * vec4(a_normal, 0.0)).xyz);\n";
    ss << "    v_uv = a_uv;\n";
    ss << "    v_view_dir = normalize(u_camera_pos - world_pos.xyz);\n";
    ss << "    gl_Position = u_projection * u_view * world_pos;\n";
    ss << "}\n";
    return ss.str();
}

std::string ShaderCompiler::glsl_fragment_uniforms(const ShaderGraph& graph) const {
    std::ostringstream ss;
    ss << "layout(set = 0, binding = 0) uniform GlobalUBO {\n";
    ss << "    mat4 u_view;\n";
    ss << "    mat4 u_projection;\n";
    ss << "    float u_time;\n";
    ss << "    vec3 u_camera_pos;\n";
    ss << "};\n\n";

    // Light data
    ss << "layout(set = 0, binding = 1) uniform LightUBO {\n";
    ss << "    vec3 u_light_dir;\n";
    ss << "    float _pad0;\n";
    ss << "    vec3 u_light_color;\n";
    ss << "    float u_light_intensity;\n";
    ss << "    vec3 u_ambient_color;\n";
    ss << "};\n\n";

    // Material properties from graph
    auto uniforms = graph.collect_uniforms();
    uint32_t binding = 0;
    bool has_material_uniforms = false;
    for (auto& [name, type] : uniforms) {
        if (type == ShaderDataType::Texture2D) continue;
        if (!has_material_uniforms) {
            ss << "layout(set = 2, binding = 0) uniform MaterialUBO {\n";
            has_material_uniforms = true;
        }
        ss << "    " << shader_data_type_name(type) << " u_" << name << ";\n";
    }
    if (has_material_uniforms) {
        ss << "};\n\n";
        binding = 1;
    }

    // Texture samplers
    for (auto& [name, type] : uniforms) {
        if (type == ShaderDataType::Texture2D) {
            ss << "layout(set = 2, binding = " << binding++ << ") uniform sampler2D u_"
               << name << ";\n";
        }
    }
    if (binding > 0) ss << "\n";

    return ss.str();
}

std::string ShaderCompiler::glsl_default_value(const ShaderPort& port) const {
    return std::visit([&](const auto& val) -> std::string {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, float>) {
            std::ostringstream ss;
            ss << val;
            return ss.str();
        } else if constexpr (std::is_same_v<T, std::array<float, 2>>) {
            std::ostringstream ss;
            ss << "vec2(" << val[0] << ", " << val[1] << ")";
            return ss.str();
        } else if constexpr (std::is_same_v<T, std::array<float, 3>>) {
            std::ostringstream ss;
            ss << "vec3(" << val[0] << ", " << val[1] << ", " << val[2] << ")";
            return ss.str();
        } else if constexpr (std::is_same_v<T, std::array<float, 4>>) {
            std::ostringstream ss;
            ss << "vec4(" << val[0] << ", " << val[1] << ", " << val[2] << ", " << val[3] << ")";
            return ss.str();
        } else if constexpr (std::is_same_v<T, bool>) {
            return val ? "true" : "false";
        }
        return "0.0";
    }, port.default_value.data);
}

std::string ShaderCompiler::glsl_type_cast(const std::string& expr,
                                           ShaderDataType from, ShaderDataType to) const {
    if (from == to) return expr;

    if (from == ShaderDataType::Float) {
        switch (to) {
        case ShaderDataType::Vec2: return "vec2(" + expr + ")";
        case ShaderDataType::Vec3: return "vec3(" + expr + ")";
        case ShaderDataType::Vec4: return "vec4(vec3(" + expr + "), 1.0)";
        default: break;
        }
    }
    if (from == ShaderDataType::Vec3 && to == ShaderDataType::Vec4)
        return "vec4(" + expr + ", 1.0)";
    if (from == ShaderDataType::Vec4 && to == ShaderDataType::Vec3)
        return expr + ".xyz";
    return expr;
}

std::string ShaderCompiler::glsl_node_expression(
    const ShaderGraph& graph, uint32_t node_id, uint32_t port_index,
    std::unordered_set<uint32_t>& emitted, std::ostringstream& body) const
{
    auto* node = graph.get_node(node_id);
    if (!node) return "0.0";

    std::string vn = var_name(node_id, port_index);

    // If already emitted, just reference the variable
    uint64_t key = (static_cast<uint64_t>(node_id) << 32) | port_index;
    if (emitted.count(static_cast<uint32_t>(key))) return vn;

    // Resolve all inputs recursively
    auto resolve_input = [&](uint32_t input_idx) -> std::string {
        auto* conn = graph.find_input_connection(node_id, input_idx);
        if (conn) {
            auto* src_node = graph.get_node(conn->source_node);
            if (!src_node) return "0.0";
            std::string src_expr = glsl_node_expression(
                graph, conn->source_node, conn->source_port, emitted, body);
            // Type cast if needed
            ShaderDataType src_type = src_node->outputs[conn->source_port].data_type;
            ShaderDataType dst_type = node->inputs[input_idx].data_type;
            return glsl_type_cast(src_expr, src_type, dst_type);
        }
        if (input_idx < node->inputs.size()) {
            return glsl_default_value(node->inputs[input_idx]);
        }
        return "0.0";
    };

    // Generate expression based on node type
    std::string expr;
    std::string type_str;

    std::visit([&](const auto& data) {
        using T = std::decay_t<decltype(data)>;

        if constexpr (std::is_same_v<T, NodePropertyFloat>) {
            expr = "u_" + data.uniform_name;
            type_str = "float";
        }
        else if constexpr (std::is_same_v<T, NodePropertyVec4>) {
            type_str = "vec4";
            std::string base = "u_" + data.uniform_name;
            switch (port_index) {
            case 0: expr = base; break;               // RGBA
            case 1: expr = base + ".rgb"; type_str = "vec3"; break;  // RGB
            case 2: expr = base + ".r"; type_str = "float"; break;   // R
            case 3: expr = base + ".g"; type_str = "float"; break;   // G
            case 4: expr = base + ".b"; type_str = "float"; break;   // B
            case 5: expr = base + ".a"; type_str = "float"; break;   // A
            default: expr = base; break;
            }
        }
        else if constexpr (std::is_same_v<T, NodeMath>) {
            type_str = shader_data_type_name(node->outputs[port_index].data_type);
            std::string a = resolve_input(0);
            switch (data.op) {
            case MathOp::Add: expr = "(" + a + " + " + resolve_input(1) + ")"; break;
            case MathOp::Subtract: expr = "(" + a + " - " + resolve_input(1) + ")"; break;
            case MathOp::Multiply: expr = "(" + a + " * " + resolve_input(1) + ")"; break;
            case MathOp::Divide: expr = "(" + a + " / max(" + resolve_input(1) + ", 0.0001))"; break;
            case MathOp::Power: expr = "pow(" + a + ", " + resolve_input(1) + ")"; break;
            case MathOp::SquareRoot: expr = "sqrt(max(" + a + ", 0.0))"; break;
            case MathOp::Abs: expr = "abs(" + a + ")"; break;
            case MathOp::Min: expr = "min(" + a + ", " + resolve_input(1) + ")"; break;
            case MathOp::Max: expr = "max(" + a + ", " + resolve_input(1) + ")"; break;
            case MathOp::Clamp:
                expr = "clamp(" + a + ", " + resolve_input(1) + ", " + resolve_input(2) + ")";
                break;
            case MathOp::Lerp:
                expr = "mix(" + a + ", " + resolve_input(1) + ", " + resolve_input(2) + ")";
                break;
            case MathOp::Dot: expr = "dot(" + a + ", " + resolve_input(1) + ")"; break;
            case MathOp::Cross: expr = "cross(" + a + ", " + resolve_input(1) + ")"; break;
            case MathOp::Normalize: expr = "normalize(" + a + ")"; break;
            case MathOp::Length: expr = "length(" + a + ")"; break;
            case MathOp::Negate: expr = "(-" + a + ")"; break;
            case MathOp::Fract: expr = "fract(" + a + ")"; break;
            case MathOp::Floor: expr = "floor(" + a + ")"; break;
            case MathOp::Ceil: expr = "ceil(" + a + ")"; break;
            case MathOp::Step: expr = "step(" + a + ", " + resolve_input(1) + ")"; break;
            case MathOp::SmoothStep:
                expr = "smoothstep(" + a + ", " + resolve_input(1) + ", " + resolve_input(2) + ")";
                break;
            }
        }
        else if constexpr (std::is_same_v<T, NodeTrig>) {
            type_str = "float";
            std::string a = resolve_input(0);
            switch (data.op) {
            case TrigOp::Sin: expr = "sin(" + a + ")"; break;
            case TrigOp::Cos: expr = "cos(" + a + ")"; break;
            case TrigOp::Tan: expr = "tan(" + a + ")"; break;
            case TrigOp::Asin: expr = "asin(clamp(" + a + ", -1.0, 1.0))"; break;
            case TrigOp::Acos: expr = "acos(clamp(" + a + ", -1.0, 1.0))"; break;
            case TrigOp::Atan: expr = "atan(" + a + ")"; break;
            case TrigOp::Atan2: expr = "atan(" + a + ", " + resolve_input(1) + ")"; break;
            }
        }
        else if constexpr (std::is_same_v<T, NodeTextureSample>) {
            std::string uv = resolve_input(0);
            std::string sampler_name = "u_" + data.texture_uniform;
            std::string base_expr;
            switch (data.op) {
            case TextureOp::Sample:
                base_expr = "texture(" + sampler_name + ", " + uv + ")";
                break;
            case TextureOp::SampleLod:
                base_expr = "textureLod(" + sampler_name + ", " + uv + ", " +
                            resolve_input(1) + ")";
                break;
            case TextureOp::SampleBias:
                base_expr = "texture(" + sampler_name + ", " + uv + ", " +
                            resolve_input(1) + ")";
                break;
            default:
                base_expr = "texture(" + sampler_name + ", " + uv + ")";
                break;
            }
            // Store base sample as a temp variable for multi-output
            std::string base_var = var_name(node_id, 99);
            body << "    vec4 " << base_var << " = " << base_expr << ";\n";

            switch (port_index) {
            case 0: expr = base_var; type_str = "vec4"; break;
            case 1: expr = base_var + ".rgb"; type_str = "vec3"; break;
            case 2: expr = base_var + ".r"; type_str = "float"; break;
            case 3: expr = base_var + ".g"; type_str = "float"; break;
            case 4: expr = base_var + ".b"; type_str = "float"; break;
            case 5: expr = base_var + ".a"; type_str = "float"; break;
            default: expr = base_var; type_str = "vec4"; break;
            }
        }
        else if constexpr (std::is_same_v<T, NodeSwizzle>) {
            type_str = shader_data_type_name(node->outputs[0].data_type);
            std::string in_expr = resolve_input(0);
            expr = in_expr + ".";
            for (uint8_t i = 0; i < data.count; ++i) {
                expr += data.components[i];
            }
        }
        else if constexpr (std::is_same_v<T, NodeSplit>) {
            type_str = "float";
            std::string in_expr = resolve_input(0);
            const char* channels[] = {"x", "y", "z", "w"};
            expr = in_expr + "." + channels[port_index];
        }
        else if constexpr (std::is_same_v<T, NodeCombine>) {
            type_str = shader_data_type_name(data.output_type);
            size_t n = node->inputs.size();
            expr = std::string(type_str) + "(";
            for (size_t i = 0; i < n; ++i) {
                if (i > 0) expr += ", ";
                expr += resolve_input(static_cast<uint32_t>(i));
            }
            expr += ")";
        }
        else if constexpr (std::is_same_v<T, NodeTime>) {
            type_str = "float";
            std::string speed_str = std::to_string(data.speed);
            switch (port_index) {
            case 0: expr = "(u_time * " + speed_str + ")"; break;
            case 1: expr = "sin(u_time * " + speed_str + ")"; break;
            case 2: expr = "cos(u_time * " + speed_str + ")"; break;
            default: expr = "u_time"; break;
            }
        }
        else if constexpr (std::is_same_v<T, NodeUV>) {
            type_str = "vec2";
            std::string uv = resolve_input(0);
            if (uv == "0.0" || uv == "vec2(0, 0)") uv = "v_uv";

            switch (data.op) {
            case UvOp::TilingOffset: {
                std::string tiling = resolve_input(1);
                std::string offset = resolve_input(2);
                expr = "(" + uv + " * " + tiling + " + " + offset + ")";
                break;
            }
            case UvOp::Rotate: {
                std::string center = resolve_input(1);
                std::string rot = resolve_input(2);
                std::string cos_v = "cos(" + rot + ")";
                std::string sin_v = "sin(" + rot + ")";
                body << "    vec2 " << vn << "_centered = " << uv << " - " << center << ";\n";
                expr = center + " + vec2(" + vn + "_centered.x * " + cos_v + " - " +
                       vn + "_centered.y * " + sin_v + ", " +
                       vn + "_centered.x * " + sin_v + " + " +
                       vn + "_centered.y * " + cos_v + ")";
                break;
            }
            case UvOp::Polar: {
                std::string center = resolve_input(1);
                body << "    vec2 " << vn << "_delta = " << uv << " - " << center << ";\n";
                expr = "vec2(atan(" + vn + "_delta.y, " + vn + "_delta.x) / 6.28318 + 0.5, "
                       "length(" + vn + "_delta))";
                break;
            }
            default:
                expr = uv;
                break;
            }
        }
        else if constexpr (std::is_same_v<T, NodeNormal>) {
            type_str = "vec3";
            expr = data.world_space ? "v_world_normal" : "a_normal";
        }
        else if constexpr (std::is_same_v<T, NodePosition>) {
            type_str = "vec3";
            expr = data.world_space ? "v_world_pos" : "a_position";
        }
        else if constexpr (std::is_same_v<T, NodeViewDirection>) {
            type_str = "vec3";
            expr = "v_view_dir";
        }
        else if constexpr (std::is_same_v<T, NodeLighting>) {
            std::string normal = resolve_input(0);
            std::string albedo = resolve_input(1);

            switch (data.model) {
            case LightModel::Lambert: {
                body << "    float " << vn << "_ndotl = max(dot(" << normal
                     << ", -u_light_dir), 0.0);\n";
                if (port_index == 0) {
                    expr = "(" + albedo + " * u_light_color * u_light_intensity * " +
                           vn + "_ndotl + " + albedo + " * u_ambient_color)";
                    type_str = "vec3";
                } else {
                    expr = vn + "_ndotl";
                    type_str = "float";
                }
                break;
            }
            case LightModel::BlinnPhong: {
                std::string spec = resolve_input(2);
                body << "    float " << vn << "_ndotl = max(dot(" << normal
                     << ", -u_light_dir), 0.0);\n";
                body << "    vec3 " << vn << "_half = normalize(-u_light_dir + v_view_dir);\n";
                body << "    float " << vn << "_spec = pow(max(dot(" << normal << ", "
                     << vn << "_half), 0.0), " << spec << " * 128.0);\n";
                if (port_index == 0) {
                    expr = "(" + albedo + " * u_light_color * u_light_intensity * " +
                           vn + "_ndotl + vec3(" + vn + "_spec) * u_light_color + " +
                           albedo + " * u_ambient_color)";
                    type_str = "vec3";
                } else {
                    expr = vn + "_ndotl";
                    type_str = "float";
                }
                break;
            }
            case LightModel::CookTorrance: {
                std::string spec = resolve_input(2);
                std::string metallic = resolve_input(3);
                std::string roughness = resolve_input(4);
                body << "    // Cook-Torrance BRDF\n";
                body << "    vec3 " << vn << "_N = normalize(" << normal << ");\n";
                body << "    vec3 " << vn << "_L = normalize(-u_light_dir);\n";
                body << "    vec3 " << vn << "_V = normalize(v_view_dir);\n";
                body << "    vec3 " << vn << "_H = normalize(" << vn << "_L + " << vn << "_V);\n";
                body << "    float " << vn << "_NdotL = max(dot(" << vn << "_N, " << vn << "_L), 0.001);\n";
                body << "    float " << vn << "_NdotV = max(dot(" << vn << "_N, " << vn << "_V), 0.001);\n";
                body << "    float " << vn << "_NdotH = max(dot(" << vn << "_N, " << vn << "_H), 0.001);\n";
                body << "    float " << vn << "_VdotH = max(dot(" << vn << "_V, " << vn << "_H), 0.001);\n";
                body << "    float " << vn << "_a = " << roughness << " * " << roughness << ";\n";
                body << "    float " << vn << "_a2 = " << vn << "_a * " << vn << "_a;\n";
                body << "    float " << vn << "_denom = " << vn << "_NdotH * " << vn << "_NdotH * ("
                     << vn << "_a2 - 1.0) + 1.0;\n";
                body << "    float " << vn << "_D = " << vn << "_a2 / (3.14159 * " << vn << "_denom * " << vn << "_denom);\n";
                body << "    float " << vn << "_k = (" << roughness << " + 1.0) * (" << roughness << " + 1.0) / 8.0;\n";
                body << "    float " << vn << "_G1L = " << vn << "_NdotL / (" << vn << "_NdotL * (1.0 - " << vn << "_k) + " << vn << "_k);\n";
                body << "    float " << vn << "_G1V = " << vn << "_NdotV / (" << vn << "_NdotV * (1.0 - " << vn << "_k) + " << vn << "_k);\n";
                body << "    float " << vn << "_G = " << vn << "_G1L * " << vn << "_G1V;\n";
                body << "    vec3 " << vn << "_F0 = mix(vec3(0.04), " << albedo << ", " << metallic << ");\n";
                body << "    vec3 " << vn << "_F = " << vn << "_F0 + (1.0 - " << vn << "_F0) * pow(1.0 - " << vn << "_VdotH, 5.0);\n";
                body << "    vec3 " << vn << "_specular = (" << vn << "_D * " << vn << "_G * " << vn << "_F) / "
                     << "(4.0 * " << vn << "_NdotL * " << vn << "_NdotV);\n";
                body << "    vec3 " << vn << "_kD = (1.0 - " << vn << "_F) * (1.0 - " << metallic << ");\n";
                if (port_index == 0) {
                    expr = "(" + vn + "_kD * " + albedo + " / 3.14159 + " + vn +
                           "_specular) * u_light_color * u_light_intensity * " + vn +
                           "_NdotL + " + albedo + " * u_ambient_color";
                    type_str = "vec3";
                } else {
                    expr = vn + "_NdotL";
                    type_str = "float";
                }
                break;
            }
            case LightModel::Toon: {
                std::string steps = resolve_input(2);
                body << "    float " << vn << "_ndotl = max(dot(" << normal
                     << ", -u_light_dir), 0.0);\n";
                body << "    float " << vn << "_toon = floor(" << vn << "_ndotl * "
                     << steps << ") / " << steps << ";\n";
                if (port_index == 0) {
                    expr = "(" + albedo + " * u_light_color * " + vn + "_toon + " +
                           albedo + " * u_ambient_color)";
                    type_str = "vec3";
                } else {
                    expr = vn + "_toon";
                    type_str = "float";
                }
                break;
            }
            case LightModel::Unlit:
                expr = albedo;
                type_str = "vec3";
                break;
            }
        }
        else if constexpr (std::is_same_v<T, NodeBlend>) {
            type_str = "vec3";
            std::string base = resolve_input(0);
            std::string blend = resolve_input(1);
            std::string opacity = resolve_input(2);

            switch (data.mode) {
            case BlendMode::Normal:
                expr = "mix(" + base + ", " + blend + ", " + opacity + ")";
                break;
            case BlendMode::Additive:
                expr = "mix(" + base + ", " + base + " + " + blend + ", " + opacity + ")";
                break;
            case BlendMode::Multiply:
                expr = "mix(" + base + ", " + base + " * " + blend + ", " + opacity + ")";
                break;
            case BlendMode::Screen:
                expr = "mix(" + base + ", 1.0 - (1.0 - " + base + ") * (1.0 - " + blend + "), " + opacity + ")";
                break;
            case BlendMode::Overlay:
                body << "    vec3 " << vn << "_overlay = vec3(\n";
                body << "        " << base << ".r < 0.5 ? 2.0 * " << base << ".r * " << blend << ".r : 1.0 - 2.0 * (1.0 - " << base << ".r) * (1.0 - " << blend << ".r),\n";
                body << "        " << base << ".g < 0.5 ? 2.0 * " << base << ".g * " << blend << ".g : 1.0 - 2.0 * (1.0 - " << base << ".g) * (1.0 - " << blend << ".g),\n";
                body << "        " << base << ".b < 0.5 ? 2.0 * " << base << ".b * " << blend << ".b : 1.0 - 2.0 * (1.0 - " << base << ".b) * (1.0 - " << blend << ".b));\n";
                expr = "mix(" + base + ", " + vn + "_overlay, " + opacity + ")";
                break;
            }
        }
        else if constexpr (std::is_same_v<T, NodeCompare>) {
            type_str = "bool";
            std::string a = resolve_input(0);
            std::string b = resolve_input(1);
            switch (data.op) {
            case CompareOp::Equal: expr = "(" + a + " == " + b + ")"; break;
            case CompareOp::NotEqual: expr = "(" + a + " != " + b + ")"; break;
            case CompareOp::Greater: expr = "(" + a + " > " + b + ")"; break;
            case CompareOp::Less: expr = "(" + a + " < " + b + ")"; break;
            case CompareOp::GreaterEqual: expr = "(" + a + " >= " + b + ")"; break;
            case CompareOp::LessEqual: expr = "(" + a + " <= " + b + ")"; break;
            }
        }
        else if constexpr (std::is_same_v<T, NodeBranch>) {
            type_str = shader_data_type_name(node->outputs[0].data_type);
            std::string cond = resolve_input(0);
            std::string t_val = resolve_input(1);
            std::string f_val = resolve_input(2);
            expr = "(" + cond + " ? " + t_val + " : " + f_val + ")";
        }
        else if constexpr (std::is_same_v<T, NodeFresnel>) {
            type_str = "float";
            std::string normal = resolve_input(0);
            std::string view = resolve_input(1);
            std::string power = resolve_input(2);
            expr = "pow(1.0 - max(dot(" + normal + ", " + view + "), 0.0), " + power + ")";
        }
        else if constexpr (std::is_same_v<T, NodeConstant>) {
            type_str = shader_data_type_name(data.output_type);
            expr = glsl_default_value(node->outputs[0]);
        }
        else if constexpr (std::is_same_v<T, NodeCustom>) {
            type_str = shader_data_type_name(node->outputs[port_index].data_type);
            // Custom nodes inline their code directly
            // Replace ${input_N} with resolved inputs
            std::string code = data.glsl_code;
            for (size_t i = 0; i < node->inputs.size(); ++i) {
                std::string placeholder = "${input_" + std::to_string(i) + "}";
                std::string replacement = resolve_input(static_cast<uint32_t>(i));
                size_t pos = 0;
                while ((pos = code.find(placeholder, pos)) != std::string::npos) {
                    code.replace(pos, placeholder.length(), replacement);
                    pos += replacement.length();
                }
            }
            expr = code;
        }
        else if constexpr (std::is_same_v<T, NodeOutput>) {
            // Output node: should not generate an expression itself
            expr = "";
            type_str = "";
        }
    }, node->data);

    if (!expr.empty() && !type_str.empty()) {
        body << "    " << type_str << " " << vn << " = " << expr << ";\n";
        emitted.insert(static_cast<uint32_t>(key));
    }

    return vn;
}

std::string ShaderCompiler::generate_vertex(const ShaderGraph& /*graph*/) const {
    if (language_ == ShaderLanguage::WGSL) {
        return wgsl_vertex_body();
    }
    return glsl_vertex_body();
}

std::string ShaderCompiler::generate_fragment(const ShaderGraph& graph) const {
    if (language_ == ShaderLanguage::WGSL) {
        // WGSL path
        std::ostringstream ss;
        ss << wgsl_header();
        ss << wgsl_fragment_uniforms(graph);

        ss << "@fragment\n";
        ss << "fn fs_main(\n";
        ss << "    @location(0) v_world_pos: vec3<f32>,\n";
        ss << "    @location(1) v_world_normal: vec3<f32>,\n";
        ss << "    @location(2) v_uv: vec2<f32>,\n";
        ss << "    @location(3) v_view_dir: vec3<f32>\n";
        ss << ") -> @location(0) vec4<f32> {\n";

        uint32_t output_node = graph.find_output_node();
        if (output_node == 0) {
            ss << "    return vec4<f32>(1.0, 0.0, 1.0, 1.0); // No output node\n";
            ss << "}\n";
            return ss.str();
        }

        std::unordered_set<uint32_t> emitted;
        std::ostringstream body;

        auto* out = graph.get_node(output_node);
        std::string albedo = "vec3<f32>(0.5, 0.5, 0.5)";
        std::string alpha = "1.0";

        for (uint32_t i = 0; i < out->inputs.size(); ++i) {
            auto* conn = graph.find_input_connection(output_node, i);
            if (conn) {
                std::string expr = wgsl_node_expression(
                    graph, conn->source_node, conn->source_port, emitted, body);
                auto* src_node = graph.get_node(conn->source_node);
                ShaderDataType src_type = src_node->outputs[conn->source_port].data_type;
                ShaderDataType dst_type = out->inputs[i].data_type;
                expr = wgsl_type_cast(expr, src_type, dst_type);

                if (out->inputs[i].name == "Albedo") albedo = expr;
                else if (out->inputs[i].name == "Alpha") alpha = expr;
            }
        }

        ss << body.str();
        ss << "    return vec4<f32>(" << albedo << ", " << alpha << ");\n";
        ss << "}\n";
        return ss.str();
    }

    // GLSL path
    std::ostringstream ss;
    ss << glsl_header();
    ss << "layout(location = 0) in vec3 v_world_pos;\n";
    ss << "layout(location = 1) in vec3 v_world_normal;\n";
    ss << "layout(location = 2) in vec2 v_uv;\n";
    ss << "layout(location = 3) in vec3 v_view_dir;\n\n";
    ss << glsl_fragment_uniforms(graph);
    ss << "layout(location = 0) out vec4 frag_color;\n\n";
    ss << "void main() {\n";

    uint32_t output_node = graph.find_output_node();
    if (output_node == 0) {
        ss << "    frag_color = vec4(1.0, 0.0, 1.0, 1.0); // No output node\n";
        ss << "}\n";
        return ss.str();
    }

    std::unordered_set<uint32_t> emitted;
    std::ostringstream body;

    auto* out = graph.get_node(output_node);
    // Resolve each input of the output node
    std::string albedo = "vec3(0.5)";
    std::string normal = "v_world_normal";
    std::string metallic = "0.0";
    std::string roughness = "0.5";
    std::string emission = "vec3(0.0)";
    std::string alpha = "1.0";
    std::string ao = "1.0";

    for (uint32_t i = 0; i < out->inputs.size(); ++i) {
        auto* conn = graph.find_input_connection(output_node, i);
        if (!conn) continue;

        std::string expr = glsl_node_expression(
            graph, conn->source_node, conn->source_port, emitted, body);

        // Type cast
        auto* src_node = graph.get_node(conn->source_node);
        ShaderDataType src_type = src_node->outputs[conn->source_port].data_type;
        ShaderDataType dst_type = out->inputs[i].data_type;
        expr = glsl_type_cast(expr, src_type, dst_type);

        if (out->inputs[i].name == "Albedo") albedo = expr;
        else if (out->inputs[i].name == "Normal") normal = expr;
        else if (out->inputs[i].name == "Metallic") metallic = expr;
        else if (out->inputs[i].name == "Roughness") roughness = expr;
        else if (out->inputs[i].name == "Emission") emission = expr;
        else if (out->inputs[i].name == "Alpha") alpha = expr;
        else if (out->inputs[i].name == "AO") ao = expr;
    }

    ss << body.str();
    ss << "\n    // Surface output\n";
    ss << "    vec3 final_color = " << albedo << ";\n";
    ss << "    final_color += " << emission << ";\n";
    ss << "    final_color *= " << ao << ";\n";
    ss << "    frag_color = vec4(final_color, " << alpha << ");\n";
    ss << "}\n";

    return ss.str();
}

// ============================================================
// WGSL generation
// ============================================================

std::string ShaderCompiler::wgsl_header() const {
    return "// Generated by Ergo ShaderCompiler (WGSL)\n\n";
}

std::string ShaderCompiler::wgsl_type_name(ShaderDataType dt) const {
    switch (dt) {
    case ShaderDataType::Float: return "f32";
    case ShaderDataType::Vec2: return "vec2<f32>";
    case ShaderDataType::Vec3: return "vec3<f32>";
    case ShaderDataType::Vec4: return "vec4<f32>";
    case ShaderDataType::Mat3: return "mat3x3<f32>";
    case ShaderDataType::Mat4: return "mat4x4<f32>";
    case ShaderDataType::Bool: return "bool";
    default: return "f32";
    }
}

std::string ShaderCompiler::wgsl_vertex_body() const {
    std::ostringstream ss;
    ss << wgsl_header();

    ss << "struct GlobalUBO {\n";
    ss << "    view: mat4x4<f32>,\n";
    ss << "    projection: mat4x4<f32>,\n";
    ss << "    time: f32,\n";
    ss << "    camera_pos: vec3<f32>,\n";
    ss << "};\n\n";

    ss << "struct ObjectUBO {\n";
    ss << "    model: mat4x4<f32>,\n";
    ss << "    normal_matrix: mat4x4<f32>,\n";
    ss << "};\n\n";

    ss << "@group(0) @binding(0) var<uniform> global: GlobalUBO;\n";
    ss << "@group(1) @binding(0) var<uniform> object: ObjectUBO;\n\n";

    ss << "struct VertexInput {\n";
    ss << "    @location(0) position: vec3<f32>,\n";
    ss << "    @location(1) normal: vec3<f32>,\n";
    ss << "    @location(2) uv: vec2<f32>,\n";
    ss << "};\n\n";

    ss << "struct VertexOutput {\n";
    ss << "    @builtin(position) clip_pos: vec4<f32>,\n";
    ss << "    @location(0) world_pos: vec3<f32>,\n";
    ss << "    @location(1) world_normal: vec3<f32>,\n";
    ss << "    @location(2) uv: vec2<f32>,\n";
    ss << "    @location(3) view_dir: vec3<f32>,\n";
    ss << "};\n\n";

    ss << "@vertex\n";
    ss << "fn vs_main(in: VertexInput) -> VertexOutput {\n";
    ss << "    var out: VertexOutput;\n";
    ss << "    let world_pos = object.model * vec4<f32>(in.position, 1.0);\n";
    ss << "    out.world_pos = world_pos.xyz;\n";
    ss << "    out.world_normal = normalize((object.normal_matrix * vec4<f32>(in.normal, 0.0)).xyz);\n";
    ss << "    out.uv = in.uv;\n";
    ss << "    out.view_dir = normalize(global.camera_pos - world_pos.xyz);\n";
    ss << "    out.clip_pos = global.projection * global.view * world_pos;\n";
    ss << "    return out;\n";
    ss << "}\n";
    return ss.str();
}

std::string ShaderCompiler::wgsl_fragment_uniforms(const ShaderGraph& graph) const {
    std::ostringstream ss;

    ss << "struct GlobalUBO {\n";
    ss << "    view: mat4x4<f32>,\n";
    ss << "    projection: mat4x4<f32>,\n";
    ss << "    time: f32,\n";
    ss << "    camera_pos: vec3<f32>,\n";
    ss << "};\n\n";

    ss << "struct LightUBO {\n";
    ss << "    light_dir: vec3<f32>,\n";
    ss << "    light_color: vec3<f32>,\n";
    ss << "    light_intensity: f32,\n";
    ss << "    ambient_color: vec3<f32>,\n";
    ss << "};\n\n";

    ss << "@group(0) @binding(0) var<uniform> global: GlobalUBO;\n";
    ss << "@group(0) @binding(1) var<uniform> light: LightUBO;\n\n";

    auto uniforms = graph.collect_uniforms();
    bool has_material = false;
    for (auto& [name, type] : uniforms) {
        if (type == ShaderDataType::Texture2D) continue;
        if (!has_material) {
            ss << "struct MaterialUBO {\n";
            has_material = true;
        }
        ss << "    " << name << ": " << wgsl_type_name(type) << ",\n";
    }
    if (has_material) {
        ss << "};\n";
        ss << "@group(2) @binding(0) var<uniform> material: MaterialUBO;\n\n";
    }

    uint32_t binding = has_material ? 1 : 0;
    for (auto& [name, type] : uniforms) {
        if (type == ShaderDataType::Texture2D) {
            ss << "@group(2) @binding(" << binding++ << ") var t_" << name << ": texture_2d<f32>;\n";
            ss << "@group(2) @binding(" << binding++ << ") var s_" << name << ": sampler;\n";
        }
    }
    ss << "\n";
    return ss.str();
}

std::string ShaderCompiler::wgsl_default_value(const ShaderPort& port) const {
    return std::visit([&](const auto& val) -> std::string {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, float>) {
            std::ostringstream ss;
            ss << val;
            return ss.str();
        } else if constexpr (std::is_same_v<T, std::array<float, 2>>) {
            std::ostringstream ss;
            ss << "vec2<f32>(" << val[0] << ", " << val[1] << ")";
            return ss.str();
        } else if constexpr (std::is_same_v<T, std::array<float, 3>>) {
            std::ostringstream ss;
            ss << "vec3<f32>(" << val[0] << ", " << val[1] << ", " << val[2] << ")";
            return ss.str();
        } else if constexpr (std::is_same_v<T, std::array<float, 4>>) {
            std::ostringstream ss;
            ss << "vec4<f32>(" << val[0] << ", " << val[1] << ", " << val[2] << ", " << val[3] << ")";
            return ss.str();
        } else if constexpr (std::is_same_v<T, bool>) {
            return val ? "true" : "false";
        }
        return "0.0";
    }, port.default_value.data);
}

std::string ShaderCompiler::wgsl_type_cast(const std::string& expr,
                                           ShaderDataType from, ShaderDataType to) const {
    if (from == to) return expr;

    if (from == ShaderDataType::Float) {
        switch (to) {
        case ShaderDataType::Vec2: return "vec2<f32>(" + expr + ")";
        case ShaderDataType::Vec3: return "vec3<f32>(" + expr + ")";
        case ShaderDataType::Vec4: return "vec4<f32>(vec3<f32>(" + expr + "), 1.0)";
        default: break;
        }
    }
    if (from == ShaderDataType::Vec3 && to == ShaderDataType::Vec4)
        return "vec4<f32>(" + expr + ", 1.0)";
    if (from == ShaderDataType::Vec4 && to == ShaderDataType::Vec3)
        return expr + ".xyz";
    return expr;
}

std::string ShaderCompiler::wgsl_node_expression(
    const ShaderGraph& graph, uint32_t node_id, uint32_t port_index,
    std::unordered_set<uint32_t>& emitted, std::ostringstream& body) const
{
    auto* node = graph.get_node(node_id);
    if (!node) return "0.0";

    std::string vn = var_name(node_id, port_index);
    uint64_t key = (static_cast<uint64_t>(node_id) << 32) | port_index;
    if (emitted.count(static_cast<uint32_t>(key))) return vn;

    auto resolve_input = [&](uint32_t input_idx) -> std::string {
        auto* conn = graph.find_input_connection(node_id, input_idx);
        if (conn) {
            auto* src_node = graph.get_node(conn->source_node);
            if (!src_node) return "0.0";
            std::string src_expr = wgsl_node_expression(
                graph, conn->source_node, conn->source_port, emitted, body);
            ShaderDataType src_type = src_node->outputs[conn->source_port].data_type;
            ShaderDataType dst_type = node->inputs[input_idx].data_type;
            return wgsl_type_cast(src_expr, src_type, dst_type);
        }
        if (input_idx < node->inputs.size()) {
            return wgsl_default_value(node->inputs[input_idx]);
        }
        return "0.0";
    };

    std::string expr;
    std::string type_str;

    std::visit([&](const auto& data) {
        using T = std::decay_t<decltype(data)>;

        if constexpr (std::is_same_v<T, NodePropertyFloat>) {
            expr = "material." + data.uniform_name;
            type_str = "f32";
        }
        else if constexpr (std::is_same_v<T, NodePropertyVec4>) {
            std::string base = "material." + data.uniform_name;
            switch (port_index) {
            case 0: expr = base; type_str = "vec4<f32>"; break;
            case 1: expr = base + ".xyz"; type_str = "vec3<f32>"; break;
            case 2: expr = base + ".x"; type_str = "f32"; break;
            case 3: expr = base + ".y"; type_str = "f32"; break;
            case 4: expr = base + ".z"; type_str = "f32"; break;
            case 5: expr = base + ".w"; type_str = "f32"; break;
            default: expr = base; type_str = "vec4<f32>"; break;
            }
        }
        else if constexpr (std::is_same_v<T, NodeMath>) {
            type_str = wgsl_type_name(node->outputs[port_index].data_type);
            std::string a = resolve_input(0);
            switch (data.op) {
            case MathOp::Add: expr = "(" + a + " + " + resolve_input(1) + ")"; break;
            case MathOp::Subtract: expr = "(" + a + " - " + resolve_input(1) + ")"; break;
            case MathOp::Multiply: expr = "(" + a + " * " + resolve_input(1) + ")"; break;
            case MathOp::Divide: expr = "(" + a + " / max(" + resolve_input(1) + ", 0.0001))"; break;
            case MathOp::Power: expr = "pow(" + a + ", " + resolve_input(1) + ")"; break;
            case MathOp::SquareRoot: expr = "sqrt(max(" + a + ", 0.0))"; break;
            case MathOp::Abs: expr = "abs(" + a + ")"; break;
            case MathOp::Min: expr = "min(" + a + ", " + resolve_input(1) + ")"; break;
            case MathOp::Max: expr = "max(" + a + ", " + resolve_input(1) + ")"; break;
            case MathOp::Clamp:
                expr = "clamp(" + a + ", " + resolve_input(1) + ", " + resolve_input(2) + ")";
                break;
            case MathOp::Lerp:
                expr = "mix(" + a + ", " + resolve_input(1) + ", " + resolve_input(2) + ")";
                break;
            case MathOp::Dot: expr = "dot(" + a + ", " + resolve_input(1) + ")"; break;
            case MathOp::Cross: expr = "cross(" + a + ", " + resolve_input(1) + ")"; break;
            case MathOp::Normalize: expr = "normalize(" + a + ")"; break;
            case MathOp::Length: expr = "length(" + a + ")"; break;
            case MathOp::Negate: expr = "(-" + a + ")"; break;
            case MathOp::Fract: expr = "fract(" + a + ")"; break;
            case MathOp::Floor: expr = "floor(" + a + ")"; break;
            case MathOp::Ceil: expr = "ceil(" + a + ")"; break;
            case MathOp::Step: expr = "step(" + a + ", " + resolve_input(1) + ")"; break;
            case MathOp::SmoothStep:
                expr = "smoothstep(" + a + ", " + resolve_input(1) + ", " + resolve_input(2) + ")";
                break;
            }
        }
        else if constexpr (std::is_same_v<T, NodeTrig>) {
            type_str = "f32";
            std::string a = resolve_input(0);
            switch (data.op) {
            case TrigOp::Sin: expr = "sin(" + a + ")"; break;
            case TrigOp::Cos: expr = "cos(" + a + ")"; break;
            case TrigOp::Tan: expr = "tan(" + a + ")"; break;
            case TrigOp::Asin: expr = "asin(clamp(" + a + ", -1.0, 1.0))"; break;
            case TrigOp::Acos: expr = "acos(clamp(" + a + ", -1.0, 1.0))"; break;
            case TrigOp::Atan: expr = "atan(" + a + ")"; break;
            case TrigOp::Atan2: expr = "atan2(" + a + ", " + resolve_input(1) + ")"; break;
            }
        }
        else if constexpr (std::is_same_v<T, NodeTextureSample>) {
            std::string uv = resolve_input(0);
            std::string tex_name = "t_" + data.texture_uniform;
            std::string samp_name = "s_" + data.texture_uniform;
            std::string base_var = var_name(node_id, 99);
            body << "    let " << base_var << " = textureSample(" << tex_name << ", "
                 << samp_name << ", " << uv << ");\n";

            switch (port_index) {
            case 0: expr = base_var; type_str = "vec4<f32>"; break;
            case 1: expr = base_var + ".xyz"; type_str = "vec3<f32>"; break;
            case 2: expr = base_var + ".x"; type_str = "f32"; break;
            case 3: expr = base_var + ".y"; type_str = "f32"; break;
            case 4: expr = base_var + ".z"; type_str = "f32"; break;
            case 5: expr = base_var + ".w"; type_str = "f32"; break;
            default: expr = base_var; type_str = "vec4<f32>"; break;
            }
        }
        else if constexpr (std::is_same_v<T, NodeTime>) {
            type_str = "f32";
            std::string speed_str = std::to_string(data.speed);
            switch (port_index) {
            case 0: expr = "(global.time * " + speed_str + ")"; break;
            case 1: expr = "sin(global.time * " + speed_str + ")"; break;
            case 2: expr = "cos(global.time * " + speed_str + ")"; break;
            default: expr = "global.time"; break;
            }
        }
        else if constexpr (std::is_same_v<T, NodeNormal>) {
            type_str = "vec3<f32>";
            expr = "v_world_normal";
        }
        else if constexpr (std::is_same_v<T, NodePosition>) {
            type_str = "vec3<f32>";
            expr = "v_world_pos";
        }
        else if constexpr (std::is_same_v<T, NodeViewDirection>) {
            type_str = "vec3<f32>";
            expr = "v_view_dir";
        }
        else if constexpr (std::is_same_v<T, NodeLighting>) {
            std::string normal = resolve_input(0);
            std::string albedo = resolve_input(1);
            body << "    let " << vn << "_ndotl = max(dot(" << normal
                 << ", -light.light_dir), 0.0);\n";
            if (port_index == 0) {
                expr = "(" + albedo + " * light.light_color * light.light_intensity * "
                     + vn + "_ndotl + " + albedo + " * light.ambient_color)";
                type_str = "vec3<f32>";
            } else {
                expr = vn + "_ndotl";
                type_str = "f32";
            }
        }
        else if constexpr (std::is_same_v<T, NodeConstant>) {
            type_str = wgsl_type_name(data.output_type);
            expr = wgsl_default_value(node->outputs[0]);
        }
        else if constexpr (std::is_same_v<T, NodeCustom>) {
            type_str = wgsl_type_name(node->outputs[port_index].data_type);
            std::string code = data.wgsl_code;
            for (size_t i = 0; i < node->inputs.size(); ++i) {
                std::string placeholder = "${input_" + std::to_string(i) + "}";
                std::string replacement = resolve_input(static_cast<uint32_t>(i));
                size_t pos = 0;
                while ((pos = code.find(placeholder, pos)) != std::string::npos) {
                    code.replace(pos, placeholder.length(), replacement);
                    pos += replacement.length();
                }
            }
            expr = code;
        }
        else if constexpr (std::is_same_v<T, NodeFresnel>) {
            type_str = "f32";
            std::string normal = resolve_input(0);
            std::string view = resolve_input(1);
            std::string power = resolve_input(2);
            expr = "pow(1.0 - max(dot(" + normal + ", " + view + "), 0.0), " + power + ")";
        }
        else {
            // Fallback for node types not fully ported to WGSL
            type_str = wgsl_type_name(node->outputs.empty() ? ShaderDataType::Float
                                      : node->outputs[port_index].data_type);
            expr = "0.0";
        }
    }, node->data);

    if (!expr.empty() && !type_str.empty()) {
        body << "    let " << vn << ": " << type_str << " = " << expr << ";\n";
        emitted.insert(static_cast<uint32_t>(key));
    }

    return vn;
}
