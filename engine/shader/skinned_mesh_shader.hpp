#pragma once
#include "shader_compiler.hpp"
#include "../render/skinned_mesh.hpp"
#include <string>
#include <sstream>

// ============================================================
// SkinnedMeshShader: generates vertex/fragment shaders for
// GPU-based skeletal animation (linear blend skinning).
//
// Vertex shader:
//   Reads per-vertex bone indices (ivec4) and bone weights (vec4).
//   Applies: skinned_pos = sum_i( weight_i * bone_matrix_i * pos )
//   Transforms normals through the same bone matrices.
//
// Fragment shader:
//   Standard lit surface using the interpolated world-space
//   position/normal. Supports a base color uniform + diffuse map.
// ============================================================

class SkinnedMeshShader {
    ShaderLanguage language_ = ShaderLanguage::GLSL_450;

public:
    explicit SkinnedMeshShader(ShaderLanguage lang = ShaderLanguage::GLSL_450)
        : language_(lang) {}

    ShaderLanguage language() const { return language_; }

    // Generate vertex shader source
    std::string generate_vertex() const {
        if (language_ == ShaderLanguage::WGSL) return generate_vertex_wgsl();
        return generate_vertex_glsl();
    }

    // Generate fragment shader source
    std::string generate_fragment() const {
        if (language_ == ShaderLanguage::WGSL) return generate_fragment_wgsl();
        return generate_fragment_glsl();
    }

    // Generate both as a pair (vertex, fragment)
    std::pair<std::string, std::string> generate() const {
        return {generate_vertex(), generate_fragment()};
    }

private:
    // --------------------------------------------------------
    // GLSL 4.50 (Vulkan)
    // --------------------------------------------------------

    std::string generate_vertex_glsl() const {
        std::ostringstream s;
        s << "#version 450\n\n";

        // Vertex attributes
        s << "layout(location = 0) in vec3 a_position;\n";
        s << "layout(location = 1) in vec3 a_normal;\n";
        s << "layout(location = 2) in vec2 a_uv;\n";
        s << "layout(location = 3) in ivec4 a_bone_indices;\n";
        s << "layout(location = 4) in vec4 a_bone_weights;\n\n";

        // Uniforms
        s << "layout(set = 0, binding = 0) uniform SceneUBO {\n";
        s << "    mat4 u_view;\n";
        s << "    mat4 u_projection;\n";
        s << "};\n\n";

        s << "layout(set = 1, binding = 0) uniform ModelUBO {\n";
        s << "    mat4 u_model;\n";
        s << "};\n\n";

        s << "layout(set = 2, binding = 0) uniform BoneUBO {\n";
        s << "    mat4 u_bones[" << kMaxBones << "];\n";
        s << "};\n\n";

        // Varyings
        s << "layout(location = 0) out vec3 v_world_position;\n";
        s << "layout(location = 1) out vec3 v_world_normal;\n";
        s << "layout(location = 2) out vec2 v_uv;\n\n";

        // Main
        s << "void main() {\n";
        s << "    // Linear blend skinning\n";
        s << "    mat4 skin_matrix =\n";
        s << "        a_bone_weights.x * u_bones[a_bone_indices.x] +\n";
        s << "        a_bone_weights.y * u_bones[a_bone_indices.y] +\n";
        s << "        a_bone_weights.z * u_bones[a_bone_indices.z] +\n";
        s << "        a_bone_weights.w * u_bones[a_bone_indices.w];\n\n";
        s << "    vec4 skinned_pos = skin_matrix * vec4(a_position, 1.0);\n";
        s << "    vec4 world_pos   = u_model * skinned_pos;\n\n";
        s << "    // Transform normal (use upper-left 3x3 of skin * model)\n";
        s << "    mat3 normal_matrix = mat3(u_model) * mat3(skin_matrix);\n";
        s << "    vec3 world_normal  = normalize(normal_matrix * a_normal);\n\n";
        s << "    v_world_position = world_pos.xyz;\n";
        s << "    v_world_normal   = world_normal;\n";
        s << "    v_uv             = a_uv;\n\n";
        s << "    gl_Position = u_projection * u_view * world_pos;\n";
        s << "}\n";
        return s.str();
    }

    std::string generate_fragment_glsl() const {
        std::ostringstream s;
        s << "#version 450\n\n";

        // Varyings
        s << "layout(location = 0) in vec3 v_world_position;\n";
        s << "layout(location = 1) in vec3 v_world_normal;\n";
        s << "layout(location = 2) in vec2 v_uv;\n\n";

        // Output
        s << "layout(location = 0) out vec4 frag_color;\n\n";

        // Material uniforms
        s << "layout(set = 3, binding = 0) uniform MaterialUBO {\n";
        s << "    vec4 u_base_color;\n";
        s << "    float u_metallic;\n";
        s << "    float u_roughness;\n";
        s << "};\n\n";

        s << "layout(set = 3, binding = 1) uniform sampler2D u_diffuse_map;\n\n";

        // Simple directional light
        s << "const vec3 LIGHT_DIR = normalize(vec3(0.5, 1.0, 0.3));\n";
        s << "const vec3 LIGHT_COLOR = vec3(1.0);\n";
        s << "const vec3 AMBIENT = vec3(0.15);\n\n";

        s << "void main() {\n";
        s << "    vec4 tex_color = texture(u_diffuse_map, v_uv);\n";
        s << "    vec3 albedo    = u_base_color.rgb * tex_color.rgb;\n";
        s << "    float alpha    = u_base_color.a * tex_color.a;\n\n";
        s << "    // Lambert diffuse\n";
        s << "    vec3 N = normalize(v_world_normal);\n";
        s << "    float NdotL = max(dot(N, LIGHT_DIR), 0.0);\n";
        s << "    vec3 diffuse = albedo * LIGHT_COLOR * NdotL;\n\n";
        s << "    vec3 color = AMBIENT * albedo + diffuse;\n";
        s << "    frag_color = vec4(color, alpha);\n";
        s << "}\n";
        return s.str();
    }

    // --------------------------------------------------------
    // WGSL (WebGPU)
    // --------------------------------------------------------

    std::string generate_vertex_wgsl() const {
        std::ostringstream s;

        // Structures
        s << "struct SceneUBO {\n";
        s << "    view : mat4x4<f32>,\n";
        s << "    projection : mat4x4<f32>,\n";
        s << "};\n\n";

        s << "struct ModelUBO {\n";
        s << "    model : mat4x4<f32>,\n";
        s << "};\n\n";

        s << "struct BoneUBO {\n";
        s << "    bones : array<mat4x4<f32>, " << kMaxBones << ">,\n";
        s << "};\n\n";

        // Bindings
        s << "@group(0) @binding(0) var<uniform> scene : SceneUBO;\n";
        s << "@group(1) @binding(0) var<uniform> model : ModelUBO;\n";
        s << "@group(2) @binding(0) var<uniform> bone : BoneUBO;\n\n";

        // Input / output structs
        s << "struct VertexInput {\n";
        s << "    @location(0) position : vec3<f32>,\n";
        s << "    @location(1) normal : vec3<f32>,\n";
        s << "    @location(2) uv : vec2<f32>,\n";
        s << "    @location(3) bone_indices : vec4<i32>,\n";
        s << "    @location(4) bone_weights : vec4<f32>,\n";
        s << "};\n\n";

        s << "struct VertexOutput {\n";
        s << "    @builtin(position) clip_position : vec4<f32>,\n";
        s << "    @location(0) world_position : vec3<f32>,\n";
        s << "    @location(1) world_normal : vec3<f32>,\n";
        s << "    @location(2) uv : vec2<f32>,\n";
        s << "};\n\n";

        // Vertex main
        s << "@vertex\n";
        s << "fn vs_main(in : VertexInput) -> VertexOutput {\n";
        s << "    // Linear blend skinning\n";
        s << "    let skin_matrix =\n";
        s << "        in.bone_weights.x * bone.bones[in.bone_indices.x] +\n";
        s << "        in.bone_weights.y * bone.bones[in.bone_indices.y] +\n";
        s << "        in.bone_weights.z * bone.bones[in.bone_indices.z] +\n";
        s << "        in.bone_weights.w * bone.bones[in.bone_indices.w];\n\n";
        s << "    let skinned_pos = skin_matrix * vec4<f32>(in.position, 1.0);\n";
        s << "    let world_pos   = model.model * skinned_pos;\n\n";
        s << "    // Normal transform\n";
        s << "    let normal_matrix = mat3x3<f32>(\n";
        s << "        (model.model * skin_matrix)[0].xyz,\n";
        s << "        (model.model * skin_matrix)[1].xyz,\n";
        s << "        (model.model * skin_matrix)[2].xyz\n";
        s << "    );\n";
        s << "    let world_normal = normalize(normal_matrix * in.normal);\n\n";
        s << "    var out : VertexOutput;\n";
        s << "    out.clip_position = scene.projection * scene.view * world_pos;\n";
        s << "    out.world_position = world_pos.xyz;\n";
        s << "    out.world_normal = world_normal;\n";
        s << "    out.uv = in.uv;\n";
        s << "    return out;\n";
        s << "}\n";
        return s.str();
    }

    std::string generate_fragment_wgsl() const {
        std::ostringstream s;

        s << "struct MaterialUBO {\n";
        s << "    base_color : vec4<f32>,\n";
        s << "    metallic : f32,\n";
        s << "    roughness : f32,\n";
        s << "};\n\n";

        s << "@group(3) @binding(0) var<uniform> material : MaterialUBO;\n";
        s << "@group(3) @binding(1) var diffuse_map : texture_2d<f32>;\n";
        s << "@group(3) @binding(2) var diffuse_sampler : sampler;\n\n";

        s << "const LIGHT_DIR : vec3<f32> = vec3<f32>(0.408, 0.816, 0.245);\n";
        s << "const LIGHT_COLOR : vec3<f32> = vec3<f32>(1.0, 1.0, 1.0);\n";
        s << "const AMBIENT : vec3<f32> = vec3<f32>(0.15, 0.15, 0.15);\n\n";

        s << "struct FragmentInput {\n";
        s << "    @location(0) world_position : vec3<f32>,\n";
        s << "    @location(1) world_normal : vec3<f32>,\n";
        s << "    @location(2) uv : vec2<f32>,\n";
        s << "};\n\n";

        s << "@fragment\n";
        s << "fn fs_main(in : FragmentInput) -> @location(0) vec4<f32> {\n";
        s << "    let tex_color = textureSample(diffuse_map, diffuse_sampler, in.uv);\n";
        s << "    let albedo = material.base_color.rgb * tex_color.rgb;\n";
        s << "    let alpha  = material.base_color.a * tex_color.a;\n\n";
        s << "    let N = normalize(in.world_normal);\n";
        s << "    let NdotL = max(dot(N, LIGHT_DIR), 0.0);\n";
        s << "    let diffuse = albedo * LIGHT_COLOR * NdotL;\n\n";
        s << "    let color = AMBIENT * albedo + diffuse;\n";
        s << "    return vec4<f32>(color, alpha);\n";
        s << "}\n";
        return s.str();
    }
};
